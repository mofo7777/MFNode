//----------------------------------------------------------------------------------------------
// MFTWaveMixer_Transform.cpp
// Copyright (C) 2013 Dumonteil David
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

HRESULT CMFTWaveMixer::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

	if((pdwInputMinimum == NULL) || (pdwInputMaximum == NULL) || (pdwOutputMinimum == NULL) || (pdwOutputMaximum == NULL)){
		return E_POINTER;
	}

	// This MFT has a fixed number of streams.
	*pdwInputMinimum = 2;
	*pdwInputMaximum = 2;
	*pdwOutputMinimum = 1;
	*pdwOutputMaximum = 1;

	return S_OK;
}

HRESULT CMFTWaveMixer::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

	if((pcInputStreams == NULL) || (pcOutputStreams == NULL)){
		return E_POINTER;
	}

	// This MFT has a fixed number of streams.
	*pcInputStreams = 2;
	*pcOutputStreams = 1;

	return S_OK;
}

HRESULT CMFTWaveMixer::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	pStreamInfo->hnsMaxLatency = 0;

	// MFT_INPUT_STREAM_WHOLE_SAMPLES      : The MFT must get complete audio frames.
	// MFT_INPUT_STREAM_PROCESSES_IN_PLACE : The MFT can do in-place processing.
	// MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE  : Samples (ie, audio frames) are fixed size.
	// MFT_OUTPUT_STREAM_LAZY_READ
	pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES | MFT_INPUT_STREAM_PROCESSES_IN_PLACE | MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE;

	pStreamInfo->cbSize = 0;
	pStreamInfo->cbMaxLookahead = 0;
	pStreamInfo->cbAlignment = 0;

	AutoLock lock(m_CriticSection);

	// When the media type is set, return the minimum buffer size = one audio frame.
	if(dwInputStreamID == 0 && m_bInput0TypeSet){

		pStreamInfo->cbSize = BlockAlign();
	}
	else if(dwInputStreamID == 1 && m_bInput1TypeSet){

		pStreamInfo->cbSize = BlockAlign();
	}

	return S_OK;
}

HRESULT CMFTWaveMixer::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

	if(dwOutputStreamID != 0){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	AutoLock lock(m_CriticSection);

	// MFT_OUTPUT_STREAM_WHOLE_SAMPLES : Output buffers contain complete audio frames.
	// MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES : The MFT can allocate output buffers, or use caller-allocated buffers.
	// MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE : Samples (ie, audio frames) are fixed size.
	// MFT_OUTPUT_STREAM_LAZY_READ
	// MFT_OUTPUT_STREAM_DISCARDABLE
	pStreamInfo->dwFlags = /*MFT_OUTPUT_STREAM_LAZY_READ |*/ MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;

	// If no media type is set, use zero.
	pStreamInfo->cbSize = 0;
	pStreamInfo->cbAlignment = 0;

	// When the media type is set, return the minimum buffer size = one audio frame.
	if(m_bOutputTypeSet){
		pStreamInfo->cbSize = BlockAlign();
	}

	return S_OK;
}

