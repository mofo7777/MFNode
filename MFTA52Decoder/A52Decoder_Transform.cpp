//----------------------------------------------------------------------------------------------
// A52Decoder_Transform.cpp
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

HRESULT CA52Decoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

		TRACE_TRANSFORM((L"A52Decoder::GetStreamLimits"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

		*pdwInputMinimum = 1;
		*pdwInputMaximum = 1;
		*pdwOutputMinimum = 1;
		*pdwOutputMaximum = 1;

		return hr;
}

HRESULT CA52Decoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

		TRACE_TRANSFORM((L"A52Decoder::GetStreamCount"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

		*pcInputStreams = 1;
		*pcOutputStreams = 1;

		return hr;
}

HRESULT CA52Decoder::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

		TRACE_TRANSFORM((L"A52Decoder::GetStreamIDs"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"A52Decoder::GetInputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		
		pStreamInfo->dwFlags        = MFT_INPUT_STREAM_WHOLE_SAMPLES;
		pStreamInfo->hnsMaxLatency  = 0;
		pStreamInfo->cbSize         = 0;
		pStreamInfo->cbMaxLookahead = 0;
		pStreamInfo->cbAlignment    = 0;
    
		return hr;
}

HRESULT CA52Decoder::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"A52Decoder::GetOutputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_PROVIDES_SAMPLES;
		pStreamInfo->cbSize = 0;
		pStreamInfo->cbAlignment = 0;
    
		return hr;
}

HRESULT CA52Decoder::GetAttributes(IMFAttributes**){

		TRACE_TRANSFORM((L"A52Decoder::GetAttributes"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::GetInputStreamAttributes(DWORD, IMFAttributes**){

		TRACE_TRANSFORM((L"A52Decoder::GetInputStreamAttributes"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::GetOutputStreamAttributes(DWORD, IMFAttributes**){

		TRACE_TRANSFORM((L"A52Decoder::GetOutputStreamAttributes"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::DeleteInputStream(DWORD){

		TRACE_TRANSFORM((L"A52Decoder::DeleteInputStream"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::AddInputStreams(DWORD, DWORD*){

		TRACE_TRANSFORM((L"A52Decoder::AddInputStreams"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::GetInputAvailableType(DWORD, DWORD, IMFMediaType**){

		TRACE_TRANSFORM((L"A52Decoder::GetInputAvailableType"));

		return MF_E_NO_MORE_TYPES;
}

HRESULT CA52Decoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"A52Decoder::GetOutputAvailableType"));

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

HRESULT CA52Decoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"A52Decoder::SetInputType"));

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

HRESULT CA52Decoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"A52Decoder::SetOutputType"));

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

HRESULT CA52Decoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"A52Decoder::GetInputCurrentType"));

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

HRESULT CA52Decoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"A52Decoder::GetOutputCurrentType"));

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

HRESULT CA52Decoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

		TRACE_TRANSFORM((L"A52Decoder::GetInputStatus"));

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

HRESULT CA52Decoder::GetOutputStatus(DWORD* pdwFlags){

		TRACE_TRANSFORM((L"A52Decoder::GetOutputStatus"));

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

HRESULT CA52Decoder::SetOutputBounds(LONGLONG, LONGLONG){

		TRACE_TRANSFORM((L"A52Decoder::SetOutputBounds"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::ProcessEvent(DWORD, IMFMediaEvent*){

		TRACE_TRANSFORM((L"A52Decoder::ProcessEvent"));

		return E_NOTIMPL;
}

HRESULT CA52Decoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

		TRACE_TRANSFORM((L"A52Decoder::ProcessMessage : %s", MFTMessageString(eMessage)));

  HRESULT hr = S_OK;
		
		AutoLock lock(m_CriticSection);

		switch(eMessage){
				
		  case MFT_MESSAGE_COMMAND_FLUSH:
				case MFT_MESSAGE_NOTIFY_END_STREAMING:
				case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
						OnFlush();
						break;

				case MFT_MESSAGE_COMMAND_DRAIN:
						m_bDraining = TRUE;
						break;

				case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
				case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
						OnStartStreaming();
						break;

				case MFT_MESSAGE_SET_D3D_MANAGER:
						hr = E_NOTIMPL;
						break;
		}

		return hr;
}

HRESULT CA52Decoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

		TRACE_TRANSFORM((L"A52Decoder::ProcessInput"));

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

		REFERENCE_TIME rtSampleTime;

		if(SUCCEEDED(pSample->GetSampleTime(&rtSampleTime))){

				m_rtSampleTime = rtSampleTime;
    m_bValidTime = TRUE;
  }
  else{
    m_bValidTime = FALSE;
  }

		m_pInputSample = pSample;
		m_pInputSample->AddRef();

		return hr;
}

HRESULT CA52Decoder::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

		TRACE_TRANSFORM((L"A52Decoder::ProcessOutput"));

		HRESULT hr;
		IF_FAILED_RETURN(dwFlags != 0 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK);
		IF_FAILED_RETURN(cOutputBufferCount != 1 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].dwStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].pSample != NULL ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].dwStatus != 0 ? E_INVALIDARG : S_OK);
		IF_FAILED_RETURN(pOutputSamples[0].pEvents != NULL ? E_INVALIDARG : S_OK);
		*pdwStatus = 0;

		AutoLock lock(m_CriticSection);

		if(m_A52State == NULL){
				return E_UNEXPECTED;
		}

		if(m_pInputSample == NULL){
				return MF_E_TRANSFORM_NEED_MORE_INPUT;
		}

		DWORD dwBufferCount = 0;
		IF_FAILED_RETURN(hr = m_pInputSample->GetBufferCount(&dwBufferCount));

		assert(dwBufferCount == 1);

		IMFMediaBuffer* pInputBuffer = NULL;
		BYTE* pbInputData = NULL;
		DWORD dwInputLength;

		IMFSample* pSample = NULL;

		try{

				IF_FAILED_THROW(hr = m_pInputSample->GetBufferByIndex(0, &pInputBuffer));
				IF_FAILED_THROW(hr = pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

				IF_FAILED_THROW(hr = MFCreateSample(&pSample));

				IF_FAILED_THROW(hr = DecodeA52(pSample, pbInputData, dwInputLength));

				IF_FAILED_THROW(hr = ConvertOutput(pSample));

    pOutputSamples[0].pSample = pSample;
    pOutputSamples[0].pSample->AddRef();
		}
		catch(HRESULT){}

		if(pInputBuffer && pbInputData){
				LOG_HRESULT(pInputBuffer->Unlock());
		}

		SAFE_RELEASE(pInputBuffer);
		SAFE_RELEASE(pSample);
		SAFE_RELEASE(m_pInputSample);

		return hr;
}

HRESULT CA52Decoder::DecodeA52(IMFSample* pSample, BYTE* pData, const DWORD dwSize){

		TRACE_TRANSFORM((L"A52Decoder::DecodeA52"));

		HRESULT hr = S_OK;
		
		static uint8_t buf[3840];
  static uint8_t* bufptr = buf;
  static uint8_t* bufpos = buf + 7;

  /*
   * sample_rate and flags are static because this routine could
   * exit between the a52_syncinfo() and the ao_setup(), and we want
   * to have the same values when we get back !
   */

  static int sample_rate;
  static int flags;

		BYTE* start1 = pData;
  BYTE* end1 = pData + dwSize;

  int bit_rate;
  int len;

		//int disable_adjust = 0;
		//int disable_dynrng = 0;

  int length;

		while(1){

    len = end1 - start1;

				if(!len)
	     break;

				if(len > bufpos - bufptr)
	     len = bufpos - bufptr;

				memcpy(bufptr, start1, len);

				bufptr += len;
	   start1 += len;

				if(bufptr == bufpos){

						if(bufpos == buf + 7){

								length = a52_syncinfo(buf, &flags, &sample_rate, &bit_rate);

								if(!length){

										//fprintf (stderr, "skip\n");

										for(bufptr = buf; bufptr < buf + 6; bufptr++)
												bufptr[0] = bufptr[1];

										continue;
								}

								bufpos = buf + length;
						}
						else{

								sample_t level = 1;
								sample_t bias = 0;

								//if(ao_setup (output, sample_rate, &flags, &level, &bias))
		        //goto error;

								//if(!disable_adjust)
		        flags |= A52_ADJUST_LEVEL;

								// level *= gain;

								if(a52_frame(m_A52State, buf, &flags, &level, bias))
										goto error;

								//if(disable_dynrng)
										//a52_dynrng(state, NULL, NULL);

								IF_FAILED_RETURN(hr = AddOutputBuffer(pSample, level));

								/*DWORD dwSizeBuffer = 6 * 256 * (GetAC52Channels(m_A52State->acmod) + m_A52State->lfeon) * sizeof(float);

								for(int i = 0; i < 6 && a52_block(m_A52State) == 0; i++){

										sample_t* samples = a52_samples(m_A52State);

										for(int j = 0; j < 256; j++, samples++)
												for(int ch = 0; ch < 2; ch++)
														*dst++ = float(*(samples + 256 * ch) / level);
								}*/

								bufptr = buf;
		      bufpos = buf + 7;
		      //print_fps (0);
								continue;
	    
        error:
		        //fprintf (stderr, "error\n");
		        bufptr = buf;
		        bufpos = buf + 7;
						}
				}
		}

  return hr;
}

HRESULT CA52Decoder::AddOutputBuffer(IMFSample* pSample, const FLOAT level){

		TRACE_TRANSFORM((L"CA52Decoder::AddOutputBuffer"));

		HRESULT hr = S_OK;
		IMFMediaBuffer* pBuffer = NULL;
		BYTE* pData = NULL;

		try{

				DWORD dwSizeBuffer = 6 * 256 * (GetAC52Channels(m_A52State->acmod) + m_A52State->lfeon) * sizeof(float);

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwSizeBuffer, &pBuffer));
				
				IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

				FLOAT* pfData = (FLOAT*)pData;

				int i = 0;

				for(; i < 6 && a52_block(m_A52State) == 0; i++){

						sample_t* samples = a52_samples(m_A52State);

						for(int j = 0; j < 256; j++, samples++)
								for(int ch = 0; ch < 2; ch++)
										*pfData++ = float(*(samples + 256 * ch) / level);
				}

				assert(i == 6);

				IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwSizeBuffer));

				IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));
		}
		catch(HRESULT){}

		if(pBuffer && pData){

				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CA52Decoder::ConvertOutput(IMFSample* pSample){

		HRESULT hr = S_OK;
		DWORD dwBufferCount = 0;
		IMFMediaBuffer* pBuffer = NULL;
		DWORD dwSize = 0;
		BYTE* btData = NULL;

		try{

				IF_FAILED_THROW(hr = pSample->GetBufferCount(&dwBufferCount));

				assert(dwBufferCount != 0);

				IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));

				IF_FAILED_THROW(hr = pBuffer->Lock(&btData, NULL, &dwSize));
				//TRACE((L"Count = %d - Size = %d", dwBufferCount, dwSize));

				IF_FAILED_THROW(hr = pSample->RemoveAllBuffers());

				IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

				const UINT32 uiBlockAlign = m_uiChannels * 4;
				const UINT32 uiAvgBytePerSec = uiBlockAlign * m_uiSamplePerSec;
				
				LONGLONG llDuration = (LONGLONG)dwSize * 10000000 / uiAvgBytePerSec;
				IF_FAILED_THROW(hr = pSample->SetSampleDuration(llDuration));

				if(m_bValidTime){

						IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtSampleTime));
				}
				else{

						IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtSampleTime));

						m_rtSampleTime += llDuration;
				}
		}
		catch(HRESULT){}

		if(pBuffer && btData){
				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}