//----------------------------------------------------------------------------------------------
// MFTCudaDecoder.cpp
// Copyright (C) 2014 Dumonteil David
//
// MFNode is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MFNode is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CMFTCudaDecoder::CMFTCudaDecoder()
	: m_nRefCount(1),
	m_pInputType(NULL),
	m_pOutputType(NULL),
	m_dwTemporalReference(0),
	m_dwCudaFrameSize(0),
	m_dwSampleSize(0),
	m_uiFrameWidth(0),
	m_uiFrameHeight(0),
	m_rtAvgPerFrame(0),
	m_rtTime(0),
	m_pSliceOffset(NULL),
	m_iCurIDX(0),
	m_iLastPictureP(-1),
	m_iLastForward(-1),
	m_bSlice(FALSE),
	m_bMpeg1(FALSE),
	m_bProgressive(TRUE),
	m_bMpegHeaderEx(FALSE),
	m_bNV12(FALSE),
	m_bDraining(FALSE),
	m_bIsDiscontinuity(TRUE),
	m_bFirstPictureI(FALSE)
{
	TRACE_TRANSFORM((L"MFTCuda::CTOR"));

	m_FrameRate.Numerator = m_FrameRate.Denominator = 0;
	m_Pixel.Numerator = m_Pixel.Denominator = 0;

	memset(&m_VideoParam, 0, sizeof(CUVIDPICPARAMS));
	//InitQuanta();

		  // ToDo : not here (CreateInstance...)
	m_pSliceOffset = new (std::nothrow)unsigned int[MAX_SLICE];
}

CMFTCudaDecoder::~CMFTCudaDecoder(){

	TRACE_TRANSFORM((L"MFTCuda::DTOR"));

	AutoLock lock(m_CriticSection);

	EndStreaming();
	SAFE_RELEASE(m_pInputType);
	SAFE_RELEASE(m_pOutputType);

	SAFE_DELETE_ARRAY(m_pSliceOffset);
}

HRESULT CMFTCudaDecoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_TRANSFORM((L"MFTCuda::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMFTCudaDecoder* pMFT = new (std::nothrow)CMFTCudaDecoder;

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CMFTCudaDecoder::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"MFTCuda::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFTCudaDecoder::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"MFTCuda::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMFTCudaDecoder::QueryInterface(REFIID riid, void** ppv){

	TRACE_TRANSFORM((L"MFTCuda::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CMFTCudaDecoder, IMFTransform),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

HRESULT CMFTCudaDecoder::BeginStreaming(){

	HRESULT hr;

	IF_FAILED_RETURN(hr = ((m_pInputType == NULL || m_pOutputType == NULL) ? MF_E_TRANSFORM_TYPE_NOT_SET : S_OK));

	// MFT_MESSAGE_NOTIFY_END_STREAMING must be called
	IF_FAILED_RETURN(hr = (m_cCudaDecoder.IsInit() ? MF_E_INVALIDREQUEST : S_OK));

	IMFSample* pSampleOut = NULL;

	try{

		IF_FAILED_THROW(hr = m_InputBuffer.Initialize());

		IF_FAILED_THROW(hr = m_SliceBuffer.Initialize(SLICE_BUFFER_SIZE));

		IF_FAILED_THROW(hr = m_cCudaManager.InitCudaManager());

		InitQuanta();

		m_cCudaDecoder.Release();
		IF_FAILED_THROW(hr = (m_cCudaDecoder.InitDecoder(m_cCudaManager.GetVideoLock(), m_uiFrameWidth, m_uiFrameHeight, m_bMpeg1)));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSampleOut);

	return hr;
}

void CMFTCudaDecoder::EndStreaming(){

	m_cCudaDecoder.Release();
	m_cCudaManager.Release();

	OnFlush();
}

HRESULT CMFTCudaDecoder::OnFlush(){

	// Todo check all reset value here

	// Perhaps we should delete the buffer, but it can be reused.
	m_InputBuffer.Reset();
	m_SliceBuffer.Reset();
	m_cCudaFrame.Reset();

	while(!m_qSampleOut.empty()){

		IMFSample* pSample = m_qSampleOut.front();
		SAFE_RELEASE(pSample);
		m_qSampleOut.pop();
	}

	memset(&m_VideoParam, 0, sizeof(CUVIDPICPARAMS));
	//InitQuanta();

	m_dwTemporalReference = 0;
	//m_dwCudaFrameSize = 0;

	m_rtTime = 0;

	m_iCurIDX = 0;
	m_iLastPictureP = -1;
	m_iLastForward = -1;

	m_bSlice = FALSE;
	m_bMpeg1 = FALSE;
	//m_bProgressive = TRUE;
	m_bMpegHeaderEx = FALSE;
	//m_bNV12 = FALSE;
	m_bDraining = FALSE;
	m_bIsDiscontinuity = TRUE;
	m_bFirstPictureI = FALSE;

	return S_OK;
}

HRESULT CMFTCudaDecoder::OnDrain(){

	m_bDraining = TRUE;
	return S_OK;
}

void CMFTCudaDecoder::InitQuanta(){

	if(m_bMpeg1){

		BYTE btQuantInter[64] = {
				16, 17, 18, 19, 20, 21, 22, 23, 17, 18,
				19, 20, 21, 22, 23, 24, 18, 19, 20, 21,
				22, 23, 24, 25, 19, 20, 21, 22, 23, 24,
				26, 27, 20, 21, 22, 23, 25, 26, 27, 28,
				21, 22, 23, 24, 26, 27, 28, 30, 22, 23,
				24, 26, 27, 28, 30, 31, 23, 24, 25, 27,
				28, 30, 31, 33
		};

		memcpy(m_VideoParam.CodecSpecific.mpeg2.QuantMatrixInter, btQuantInter, 64);
		m_VideoParam.CodecSpecific.mpeg2.frame_pred_frame_dct = 1;
	}
	else{
		memset(m_VideoParam.CodecSpecific.mpeg2.QuantMatrixInter, 16, 64);
	}

	BYTE btQuantIntra[64] = {
			8, 16, 19, 22, 26, 27, 29, 34, 16, 16,
			22, 24, 27, 29, 34, 37, 19, 22, 26, 27,
			29, 34, 34, 38, 22, 22, 26, 27, 29, 34,
			37, 40, 22, 26, 27, 29, 32, 35, 40, 48,
			26, 27, 29, 32, 35, 40, 48, 58, 26, 27,
			29, 34, 38, 46, 56, 69, 27, 29, 35, 38,
			46, 56, 69, 83
	};

	memcpy(m_VideoParam.CodecSpecific.mpeg2.QuantMatrixIntra, btQuantIntra, 64);
}