HRESULT CMFTWaveMixer::GetAttributes(IMFAttributes**){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::GetInputStreamAttributes(DWORD, IMFAttributes**){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::GetOutputStreamAttributes(DWORD, IMFAttributes**){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::DeleteInputStream(DWORD){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::AddInputStreams(DWORD, DWORD*){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::GetInputAvailableType(DWORD dwInputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	if(ppType == NULL){
		return E_INVALIDARG;
	}

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	if(dwTypeIndex > 0){
		return MF_E_NO_MORE_TYPES;
	}

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	// If the output type is set, return that type as our preferred input type.
	if(m_pAudioType || m_bInput0TypeSet){

		*ppType = m_pAudioType;
		(*ppType)->AddRef();
	}
	else{

		// Input streamId 1 can be set first. It depends on the media session.
		if(m_bInput1TypeSet){

			*ppType = m_pInput1AudioType;
			(*ppType)->AddRef();
		}
		else{

			// The output type is not set. Create a partial media type.
			hr = GetMinimalType(ppType);
		}
	}

	return hr;
}

HRESULT CMFTWaveMixer::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

	if(ppType == NULL){
		return E_INVALIDARG;
	}

	if(dwOutputStreamID != 0){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	if(dwTypeIndex > 0){
		return MF_E_NO_MORE_TYPES;
	}

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	// If the input type is set, return that type as our preferred output type.
	if(m_pAudioType){

		*ppType = m_pAudioType;
		(*ppType)->AddRef();
	}
	else{

		// Input streamId 1 can be set first. It depends on the media session.
		if(m_pInput1AudioType){

			*ppType = m_pInput1AudioType;
			(*ppType)->AddRef();
		}
		else{

			// The input type is not set. Create a partial media type.
			hr = GetMinimalType(ppType);
		}
	}

	return hr;
}

HRESULT CMFTWaveMixer::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	// Validate flags.
	if(dwFlags & ~MFT_SET_TYPE_TEST_ONLY){
		return E_INVALIDARG;
	}

	if((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL)){
		return E_INVALIDARG;
	}

	AutoLock lock(m_CriticSection);

	// If we have output, the client cannot change the type now.
	if(dwInputStreamID == 0 && m_pInput0Buffer != NULL){
		return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	}
	else if(dwInputStreamID == 1 && m_pInput1Buffer != NULL){
		return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	}

	HRESULT hr = S_OK;

	// Does the caller want us to set the type, or just test it?
	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	// Validate the type.
	if(pType){
		IF_FAILED_RETURN(hr = OnCheckAudioType(pType, dwInputStreamID));
	}

	// The type is OK. Set or clear the type, unless the caller was just testing.
	if(bReallySet){

		hr = OnSetMediaType(pType, InputStream, dwInputStreamID);
	}

	return hr;
}

HRESULT CMFTWaveMixer::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

	if(dwOutputStreamID != 0){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	// Validate flags.
	if(dwFlags & ~MFT_SET_TYPE_TEST_ONLY){
		return E_INVALIDARG;
	}

	if((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL)){
		return E_INVALIDARG;
	}

	AutoLock lock(m_CriticSection);

	// If we have output, the client cannot change the type now.
	if(m_pInput0Buffer != NULL || m_pInput1Buffer != NULL){
		return MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
	}

	HRESULT hr = S_OK;

	// Does the caller want us to set the type, or just test it?
	BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

	// Validate the type.
	if(pType){
		IF_FAILED_RETURN(hr = OnCheckAudioType(pType, 0));
	}

	// The type is OK. Set or clear the type, unless the caller was just testing.
	if(bReallySet){

		hr = OnSetMediaType(pType, OutputStream, dwOutputStreamID);
	}

	return hr;
}

HRESULT CMFTWaveMixer::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

	if(ppType == NULL){
		return E_POINTER;
	}

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	AutoLock lock(m_CriticSection);

	if(dwInputStreamID == 0){

		if(!m_bInput0TypeSet){

			return MF_E_TRANSFORM_TYPE_NOT_SET;
		}
		else{

			*ppType = m_pAudioType;
			(*ppType)->AddRef();
		}
	}
	else if(dwInputStreamID == 1){

		if(!m_bInput1TypeSet){

			return MF_E_TRANSFORM_TYPE_NOT_SET;
		}
		else{

			*ppType = m_pInput1AudioType;
			(*ppType)->AddRef();
		}
	}

	return S_OK;
}

HRESULT CMFTWaveMixer::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

	if(ppType == NULL){
		return E_POINTER;
	}

	if(dwOutputStreamID != 0){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	AutoLock lock(m_CriticSection);

	if(!m_bOutputTypeSet){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	*ppType = m_pAudioType;
	(*ppType)->AddRef();

	return S_OK;
}

HRESULT CMFTWaveMixer::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

	if(pdwFlags == NULL){
		return E_POINTER;
	}

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	AutoLock lock(m_CriticSection);

	IMFMediaBuffer* pBuffer;

	if(dwInputStreamID == 0){
		pBuffer = m_pInput0Buffer;
	}
	else{
		pBuffer = m_pInput1Buffer;
	}

	// If we have output data to give to the client, then we don't accept
	// new input until the client calls ProcessOutput or Flush.
	if(pBuffer != NULL){
		*pdwFlags = 0;
	}
	else{
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
	}

	return S_OK;
}

HRESULT CMFTWaveMixer::GetOutputStatus(DWORD* pdwFlags){

	if(pdwFlags == NULL){
		return E_POINTER;
	}

	AutoLock lock(m_CriticSection);

	if(m_pInput0Buffer != NULL && m_pInput1Buffer != NULL){
		*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
	}
	else{
		*pdwFlags = 0;
	}

	return S_OK;
}

HRESULT CMFTWaveMixer::SetOutputBounds(LONGLONG, LONGLONG){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::ProcessEvent(DWORD, IMFMediaEvent*){

	return E_NOTIMPL;
}

HRESULT CMFTWaveMixer::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;

	switch(eMessage){

	case MFT_MESSAGE_COMMAND_FLUSH:
		OnFlushOrDrain();
		break;

	case MFT_MESSAGE_COMMAND_DRAIN:
		OnFlushOrDrain();
		break;

	case MFT_MESSAGE_SET_D3D_MANAGER:
		// The pipeline should never send this message unless the MFT
		// has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
		// do get this message, it's invalid and we don't implement it.
		hr = E_NOTIMPL;
		break;

	case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
		hr = OnBeginStream();
		break;

	case MFT_MESSAGE_NOTIFY_END_STREAMING:
		ReleaseInputBuffer();
		break;

	case MFT_MESSAGE_DROP_SAMPLES:
	case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
	case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
	case MFT_MESSAGE_COMMAND_MARKER:
		break;
	}

	return hr;
}

HRESULT CMFTWaveMixer::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

	if(pSample == NULL){
		return E_POINTER;
	}

	if(!IsValidInputStream(dwInputStreamID)){
		return MF_E_INVALIDSTREAMNUMBER;
	}

	if(dwFlags != 0){
		return E_INVALIDARG;
	}

	AutoLock lock(m_CriticSection);

	if(!m_bInput0TypeSet || !m_bInput1TypeSet || !m_bOutputTypeSet){
		return MF_E_NOTACCEPTING;
	}

	if(dwInputStreamID == 0 && m_pInput0Buffer != NULL){
		return MF_E_NOTACCEPTING;
	}
	else if(dwInputStreamID == 1 && m_pInput1Buffer != NULL){
		return MF_E_NOTACCEPTING;
	}

	HRESULT hr = S_OK;
	DWORD dwBufferCount = 0;

	// Allocate resources, in case the client did not send the MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.
	// The tool i provide send this message.
	//IF_FAILED_RETURN(hr = AllocateStreamingResources());

	IF_FAILED_RETURN(hr = pSample->GetBufferCount(&dwBufferCount));

	if(dwInputStreamID == 0){

		IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&m_pInput0Buffer));
		m_pSample0 = pSample;
		m_pSample0->AddRef();
	}
	else{

		IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&m_pInput1Buffer));
		m_pSample1 = pSample;
		m_pSample1->AddRef();
	}

	LONGLONG hnsTime = 0;

	if(SUCCEEDED(pSample->GetSampleTime(&hnsTime))){

		if(dwInputStreamID == 0){
			m_bValidTime = TRUE;
			m_rtTimestamp = hnsTime;
		}
	}
	else{

		if(dwInputStreamID == 0)
			m_bValidTime = FALSE;
	}

	return hr;
}

