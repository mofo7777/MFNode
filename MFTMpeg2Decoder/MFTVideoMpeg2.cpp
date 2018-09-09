//----------------------------------------------------------------------------------------------
// MFTVideoMpeg2.cpp
// Copyright (C) 2012 Dumonteil David
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

CMFTVideoMpeg2::CMFTVideoMpeg2()
	: m_pMpeg2Decoder(NULL),
	m_pInputType(NULL),
	m_pOutputType(NULL),
	m_pInputBuffer(NULL),
	//m_pOuputLastBuffer(NULL),
	m_pbInputData(NULL),
	m_dwInputLength(0),
	m_pSampleOut(NULL),
	//m_bDiscontinuity(FALSE),
	//m_bSetDiscontinuity(FALSE),
	//m_rtFrameTest(0),
	m_dwNumVideoFrame(0),
	m_rtFrame(0),
	m_rtAvgPerFrame(0),
	m_rtAvgPerFrameInput(0),
	m_uiWidthInPixels2(0),
	m_uiHeightInPixels2(0),
	m_dwSampleSize2(0){

	m_FrameRate.Numerator = m_FrameRate.Denominator = 0;

	//TRACE((L"MFTVideo::CTOR"));
}

CMFTVideoMpeg2::~CMFTVideoMpeg2(){

	AutoLock lock(m_CriticSection);

	FreeStreamingResources();
	SAFE_RELEASE(m_pInputType);
	SAFE_RELEASE(m_pOutputType);
}

HRESULT CMFTVideoMpeg2::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CMFTVideoMpeg2* pMFT = new (std::nothrow)CMFTVideoMpeg2;

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

