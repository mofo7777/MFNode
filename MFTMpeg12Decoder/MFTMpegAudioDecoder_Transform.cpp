//-----------------------------------------------------------------------------------------------
// MFTMpegAudioDecoder_Transform.cpp
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
//-----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CMFTMpegAudioDecoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

		TRACE_TRANSFORM((L"Transform::GetStreamLimits"));

		IF_FAILED_RETURN(((pdwInputMinimum == NULL) || (pdwInputMaximum == NULL) || (pdwOutputMinimum == NULL) || (pdwOutputMaximum == NULL)) ? E_POINTER : S_OK);

		// This MFT has a fixed number of streams.
		*pdwInputMinimum = 1;
		*pdwInputMaximum = 1;
		*pdwOutputMinimum = 1;
		*pdwOutputMaximum = 1;

		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

		TRACE_TRANSFORM((L"Transform::GetStreamCount"));
		
		IF_FAILED_RETURN(((pcInputStreams == NULL) || (pcOutputStreams == NULL)) ? E_POINTER : S_OK);

		// This MFT has a fixed number of streams.
		*pcInputStreams = 1;
		*pcOutputStreams = 1;

		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"Transform::GetInputStreamInfo"));

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);

		AutoLock lock(m_CriticSection);

		pStreamInfo->hnsMaxLatency = 0;
		pStreamInfo->dwFlags = MFT_INPUT_STREAM_DOES_NOT_ADDREF;

		pStreamInfo->cbSize = 0;
		pStreamInfo->cbMaxLookahead = 0;
		pStreamInfo->cbAlignment = 0;

		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::GetOutputStreamInfo(DWORD dwInputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"Transform::GetOutputStreamInfo"));

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);

		AutoLock lock(m_CriticSection);

		pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;

		pStreamInfo->cbSize = 0;
		pStreamInfo->cbAlignment = 0;
		
		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::GetInputAvailableType(DWORD dwInputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"Transform::GetInputAvailableType"));
		
		IF_FAILED_RETURN(dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK);
		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(ppType == NULL ? E_INVALIDARG : S_OK);

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		// If the input type is set, return that type as our preferred input type.
		if(m_pInputType){

				*ppType = m_pInputType;
				(*ppType)->AddRef();
		}
		else{
				
				// The input type is not set. Create a partial media type.
				LOG_HRESULT(hr = GetInputType(ppType));
		}
		return hr;
}

HRESULT CMFTMpegAudioDecoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"Transform::GetOutputAvailableType"));

		IF_FAILED_RETURN(dwTypeIndex != 0 ? MF_E_NO_MORE_TYPES : S_OK);
		IF_FAILED_RETURN(dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(ppType == NULL ? E_INVALIDARG : S_OK);

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		if(m_pInputType == NULL){
				LOG_HRESULT(hr = MF_E_TRANSFORM_TYPE_NOT_SET);
		}

		// If the output type is set, return that type as our preferred output type.
		if(m_pOutputType){

				*ppType = m_pOutputType;
				(*ppType)->AddRef();
		}
		else{
				
				LOG_HRESULT(hr = GetOutputType(ppType));
		}
		return hr;
}

HRESULT CMFTMpegAudioDecoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"Transform::SetInputType"));

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL)) ? E_INVALIDARG : S_OK);

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		// If we have output, the client cannot change the type now.
		if(m_HasAudio){
				IF_FAILED_RETURN(hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING);
		}

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		if(pType){
				LOG_HRESULT(hr = OnCheckInputType(pType));
		}

		if(SUCCEEDED(hr)){

				if(bReallySet){

						// The type is OK. Set the type, unless the caller was just testing.
						SAFE_RELEASE(m_pInputType);

						if(pType){

								// Store the media type.
								m_pInputType = pType;
								m_pInputType->AddRef();
						}
				}
		}
		return hr;
}

HRESULT CMFTMpegAudioDecoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"Transform::SetOutputType"));

		IF_FAILED_RETURN(dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(((dwFlags & MFT_SET_TYPE_TEST_ONLY) && (pType == NULL)) ? E_INVALIDARG : S_OK);

		AutoLock lock(m_CriticSection);
		
		HRESULT hr = S_OK;

		// If we have output, the client cannot change the type now.
		if(m_HasAudio){
				IF_FAILED_RETURN(hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING);
		}

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		if(pType){
				LOG_HRESULT(hr = OnCheckOutputType(pType));
		}

		if(SUCCEEDED(hr)){

				if(bReallySet){

						// The type is OK. Set the type, unless the caller was just testing.
						SAFE_RELEASE(m_pOutputType);

						if(pType){

								// Store the media type.
								m_pOutputType = pType;
								m_pOutputType->AddRef();
						}
				}
		}
		return hr;
}

HRESULT CMFTMpegAudioDecoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"Transform::GetInputCurrentType"));

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(ppType == NULL ? E_POINTER : S_OK);

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		if(!m_pInputType){
				IF_FAILED_RETURN(hr = MF_E_TRANSFORM_TYPE_NOT_SET);
		}

		*ppType = m_pInputType;
		(*ppType)->AddRef();

		return hr;
}

HRESULT CMFTMpegAudioDecoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"Transform::GetOutputCurrentType"));

		IF_FAILED_RETURN(dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(ppType == NULL ? E_POINTER : S_OK);

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		if(!m_pOutputType){
				IF_FAILED_RETURN(hr = MF_E_TRANSFORM_TYPE_NOT_SET);
		}

		*ppType = m_pOutputType;
		(*ppType)->AddRef();

		return hr;
}

HRESULT CMFTMpegAudioDecoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

		TRACE_TRANSFORM((L"Transform::GetInputStatus"));

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(pdwFlags == NULL ? E_POINTER : S_OK);

		AutoLock lock(m_CriticSection);

		// If we have output data to give to the client, then we don't accept
		// new input until the client calls ProcessOutput or Flush.
		if(m_HasAudio){
				*pdwFlags = 0;
		}
		else{
				*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
		}

		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::GetOutputStatus(DWORD* pdwFlags){

		TRACE_TRANSFORM((L"Transform::GetOutputStatus"));

		IF_FAILED_RETURN(pdwFlags == NULL ? E_POINTER : S_OK);

		AutoLock lock(m_CriticSection);

		if(m_HasAudio){
				*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
		}
		else{
				*pdwFlags = 0;
		}

		return S_OK;
}

HRESULT CMFTMpegAudioDecoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

		TRACE_TRANSFORM((L"Transform::ProcessMessage : %s", MFTMessageString(eMessage)));

  AutoLock lock(m_CriticSection);
		
		HRESULT hr = S_OK;

		switch(eMessage){
		
		  case MFT_MESSAGE_COMMAND_FLUSH:
						// Flush the MFT.
						OnFlush();
						break;

				case MFT_MESSAGE_COMMAND_DRAIN:
						// Set the discontinuity flag on all of the input.
						OnDiscontinuity();
						break;

				case MFT_MESSAGE_SET_D3D_MANAGER:
						// The pipeline should never send this message unless the MFT
						// has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
						// do get this message, it's invalid and we don't implement it.
						hr = E_NOTIMPL;
						break;

				case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
						LOG_HRESULT(hr = AllocateStreamingResources());
						break;

				case MFT_MESSAGE_NOTIFY_END_STREAMING:
						FreeStreamingResources();
						break;

				// These messages do not require a response.
				case MFT_MESSAGE_NOTIFY_START_OF_STREAM: 
				case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
						break;        
		}

		return hr;
}

HRESULT CMFTMpegAudioDecoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

		TRACE_TRANSFORM((L"Transform::ProcessInput"));

		HRESULT hr = S_OK;

		IF_FAILED_RETURN(dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(pSample == NULL ? E_POINTER : S_OK);
		IF_FAILED_RETURN(dwFlags != 0 ? E_INVALIDARG : S_OK);

		AutoLock lock(m_CriticSection);

		//if(!m_pInputType || !m_pOutputType){
		if(!m_pOutputType){
				
				// The client must set the input and output types before calling ProcessInput.
				//IF_FAILED_RETURN(hr = MF_E_TRANSFORM_TYPE_NOT_SET);
				return MF_E_TRANSFORM_TYPE_NOT_SET;
		}

		if(m_HasAudio){
				
				// Not accepting input because there is still data to process.
				//IF_FAILED_RETURN(hr = MF_E_NOTACCEPTING);
				return MF_E_NOTACCEPTING;
		}

		DWORD dwBufferCount = 0;
		IMFMediaBuffer* pBuffer = NULL;

		BYTE* pbInputData = NULL;
		DWORD dwInputLength = 0;

		BYTE* pbTmpData = NULL;

		try{

				// Allocate resources, in case the client did not send the MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.
				IF_FAILED_THROW(hr = AllocateStreamingResources());

				// Get the input buffer(s) from the sample.
				IF_FAILED_THROW(hr = pSample->GetBufferCount(&dwBufferCount));

				assert(dwBufferCount == 1);

				// Convert to a single contiguous buffer. NOTE: This does not cause a copy unless there are multiple buffers
				IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));
				IF_FAILED_THROW(hr = pBuffer->Lock(&pbInputData, NULL, &dwInputLength));

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwInputLength, &m_pInputBuffer));

				IF_FAILED_THROW(hr = m_pInputBuffer->Lock(&pbTmpData, NULL, NULL));
				memcpy(pbTmpData, pbInputData, dwInputLength);

				IF_FAILED_THROW(hr = m_pInputBuffer->Unlock());
				pbTmpData = NULL;

				IF_FAILED_THROW(hr = pBuffer->Unlock());
				pbInputData = NULL;

				IF_FAILED_THROW(hr = m_pInputBuffer->SetCurrentLength(dwInputLength));

				if(m_pLastInputBuffer != NULL){
						IF_FAILED_THROW(hr = GetFullBuffer());
				}

				if(SUCCEEDED(pSample->GetSampleTime(&m_rtTime))){
						m_bValidTime = TRUE;

      #ifdef TRACE_REFERENCE_TIME
						  TRACE((L"GetSampleTime : %I64d", m_rtTime));
      #endif
				}
				else{
						m_bValidTime = FALSE;

						#ifdef TRACE_REFERENCE_TIME
						  TRACE((L"No Time"));
      #endif
				}
		}
		catch(HRESULT){}

		if(pBuffer && pbInputData != NULL){
				LOG_HRESULT(pBuffer->Unlock());
		}

		if(m_pInputBuffer && pbTmpData != NULL){
				LOG_HRESULT(m_pInputBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::ProcessOutput(DWORD dwFlags , DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

		TRACE_TRANSFORM((L"Transform::ProcessOutput"));
		
		HRESULT hr = S_OK;

		// No lazy/optional stream
		IF_FAILED_RETURN(dwFlags != 0 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK);
		IF_FAILED_RETURN(cOutputBufferCount != 1 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].dwStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].pSample != NULL ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].dwStatus != 0 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].pEvents != NULL ? E_INVALIDARG : S_OK);
		*pdwStatus = 0;
		
		AutoLock lock(m_CriticSection);
		
		if(m_pInputBuffer == NULL){
				//IF_FAILED_RETURN(hr = MF_E_TRANSFORM_NEED_MORE_INPUT);
				return MF_E_TRANSFORM_NEED_MORE_INPUT;
		}

		DWORD dwCurInputIndex = 0;

		IMFSample* pSample = NULL;

		BYTE* pbInputData = NULL;
		DWORD dwInputLength = 0;

		UINT uiFrameSize = 0;
		UINT uiDecodedSize = 0;

		try{

				IF_FAILED_THROW(hr = MFCreateSample(&pSample));

				IF_FAILED_THROW(hr = m_pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

				while(hr == S_OK){

						IF_FAILED_THROW(hr = m_cMpeg12Decoder.DecodeFrame(pbInputData + dwCurInputIndex, dwInputLength - dwCurInputIndex, &uiFrameSize, m_sAudioBuffer, &uiDecodedSize));

						assert(uiDecodedSize <= MAX_AUDIO_BUFFER_SIZE);

						if(hr == S_FALSE){

								IF_FAILED_THROW(hr = BufferKeepLastByte(pbInputData + dwCurInputIndex, dwInputLength - dwCurInputIndex));

								IF_FAILED_THROW(hr = m_pInputBuffer->Unlock());
								pbInputData = NULL;

								SAFE_RELEASE(m_pInputBuffer);

								if(m_HasAudio){

										m_HasAudio = FALSE;

										IF_FAILED_THROW(hr = ConvertOutput(pSample));

										pOutputSamples[0].pSample = m_pAudioSample;
										pOutputSamples[0].pSample->AddRef();
								}
								/*else{

										m_HasAudio = FALSE;
								}*/

								break;
						}
						else{

								dwCurInputIndex += uiFrameSize;
								m_HasAudio = TRUE;

								IF_FAILED_THROW(hr = AddOutputBuffer(pSample, uiDecodedSize));
						}
				}
		}
		catch(HRESULT){}

		if(m_pInputBuffer && pbInputData){

				LOG_HRESULT(m_pInputBuffer->Unlock());
		}

		// Does it release all buffers ?
		SAFE_RELEASE(pSample);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::AddOutputBuffer(IMFSample* pSample, const UINT uiDecodedSize){

		TRACE_TRANSFORM((L"Transform::CreateOutputBuffer"));

		HRESULT hr = S_OK;
		IMFMediaBuffer* pBuffer = NULL;
		BYTE* pData = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(uiDecodedSize, &pBuffer));
				
				IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

				CopyMemory(pData, m_sAudioBuffer, uiDecodedSize);

				IF_FAILED_THROW(hr = pBuffer->Unlock());
				pData = NULL;

				IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(uiDecodedSize));

				IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));
		}
		catch(HRESULT){}

		if(pBuffer && pData){

				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::ConvertOutput(IMFSample* pSample){

		HRESULT hr = S_OK;
		DWORD dwBufferCount = 0;
		IMFMediaBuffer* pBuffer = NULL;
		DWORD dwSize = 0;
		BYTE* bt = NULL;

		try{

				// Get the input buffer(s) from the sample.
				IF_FAILED_THROW(hr = pSample->GetBufferCount(&dwBufferCount));

				assert(dwBufferCount != 0);

				// Convert to a single contiguous buffer. NOTE: This does not cause a copy unless there are multiple buffers
				IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));

				IF_FAILED_THROW(hr = pBuffer->Lock(&bt, NULL, &dwSize));

				//TRACE((L"Count = %d - Size = %d", dwBufferCount, dwSize));

				IF_FAILED_THROW(hr = pBuffer->Unlock());
				bt = NULL;

				IF_FAILED_THROW(hr = m_pAudioSample->RemoveAllBuffers());

				IF_FAILED_THROW(hr = m_pAudioSample->AddBuffer(pBuffer));
				
				LONGLONG llDuration = (LONGLONG)dwSize * 10000000 / m_uiAvgRate;
				IF_FAILED_THROW(hr = m_pAudioSample->SetSampleDuration(llDuration));

				if(m_bValidTime){

						IF_FAILED_THROW(hr = m_pAudioSample->SetSampleTime(m_rtTime));

						#ifdef TRACE_REFERENCE_TIME
						  TRACE((L"SetSampleTime : %I64d", m_rtTime));
      #endif
				}
				else{

						IF_FAILED_THROW(hr = m_pAudioSample->SetSampleTime(m_rtTime));

						#ifdef TRACE_REFERENCE_TIME
						  TRACE((L"No Time : %I64d", m_rtTime));
      #endif

						m_rtTime += llDuration;
				}
		}
		catch(HRESULT){}

		if(pBuffer && bt){
				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::GetFullBuffer(){

		HRESULT hr = S_OK;

		BYTE* pbInputData = NULL;
		BYTE* pbLastInputData = NULL;
		BYTE* pbNewInputData = NULL;
		DWORD dwInputLength = 0;
		DWORD dwLastInputLength = 0;

		IMFMediaBuffer* pBuffer = NULL;

		try{

				IF_FAILED_THROW(hr = m_pLastInputBuffer->Lock(&pbLastInputData, NULL, &dwLastInputLength));

				if(dwLastInputLength != 0){

						IF_FAILED_THROW(hr = m_pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));
						
						IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwInputLength + dwLastInputLength, &pBuffer));
						IF_FAILED_THROW(hr = pBuffer->Lock(&pbNewInputData, NULL, NULL));
						
						memcpy(pbNewInputData, pbLastInputData, dwLastInputLength);
						memcpy(pbNewInputData + dwLastInputLength, pbInputData, dwInputLength);

						IF_FAILED_THROW(hr = pBuffer->Unlock());
						pbNewInputData = NULL;

						IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwInputLength + dwLastInputLength));

						IF_FAILED_THROW(hr = m_pInputBuffer->Unlock());
						pbInputData = NULL;

						SAFE_RELEASE(m_pInputBuffer);

						m_pInputBuffer = pBuffer;
						m_pInputBuffer->AddRef();
				}

				IF_FAILED_THROW(hr = m_pLastInputBuffer->Unlock());
				pbLastInputData = NULL;
		}
		catch(HRESULT){}

		if(pBuffer && pbNewInputData){
				LOG_HRESULT(hr = pBuffer->Unlock());
		}

		if(m_pLastInputBuffer && pbLastInputData){
				LOG_HRESULT(hr = m_pLastInputBuffer->Unlock());
		}

		if(m_pInputBuffer && pbInputData){
				LOG_HRESULT(hr = m_pInputBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);
		SAFE_RELEASE(m_pLastInputBuffer);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::BufferKeepLastByte(const BYTE* pData, const DWORD dwLenght){

		assert(m_pLastInputBuffer == NULL);

		HRESULT hr = S_OK;
		BYTE* pbKeepData = NULL;

		if(dwLenght == 0)
				return hr;

		try{

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwLenght, &m_pLastInputBuffer));

				IF_FAILED_THROW(hr = m_pLastInputBuffer->Lock(&pbKeepData, NULL, NULL));

				CopyMemory(pbKeepData, pData, dwLenght);

				IF_FAILED_THROW(hr = m_pLastInputBuffer->Unlock());
				pbKeepData = NULL;

				IF_FAILED_THROW(hr = m_pLastInputBuffer->SetCurrentLength(dwLenght));
		}
		catch(HRESULT){}

		if(m_pLastInputBuffer && pbKeepData){
				LOG_HRESULT(hr = m_pLastInputBuffer->Unlock());
		}

		return hr;
}