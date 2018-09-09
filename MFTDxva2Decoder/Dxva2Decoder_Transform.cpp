//----------------------------------------------------------------------------------------------
// Dxva2Decoder_Transform.cpp
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

HRESULT CDxva2Decoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

	TRACE_TRANSFORM((L"MFTDxva2::GetStreamLimits"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return hr;
}

HRESULT CDxva2Decoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

	TRACE_TRANSFORM((L"MFTDxva2::GetStreamCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return hr;
}

HRESULT CDxva2Decoder::GetStreamIDs(DWORD /*dwInputIDArraySize*/, DWORD* /*pdwInputIDs*/, DWORD /*dwOutputIDArraySize*/, DWORD* /*pdwOutputIDs*/){

	TRACE_TRANSFORM((L"MFTDxva2::GetStreamIDs"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"MFTDxva2::GetInputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags = MFT_INPUT_STREAM_DOES_NOT_ADDREF;
	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	return hr;
}

HRESULT CDxva2Decoder::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"MFTDxva2::GetOutputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	pStreamInfo->dwFlags =
		MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
		MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
		MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE |
		MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

	pStreamInfo->cbAlignment = 0;
	pStreamInfo->cbSize = 0;

	return hr;
}

HRESULT CDxva2Decoder::GetAttributes(IMFAttributes** pAttributes){

	TRACE_TRANSFORM((L"MFTDxva2::GetAttributes"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pAttributes == NULL ? E_POINTER : S_OK));

	IMFAttributes* pAttribute = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateAttributes(&pAttribute, 1));
		IF_FAILED_THROW(hr = pAttribute->SetUINT32(MF_SA_D3D_AWARE, TRUE));

		*pAttributes = pAttribute;
		(*pAttributes)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pAttribute);

	return hr;
}

HRESULT CDxva2Decoder::GetInputStreamAttributes(DWORD /*dwInputStreamID*/, IMFAttributes** /*ppAttributes*/){

	TRACE_TRANSFORM((L"MFTDxva2::GetInputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::GetOutputStreamAttributes(DWORD /*dwOutputStreamID*/, IMFAttributes** /*ppAttributes*/){

	TRACE_TRANSFORM((L"MFTDxva2::GetOutputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::DeleteInputStream(DWORD /*dwStreamID*/){

	TRACE_TRANSFORM((L"MFTDxva2::DeleteInputStream"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::AddInputStreams(DWORD /*cStreams*/, DWORD* /*adwStreamIDs*/){

	TRACE_TRANSFORM((L"MFTDxva2::AddInputStreams"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::GetInputAvailableType(DWORD /*dwInputStreamID*/, DWORD /*dwTypeIndex*/, IMFMediaType** /*ppType*/){

	TRACE_TRANSFORM((L"MFTDxva2::GetInputAvailableType"));

	return MF_E_NO_MORE_TYPES;
}

HRESULT CDxva2Decoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"MFTDxva2::GetOutputAvailableType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputType == NULL){
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else{
		hr = GetOutputType(ppType);
	}

	return hr;
}

HRESULT CDxva2Decoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"MFTDxva2::SetInputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_InputBuffer.GetBufferSize() != 0 ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

	if(pType){
		hr = OnCheckInputType(pType);
	}

	if(SUCCEEDED(hr)){

		if(bReallySet){
			hr = OnSetInputType(pType);
		}
	}

	return hr;
}

HRESULT CDxva2Decoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"MFTDxva2::SetOutputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_InputBuffer.GetBufferSize() != 0 ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

	if(pType){
		hr = OnCheckOutputType(pType);
	}

	if(SUCCEEDED(hr)){

		if(bReallySet){

			SAFE_RELEASE(m_pOutputType);

			m_pOutputType = pType;
			m_pOutputType->AddRef();
		}
	}

	return hr;
}

HRESULT CDxva2Decoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"MFTDxva2::GetInputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	if(!m_pInputType){
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else{

		*ppType = m_pInputType;
		(*ppType)->AddRef();
	}

	return hr;
}

HRESULT CDxva2Decoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"MFTDxva2::GetOutputCurrentType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	if(!m_pOutputType){
		hr = MF_E_TRANSFORM_TYPE_NOT_SET;
	}
	else{

		*ppType = m_pOutputType;
		(*ppType)->AddRef();
	}

	return hr;
}

HRESULT CDxva2Decoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

	TRACE_TRANSFORM((L"MFTDxva2::GetInputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	// I think we can always process
	*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;

	return hr;
}

HRESULT CDxva2Decoder::GetOutputStatus(DWORD* pdwFlags){

	TRACE_TRANSFORM((L"MFTDxva2::GetOutputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_bHaveOuput){
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	}
	else{
		*pdwFlags = 0;
	}

	return hr;
}

HRESULT CDxva2Decoder::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/){

	TRACE_TRANSFORM((L"MFTDxva2::SetOutputBounds"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent */){

	TRACE_TRANSFORM((L"MFTDxva2::ProcessEvent"));

	return E_NOTIMPL;
}

HRESULT CDxva2Decoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR ulParam){

	TRACE_TRANSFORM((L"MFTDxva2::ProcessMessage : %s", MFTMessageString(eMessage)));

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	switch(eMessage){

	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
	case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
		hr = BeginStreaming();
		break;

	case MFT_MESSAGE_COMMAND_FLUSH:
	case MFT_MESSAGE_NOTIFY_END_STREAMING:
	case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
		hr = Flush();
		break;

	case MFT_MESSAGE_COMMAND_DRAIN:
		m_bDraining = TRUE;
		break;

	case MFT_MESSAGE_SET_D3D_MANAGER:
		hr = GetVideoDecoder(reinterpret_cast<IDirect3DDeviceManager9*>(ulParam));
		break;
	}

	return hr;
}

HRESULT CDxva2Decoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

	TRACE_TRANSFORM((L"MFTDxva2::ProcessInput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if(!m_pInputType || !m_pOutputType || m_bHaveOuput){
		return MF_E_NOTACCEPTING;
	}

	if(m_bDraining){
		ReleaseInput();
		return MF_E_NOTACCEPTING;
	}

	DWORD dwBufferCount = 0;
	IF_FAILED_RETURN(hr = pSample->GetBufferCount(&dwBufferCount));

	assert(dwBufferCount != 0);

	IMFMediaBuffer* pInputBuffer = NULL;
	BYTE* pbInputData = NULL;
	DWORD dwInputLength;

	// Todo : do not ConvertToContiguousBuffer, there is always one buffer with MFSrMpeg2Splitter...
	IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&pInputBuffer));

	try{

		LONGLONG llPTS = -1;
		LONGLONG llDTS = -1;

		if(SUCCEEDED(pSample->GetSampleTime(&llPTS))){

			pSample->GetUINT64(MFSampleExtension_DecodedTimeStamp, (UINT64*)&llDTS);
		}

#ifdef TRACE_INPUT_REFERENCE
		TRACE((L"Time = %I64d : DTS = %I64d", llPTS, llDTS));
#endif

		if(llPTS != -1)
			m_cTSScheduler.SetTime(llPTS, llDTS);

		IF_FAILED_THROW(hr = pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

		IF_FAILED_THROW(hr = Slice(pbInputData, dwInputLength));
	}
	catch(HRESULT){}

	if(pInputBuffer){

		if(pbInputData)
			LOG_HRESULT(pInputBuffer->Unlock());

		SAFE_RELEASE(pInputBuffer);
	}

	return hr;
}

HRESULT CDxva2Decoder::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

	TRACE_TRANSFORM((L"MFTDxva2::ProcessOutput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample != NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_bDraining && !m_cTSScheduler.IsEmpty()){

		ReleaseInput();
		m_bHaveOuput = TRUE;
	}

	if(m_bHaveOuput == FALSE){
		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	}

	//IDirect3DDevice9* pDevice = NULL;
	IMFSample* pSample = NULL;
	IMFTrackedSample* pTrackSample = NULL;
	CSurfaceParam* pSurfaceParam = NULL;

	try{

		// Seems not to be needed
		//IF_FAILED_THROW(hr = m_pDeviceManager->TestDevice(m_hD3d9Device));
		//IF_FAILED_THROW(hr = m_pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));

		const STemporalRef* pRef = m_cTSScheduler.GetNextFrame();

		IF_FAILED_THROW(hr = MFCreateVideoSampleFromSurface(m_pSurface9[pRef->iIndex], &pSample));

		//IF_FAILED_THROW(hr = m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));

		pSurfaceParam = new (std::nothrow)CSurfaceParam(pRef->iIndex);
		IF_FAILED_THROW(hr = (pSurfaceParam == NULL ? E_POINTER : S_OK));

		IF_FAILED_THROW(hr = pSample->QueryInterface(IID_IMFTrackedSample, reinterpret_cast<void**>(&pTrackSample)));
		IF_FAILED_THROW(hr = pTrackSample->SetAllocator(this, pSurfaceParam));

		if(pRef->rtTime == -1){
			m_rtTime += m_rtAvgPerFrameInput;
		}
		else{
			m_rtTime = pRef->rtTime;
		}

		IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtTime));
		IF_FAILED_THROW(hr = pSample->SetSampleDuration(m_rtAvgPerFrameInput));

		if(m_bProgressive){
			IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Interlaced, 1));
		}
		else{
			IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Interlaced, 0));
		}

		if(m_bIsDiscontinuity){
			IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, m_bIsDiscontinuity));
			m_bIsDiscontinuity = FALSE;
		}

		pOutputSamples[0].pSample = pSample;
		pSample->AddRef();

		if(m_bDraining && !m_cTSScheduler.IsEmpty()){

			pOutputSamples[0].dwStatus = MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
		}
		else{

			m_bHaveOuput = FALSE;
		}
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSurfaceParam);
	//SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pTrackSample);
	SAFE_RELEASE(pSample);

	return hr;
}