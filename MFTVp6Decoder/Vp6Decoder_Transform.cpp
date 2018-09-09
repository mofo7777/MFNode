//----------------------------------------------------------------------------------------------
// Vp6Decoder_Transform.cpp
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

HRESULT CVp6Decoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

	TRACE_TRANSFORM((L"VP6Decoder::GetStreamLimits"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

	*pdwInputMinimum = 1;
	*pdwInputMaximum = 1;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return hr;
}

HRESULT CVp6Decoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

	TRACE_TRANSFORM((L"VP6Decoder::GetStreamCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

	*pcInputStreams = 1;
	*pcOutputStreams = 1;

	return hr;
}

HRESULT CVp6Decoder::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

	TRACE_TRANSFORM((L"VP6Decoder::GetStreamIDs"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"VP6Decoder::GetInputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES;
	pStreamInfo->hnsMaxLatency = 0;
	pStreamInfo->cbSize = 1;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 1;

	return hr;
}

HRESULT CVp6Decoder::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputStreamInfo"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;

	if(m_pOutputType){

		pStreamInfo->cbSize = m_dwSampleSize;
		pStreamInfo->cbAlignment = 1;
	}
	else{

		pStreamInfo->cbSize = 0;
		pStreamInfo->cbAlignment = 0;
	}

	return hr;
}

HRESULT CVp6Decoder::GetAttributes(IMFAttributes**){

	TRACE_TRANSFORM((L"VP6Decoder::GetAttributes"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::GetInputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"VP6Decoder::GetInputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::GetOutputStreamAttributes(DWORD, IMFAttributes**){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputStreamAttributes"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::DeleteInputStream(DWORD){

	TRACE_TRANSFORM((L"VP6Decoder::DeleteInputStream"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::AddInputStreams(DWORD, DWORD*){

	TRACE_TRANSFORM((L"VP6Decoder::AddInputStreams"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::GetInputAvailableType(DWORD, DWORD, IMFMediaType**){

	TRACE_TRANSFORM((L"VP6Decoder::GetInputAvailableType"));

	return MF_E_NO_MORE_TYPES;
}

HRESULT CVp6Decoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputAvailableType"));

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

HRESULT CVp6Decoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"VP6Decoder::SetInputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pInputSample != NULL ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

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

HRESULT CVp6Decoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

	TRACE_TRANSFORM((L"VP6Decoder::SetOutputType"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	AutoLock lock(m_CriticSection);

	// Todo
	//IF_FAILED_RETURN(hr = (HaveSampleOut() ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

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

HRESULT CVp6Decoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"VP6Decoder::GetInputCurrentType"));

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

HRESULT CVp6Decoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputCurrentType"));

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

HRESULT CVp6Decoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

	TRACE_TRANSFORM((L"VP6Decoder::GetInputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputSample){
		*pdwFlags = 0;
	}
	else{
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
	}

	return hr;
}

HRESULT CVp6Decoder::GetOutputStatus(DWORD* pdwFlags){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputStatus"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputSample){
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	}
	else{
		*pdwFlags = 0;
	}

	return hr;
}

HRESULT CVp6Decoder::SetOutputBounds(LONGLONG, LONGLONG){

	TRACE_TRANSFORM((L"VP6Decoder::SetOutputBounds"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::ProcessEvent(DWORD, IMFMediaEvent*){

	TRACE_TRANSFORM((L"VP6Decoder::ProcessEvent"));

	return E_NOTIMPL;
}

HRESULT CVp6Decoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

	TRACE_TRANSFORM((L"VP6Decoder::ProcessMessage : %s", MFTMessageString(eMessage)));

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	switch(eMessage){

	case MFT_MESSAGE_COMMAND_FLUSH:
	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
	case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
	case MFT_MESSAGE_NOTIFY_END_STREAMING:
	case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
		OnFlush();
		break;

	case MFT_MESSAGE_COMMAND_DRAIN:
		m_bDraining = TRUE;
		break;

	case MFT_MESSAGE_SET_D3D_MANAGER:
		hr = E_NOTIMPL;
		break;
	}

	return hr;
}

HRESULT CVp6Decoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

	TRACE_TRANSFORM((L"VP6Decoder::ProcessInput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if(!m_pOutputType){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	if(m_pInputSample || m_bDraining){
		return MF_E_NOTACCEPTING;
	}

	m_pInputSample = pSample;
	m_pInputSample->AddRef();

	return hr;
}

HRESULT CVp6Decoder::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

	TRACE_TRANSFORM((L"VP6Decoder::ProcessOutput"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	if(m_pInputSample == NULL){
		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	}

	DWORD dwBufferCount = 0;
	IF_FAILED_RETURN(hr = m_pInputSample->GetBufferCount(&dwBufferCount));

	assert(dwBufferCount == 1);

	IMFMediaBuffer* pInputBuffer = NULL;
	BYTE* pbInputData = NULL;
	DWORD dwInputLength;

	IMFMediaBuffer* pOutputBuffer = NULL;
	BYTE* pbOutputData = NULL;
	DWORD dwOutputLength = 0;

	try{

		IF_FAILED_THROW(hr = m_pInputSample->GetBufferByIndex(0, &pInputBuffer));
		IF_FAILED_THROW(hr = pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

		if(m_pDecoder.decodePacket(pbInputData, dwInputLength) < 0){
			IF_FAILED_THROW(hr = MF_E_TRANSFORM_NEED_MORE_INPUT);
		}

		BYTE* yuv[3];
		int pitch;
		m_pDecoder.getYUV(yuv, &pitch);

		//int width = 0;
		//int height = 0;

		//m_pDecoder.getImageSize(&width, &height);
		////m_pDecoder.getDisplaySize(&width, &height);

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(m_dwSampleSize, &pOutputBuffer));
		IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(m_dwSampleSize));
		IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pbOutputData, NULL, &dwOutputLength));

		Vp6CopyFrameYV12Stride(pitch, m_uiWidthInPixels, m_uiHeightInPixels, pbOutputData, yuv[0], yuv[2], yuv[1]);

		pOutputSamples[0].pSample->AddBuffer(pOutputBuffer);

		UINT32 uiTmp;

		if(SUCCEEDED(hr = m_pInputSample->GetUINT32(MFSampleExtension_CleanPoint, &uiTmp)))
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetUINT32(MFSampleExtension_CleanPoint, uiTmp));

		if(SUCCEEDED(hr = m_pInputSample->GetUINT32(MFSampleExtension_Discontinuity, &uiTmp)))
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetUINT32(MFSampleExtension_Discontinuity, uiTmp));

		LONGLONG llTmp;

		if(SUCCEEDED(hr = m_pInputSample->GetSampleTime(&llTmp)))
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleTime(llTmp));

		if(SUCCEEDED(hr = m_pInputSample->GetSampleDuration(&llTmp)))
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleDuration(llTmp));
	}
	catch(HRESULT){}

	if(pInputBuffer && pbInputData){
		LOG_HRESULT(pInputBuffer->Unlock());
	}

	if(pOutputBuffer && pbOutputData){
		LOG_HRESULT(pOutputBuffer->Unlock());
	}

	SAFE_RELEASE(pOutputBuffer);
	SAFE_RELEASE(pInputBuffer);
	SAFE_RELEASE(m_pInputSample);

	return hr;
}