HRESULT CMFTVideoMpeg2::QueryInterface(REFIID riid, void** ppv){

	if(NULL == ppv){
		return E_POINTER;
	}
	else if(riid == __uuidof(IUnknown)){
		*ppv = static_cast<IUnknown*>(this);
	}
	else if(riid == __uuidof(IMFTransform)){
		*ppv = static_cast<IMFTransform*>(this);
	}
	else{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

HRESULT CMFTVideoMpeg2::AllocateStreamingResources(){

	//TRACE((L"AllocateStreamingResources"));

	if(m_pMpeg2Decoder != NULL)
		return S_OK;

	// The client must set both media types before allocating streaming resources.
	if(m_pInputType == NULL || m_pOutputType == NULL){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	HRESULT hr = S_OK;
	CMpeg2Decoder* pDec = NULL;
	IMFSample*     pSampleOut = NULL;

	m_rtFrame = 0;

	try{

		IF_FAILED_THROW(hr = MFCreateSample(&pSampleOut));
		IF_FAILED_THROW(hr = CMpeg2Decoder::CreateInstance(&pDec));

		m_pMpeg2Decoder = pDec;
		m_pMpeg2Decoder->AddRef();
		m_pMpeg2Decoder->Mpeg2DecodeInit();

		m_pSampleOut = pSampleOut;
		m_pSampleOut->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSampleOut);
	SAFE_RELEASE(pDec);

	return hr;
}

void CMFTVideoMpeg2::FreeStreamingResources(){

	//TRACE((L"MFT::FreeStreamingResources"));

	if(m_pMpeg2Decoder){
		m_pMpeg2Decoder->Mpeg2DecodeClose();
		SAFE_RELEASE(m_pMpeg2Decoder);
	}

	ReleaseInputBuffer();
	ResetOutSample();
	SAFE_RELEASE(m_pSampleOut);
	//SAFE_RELEASE(m_pOuputLastBuffer);

	// Perhaps no need to reset
	m_rtFrame = 0;
	m_rtAvgPerFrame = 0;
	m_rtAvgPerFrameInput = 0;
	m_uiWidthInPixels2 = 0;
	m_uiHeightInPixels2 = 0;
	m_dwSampleSize2 = 0;

	m_dwNumVideoFrame = 0;

	m_FrameRate.Numerator = m_FrameRate.Denominator = 0;
}

HRESULT CMFTVideoMpeg2::GetOutputType(IMFMediaType** ppType){

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		// MFVideoFormat_YV12 MFVideoFormat_I420
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YV12));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, m_uiWidthInPixels2));

		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_dwSampleSize2));

		IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_uiWidthInPixels2, m_uiHeightInPixels2));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_FrameRate.Numerator, m_FrameRate.Denominator));

		// See MPEG2VideoSeqHeader pixelAspectRatio
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 4, 3));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CMFTVideoMpeg2::OnCheckInputType(IMFMediaType* pmt){

	// Check if the type is already set and if so reject any type that's not identical.
	if(m_pInputType){

		DWORD dwFlags = 0;

		if(m_pInputType->IsEqual(pmt, &dwFlags) == S_OK){
			return S_OK;
		}
		else{
			return MF_E_INVALIDTYPE;
		}
	}

	HRESULT hr = S_OK;

	//  We accept MFMediaType_Video, MFVideoFormat_MPG1, MFVideoFormat_MPEG2
	GUID majortype = {0};
	GUID subtype = {0};
	UINT32 width = 0;
	UINT32 height = 0;
	MFRatio fps = {0};
	UINT32 cbSeqHeader = 0;

	IF_FAILED_RETURN(hr = pmt->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height));
	// Make sure there is a frame rate. 
	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pmt, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator));
	IF_FAILED_RETURN(hr = pmt->GetBlobSize(MF_MT_MPEG_SEQUENCE_HEADER, &cbSeqHeader));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = ((subtype != MFVideoFormat_MPG1 && subtype != MFVideoFormat_MPEG2 && subtype != MEDIASUBTYPE_MPEG1Payload) ? MF_E_INVALIDTYPE : S_OK));

	// Validate the frame size.
	IF_FAILED_RETURN(hr = ((width > MAX_VIDEO_WIDTH_HEIGHT || height > MAX_VIDEO_WIDTH_HEIGHT) ? MF_E_INVALIDTYPE : S_OK));

	// Check for a sequence header.
	IF_FAILED_RETURN(hr = (cbSeqHeader < MIN_BYTE_TO_READ_MPEG_HEADER ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CMFTVideoMpeg2::OnSetInputType(IMFMediaType* pType){

	HRESULT hr = S_OK;

	SAFE_RELEASE(m_pInputType);

	IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uiWidthInPixels2, &m_uiHeightInPixels2));
	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&m_FrameRate.Numerator, (UINT32*)&m_FrameRate.Denominator));

	// Also store the frame duration, derived from the frame rate.
	IF_FAILED_RETURN(hr = MFFrameRateToAverageTimePerFrame(m_FrameRate.Numerator, m_FrameRate.Denominator, &m_rtAvgPerFrameInput));

	m_dwSampleSize2 = (m_uiHeightInPixels2 + (m_uiHeightInPixels2 / 2)) * m_uiWidthInPixels2;

	m_pInputType = pType;
	m_pInputType->AddRef();

	return hr;
}

HRESULT CMFTVideoMpeg2::OnCheckOutputType(IMFMediaType* pType){

	// Check if the type is already set and if so reject any type that's not identical.
	if(m_pOutputType){

		DWORD dwFlags = 0;

		if(m_pOutputType->IsEqual(pType, &dwFlags) == S_OK){
			return S_OK;
		}
		else{
			return MF_E_INVALIDTYPE;
		}
	}

	// Input type must be set first.
	if(m_pInputType == NULL){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	HRESULT hr = S_OK;
	IMFMediaType* pOurType = NULL;

	try{

		BOOL bMatch = FALSE;

		// Make sure their type is a superset of our proposed output type.
		IF_FAILED_THROW(hr = GetOutputType(&pOurType));

		IF_FAILED_THROW(hr = pOurType->Compare(pType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch));

		if(!bMatch)
			throw hr = MF_E_INVALIDTYPE;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOurType);
	return hr;
}

HRESULT CMFTVideoMpeg2::OnDrain(){

	ReleaseInputBuffer();

	m_rtFrame = 0;

	return S_OK;
}

HRESULT CMFTVideoMpeg2::OnFlush(){

	ReleaseInputBuffer();
	ResetOutSample();
	//SAFE_RELEASE(m_pOuputLastBuffer);

	m_rtFrame = 0;
	//m_rtFrameTest = 0;
	//m_bDiscontinuity = FALSE;
	//m_bSetDiscontinuity = FALSE;

	return S_OK;
}