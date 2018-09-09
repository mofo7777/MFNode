//----------------------------------------------------------------------------------------------
// Dxva2Decoder.cpp
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

CDxva2Decoder::CDxva2Decoder()
	: m_nRefCount(1),
	m_pInputType(NULL),
	m_pOutputType(NULL),
	m_pDeviceManager(NULL),
	m_pDecoderService(NULL),
	m_pVideoDecoder(NULL),
	m_hD3d9Device(NULL),
	m_pConfigs(NULL),
	m_gMpegVld(DXVA2_ModeMPEG2and1_VLD),
	m_dwSampleSize(0),
	m_bHaveOuput(FALSE),
	m_rtAvgPerFrameInput(0),
	m_rtTime(0),
	m_uiWidthInPixels(0),
	m_uiHeightInPixels(0),
	m_bSlice(FALSE),
	m_bMpegHeaderEx(FALSE),
	m_bIsMpeg1(FALSE),
	m_bProgressive(TRUE),
	m_dwTemporalReference(0),
	m_iCurSurfaceIndex(0),
	m_pSliceOffset(NULL),
	m_bFirstPictureI(FALSE),
	m_iLastForward(-1),
	m_iLastPictureP(-1),
	m_bIsDiscontinuity(TRUE),
	m_bDraining(FALSE)
{
	TRACE_TRANSFORM((L"MFTDxva2::CTOR"));

	m_FrameRate.Numerator = m_FrameRate.Denominator = 0;
	m_PixelRatio.Numerator = m_PixelRatio.Denominator = 0;

	for(int i = 0; i < NUM_DXVA2_SURFACE; i++){
		m_pSurface9[i] = NULL;
	}

	InitDxva2Buffers();
}

CDxva2Decoder::~CDxva2Decoder(){

	TRACE_TRANSFORM((L"MFTDxva2::DTOR"));

	AutoLock lock(m_CriticSection);

	Flush();

	if(m_pDeviceManager && m_hD3d9Device){

		LOG_HRESULT(m_pDeviceManager->CloseDeviceHandle(m_hD3d9Device));
		m_hD3d9Device = NULL;
	}

	if(m_pConfigs){
		CoTaskMemFree(m_pConfigs);
		//m_pConfigs = NULL;
	}

	SAFE_RELEASE(m_pDecoderService);
	SAFE_RELEASE(m_pDeviceManager);
	SAFE_RELEASE(m_pInputType);
	SAFE_RELEASE(m_pOutputType);
}