HRESULT CMFTWaveMixer::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

	if(dwFlags != 0){
		return E_INVALIDARG;
	}

	if(pOutputSamples == NULL || pdwStatus == NULL){
		return E_POINTER;
	}

	if(cOutputBufferCount != 1){
		return E_INVALIDARG;
	}

	AutoLock lock(m_CriticSection);

	if(m_pInput0Buffer == NULL || m_pInput1Buffer == NULL){
		return MF_E_TRANSFORM_NEED_MORE_INPUT;
	}

	HRESULT hr = S_OK;

	IMFMediaBuffer* pOutputBuffer = NULL;
	IMFSample* pOutputSample = NULL;

	BYTE* pbOutputData = NULL;
	DWORD cbOutputLength = 0;

	try{

		IF_FAILED_THROW(hr = m_pInput0Buffer->Lock(&m_pbInput0Data, NULL, &m_dwInput0Length));
		IF_FAILED_THROW(hr = m_pInput1Buffer->Lock(&m_pbInput1Data, NULL, &m_dwInput1Length));

		// Just in case
		if(m_dwInput0Length != m_dwInput1Length){
			throw E_UNEXPECTED;
		}

		// We provide sample
		IF_FAILED_THROW(hr = MFCreateSample(&pOutputSample));
		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(m_dwInput0Length, &pOutputBuffer));
		IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(m_dwInput0Length));
		IF_FAILED_THROW(hr = pOutputSample->AddBuffer(pOutputBuffer));

		pOutputSamples[0].pSample = pOutputSample;
		pOutputSamples[0].pSample->AddRef();

		IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pbOutputData, &cbOutputLength, NULL));

		MixAudio(m_pbInput0Data, m_pbInput1Data, pbOutputData, m_dwInput0Length / BlockAlign());

		if(m_bValidTime){

			// Estimate how far along we are...
			LONGLONG hnsDuration = (m_dwInput0Length / AvgBytesPerSec()) * ONE_SECOND;

			// Set the time stamp and duration on the output sample.
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleTime(m_rtTimestamp));
			IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleDuration(hnsDuration));

			m_rtTimestamp += hnsDuration;
		}

		pOutputSamples[0].dwStatus = 0;
		*pdwStatus = 0;
	}
	catch(HRESULT){ LOG_HRESULT(hr); }

	SAFE_RELEASE(pOutputSample);

	if(pOutputBuffer && pbOutputData){
		pOutputBuffer->Unlock();
	}

	SAFE_RELEASE(pOutputBuffer);

	ReleaseInputBuffer();

	return hr;
}