HRESULT CDxva2Decoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_TRANSFORM((L"MFTDxva2::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CDxva2Decoder* pMFT = new (std::nothrow)CDxva2Decoder;

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CDxva2Decoder::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"MFTDxva2::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CDxva2Decoder::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"MFTDxva2::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CDxva2Decoder::QueryInterface(REFIID riid, void** ppv){

	TRACE_TRANSFORM((L"MFTDxva2::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CDxva2Decoder, IMFTransform),
			QITABENT(CDxva2Decoder, IMFAsyncCallback),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

HRESULT CDxva2Decoder::Invoke(IMFAsyncResult* pAsyncResult){

	TRACE_TRANSFORM((L"MFTDxva2::Invoke"));

	HRESULT hr;

	LOG_HRESULT(hr = pAsyncResult->GetStatus());

	IUnknown* pState = NULL;

	if(FAILED(hr = pAsyncResult->GetState(&pState))){

		LOG_HRESULT(hr = E_POINTER);
		return S_OK;
	}

	DWORD dwIndex = ((CSurfaceParam*)pState)->GetSurfaceIndex();

	assert(dwIndex < NUM_DXVA2_SURFACE);

	SAFE_RELEASE(pState);

	AutoLock lock(m_CriticSection);

	m_bFreeSurface[dwIndex] = TRUE;

	return hr;
}

HRESULT CDxva2Decoder::BeginStreaming(){

	TRACE_TRANSFORM((L"MFTDxva2::BeginStreaming"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pDecoderService == NULL ? E_POINTER : S_OK));

	if(m_pVideoDecoder != NULL)
		return hr;

	IF_FAILED_RETURN(hr = m_InputBuffer.Initialize());
	IF_FAILED_RETURN(hr = m_SliceBuffer.Initialize(SLICE_BUFFER_SIZE));

	m_pSliceOffset = new (std::nothrow)unsigned int[MAX_SLICE];
	IF_FAILED_RETURN(m_pSliceOffset == NULL ? E_OUTOFMEMORY : S_OK);

	IF_FAILED_RETURN(hr = m_pDecoderService->CreateSurface(m_uiWidthInPixels, m_uiHeightInPixels, NUM_DXVA2_SURFACE - 1,
		static_cast<D3DFORMAT>(D3DFMT_NV12), D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, m_pSurface9, NULL));

	/*D3DLOCKED_RECT d3dRect;

	for(int i = 0; i < NUM_DXVA2_SURFACE; i++){

	IF_FAILED_RETURN(hr = m_pSurface9[i]->LockRect(&d3dRect, NULL, 0));
	memset(d3dRect.pBits, 0x80, (d3dRect.Pitch * (m_uiHeightInPixels + (m_uiHeightInPixels / 2))));
	IF_FAILED_RETURN(hr = m_pSurface9[i]->UnlockRect());
	}*/

	InitDxva2Buffers();

	IF_FAILED_RETURN(hr = m_pDecoderService->CreateVideoDecoder(m_gMpegVld, &m_Dxva2Desc, &m_pConfigs[0], m_pSurface9, NUM_DXVA2_SURFACE, &m_pVideoDecoder));

	return hr;
}

HRESULT CDxva2Decoder::Flush(){

	TRACE_TRANSFORM((L"MFTDxva2::Flush"));

	HRESULT hr = S_OK;

	if(m_pVideoDecoder == NULL)
		return hr;

	ReleaseInput();

	// Check if we need to lock directx device, if device is not closed before...
	for(int i = 0; i < NUM_DXVA2_SURFACE; i++){
		SAFE_RELEASE(m_pSurface9[i]);
	}

	m_cTSScheduler.Reset();

	m_bProgressive = TRUE;
	m_bHaveOuput = FALSE;
	m_rtTime = 0;
	m_bDraining = FALSE;

	SAFE_RELEASE(m_pVideoDecoder);

	return hr;
}

void CDxva2Decoder::ReleaseInput(){

	TRACE_TRANSFORM((L"MFTDxva2::ReleaseInput"));

	if(m_pSliceOffset == NULL)
		return;

	m_InputBuffer.Reset();
	m_SliceBuffer.Reset();

	m_bSlice = FALSE;
	m_dwTemporalReference = 0;
	m_iCurSurfaceIndex = 0;
	m_bFirstPictureI = FALSE;
	m_iLastForward = -1;
	m_iLastPictureP = -1;
	m_bIsDiscontinuity = TRUE;

	SAFE_DELETE_ARRAY(m_pSliceOffset);
}

void CDxva2Decoder::InitDxva2Buffers(){

	TRACE_TRANSFORM((L"MFTDxva2::InitDxva2Buffers"));

	memset(&m_ExecuteParams, 0, sizeof(DXVA2_DecodeExecuteParams));
	memset(m_BufferDesc, 0, 4 * sizeof(DXVA2_DecodeBufferDesc));
	memset(&m_PictureParams, 0, sizeof(DXVA_PictureParameters));
	memset(m_SliceInfo, 0, MAX_SLICE * sizeof(DXVA_SliceInfo));
	memset(&m_QuantaMatrix, 0, sizeof(DXVA_QmatrixData));
	memset(&m_VideoParam, 0, sizeof(VIDEO_PARAMS));
	memset(&m_Dxva2Desc, 0, sizeof(DXVA2_VideoDesc));
	memset(&m_bFreeSurface, 1, sizeof(m_bFreeSurface));

	m_ExecuteParams.NumCompBuffers = 4;

	m_BufferDesc[0].CompressedBufferType = DXVA2_PictureParametersBufferType;
	m_BufferDesc[0].DataSize = sizeof(DXVA_PictureParameters);

	m_BufferDesc[1].CompressedBufferType = DXVA2_InverseQuantizationMatrixBufferType;
	m_BufferDesc[1].DataSize = sizeof(DXVA_QmatrixData);

	m_BufferDesc[2].CompressedBufferType = DXVA2_BitStreamDateBufferType;

	m_BufferDesc[3].CompressedBufferType = DXVA2_SliceControlBufferType;

	m_ExecuteParams.pCompressedBuffers = m_BufferDesc;

	m_VideoParam.intra_dc_precision = 0;
	m_VideoParam.picture_structure = 3;
	m_VideoParam.top_field_first = m_bIsMpeg1;
	m_VideoParam.frame_pred_frame_dct = 1;
	m_VideoParam.concealment_motion_vectors = 0;
	m_VideoParam.q_scale_type = 0;
	m_VideoParam.intra_vlc_format = 0;
	m_VideoParam.alternate_scan = 0;
	m_VideoParam.repeat_first_field = 0;
	m_VideoParam.chroma_420_type = 1;
	m_VideoParam.progressive_frame = 1;
	m_VideoParam.f_code[0][0] = 15;
	m_VideoParam.f_code[0][1] = 15;
	m_VideoParam.f_code[1][0] = 15;
	m_VideoParam.f_code[1][1] = 15;

	// Check : already initialized
	DXVA2_Frequency Dxva2Freq;
	Dxva2Freq.Numerator = m_FrameRate.Numerator;
	Dxva2Freq.Denominator = m_FrameRate.Denominator;

	m_Dxva2Desc.SampleWidth = m_uiWidthInPixels;
	m_Dxva2Desc.SampleHeight = m_uiHeightInPixels;

	m_Dxva2Desc.SampleFormat.SampleFormat = (m_bProgressive ? MFVideoInterlace_Progressive : MFVideoInterlace_MixedInterlaceOrProgressive);

	m_Dxva2Desc.Format = static_cast<D3DFORMAT>(D3DFMT_NV12);
	m_Dxva2Desc.InputSampleFreq = Dxva2Freq;
	m_Dxva2Desc.OutputFrameFreq = Dxva2Freq;

	InitQuanta();
}