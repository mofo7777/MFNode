//----------------------------------------------------------------------------------------------
// MFTCudaDecoder_Transform.cpp
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

HRESULT CMFTCudaDecoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

		TRACE_TRANSFORM((L"MFTCuda::GetStreamLimits"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

		*pdwInputMinimum = 1;
		*pdwInputMaximum = 1;
		*pdwOutputMinimum = 1;
		*pdwOutputMaximum = 1;

		return hr;
}

HRESULT CMFTCudaDecoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

		TRACE_TRANSFORM((L"MFTCuda::GetStreamCount"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

		*pcInputStreams = 1;
		*pcOutputStreams = 1;

		return hr;
}

HRESULT CMFTCudaDecoder::GetStreamIDs(DWORD /*dwInputIDArraySize*/, DWORD* /*pdwInputIDs*/, DWORD /*dwOutputIDArraySize*/, DWORD* /*pdwOutputIDs*/){

		TRACE_TRANSFORM((L"MFTCuda::GetStreamIDs"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"MFTCuda::GetInputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		
		pStreamInfo->dwFlags        = MFT_INPUT_STREAM_DOES_NOT_ADDREF;
		pStreamInfo->hnsMaxLatency  = 0;
		pStreamInfo->cbSize         = 0;
		pStreamInfo->cbMaxLookahead = 0;
		pStreamInfo->cbAlignment    = 0;

		return hr;
}

HRESULT CMFTCudaDecoder::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"MFTCuda::GetOutputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		pStreamInfo->dwFlags        = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;
		pStreamInfo->cbSize         = 0;
		pStreamInfo->cbAlignment    = 0;

		if(m_pOutputType){

				pStreamInfo->cbSize = m_dwSampleSize;
				// Ask Nvidia maximum pitch...
				//pStreamInfo->cbSize = 768 * m_uiFrameHeight * 3 / 2;
		}
    
		return hr;
}

HRESULT CMFTCudaDecoder::GetAttributes(IMFAttributes** /*pAttributes*/){

		TRACE_TRANSFORM((L"MFTCuda::GetAttributes"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::GetInputStreamAttributes(DWORD /*dwInputStreamID*/, IMFAttributes** /*ppAttributes*/){

		TRACE_TRANSFORM((L"MFTCuda::GetInputStreamAttributes"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::GetOutputStreamAttributes(DWORD /*dwOutputStreamID*/, IMFAttributes** /*ppAttributes*/){

		TRACE_TRANSFORM((L"MFTCuda::GetOutputStreamAttributes"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::DeleteInputStream(DWORD /*dwStreamID*/){

		TRACE_TRANSFORM((L"MFTCuda::DeleteInputStream"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::AddInputStreams(DWORD /*cStreams*/, DWORD* /*adwStreamIDs*/){

		TRACE_TRANSFORM((L"MFTCuda::AddInputStreams"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::GetInputAvailableType(DWORD /*dwInputStreamID*/, DWORD /*dwTypeIndex*/, IMFMediaType** /*ppType*/){

		TRACE_TRANSFORM((L"MFTCuda::GetInputAvailableType"));

		// TODO : offer a mediatype - major VIDEO - sub MPEG2 (optional)
		return MF_E_NO_MORE_TYPES;

		/*if(ppType == NULL){
				return E_INVALIDARG;
		}

		if(dwInputStreamID != 0){
				return MF_E_INVALIDSTREAMNUMBER;
		}

		// If offer only one mediatype
		if(dwTypeIndex !=0){
		  return MF_E_NO_MORE_TYPES;
  }

		AutoLock lock(m_critSec);

		HRESULT hr = S_OK;

		ppType ---> offer an inputtype

		return hr;*/
}

HRESULT CMFTCudaDecoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTCuda::GetOutputAvailableType"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwTypeIndex != 0 && dwTypeIndex != 1 ? MF_E_NO_MORE_TYPES : S_OK));

		AutoLock lock(m_CriticSection);

		if(m_pInputType == NULL){
				hr = MF_E_TRANSFORM_TYPE_NOT_SET;
		}
		else{
				hr = GetOutputType(ppType, dwTypeIndex);
		}

		return hr;
}

HRESULT CMFTCudaDecoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTCuda::SetInputType"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		IF_FAILED_RETURN(hr = (m_cCudaDecoder.IsInit() ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		if(pType){

				hr = OnCheckInputType(pType);

				if(SUCCEEDED(hr) && bReallySet){

						hr = OnSetInputType(pType);
				}
		}

		return hr;
}

HRESULT CMFTCudaDecoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTCuda::SetOutputType"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		IF_FAILED_RETURN(hr = (m_cCudaDecoder.IsInit() ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		if(pType){

				hr = OnCheckOutputType(pType);

				if(SUCCEEDED(hr) && bReallySet){

						SAFE_RELEASE(m_pOutputType);

						m_pOutputType = pType;
						m_pOutputType->AddRef();
				}
		}

		return hr;
}

HRESULT CMFTCudaDecoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTCuda::GetInputCurrentType"));

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

HRESULT CMFTCudaDecoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTCuda::GetOutputCurrentType"));

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

HRESULT CMFTCudaDecoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

		TRACE_TRANSFORM((L"MFTCuda::GetInputStatus"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		//AutoLock lock(m_CriticSection);

		// I think we can always process
		*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;

		// If we already have an input sample, we don't accept another one until the client calls ProcessOutput or Flush.
		/*if(m_pInputBuffer == NULL){
				*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
		}
		else{
				*pdwFlags = 0;
		}*/

		return hr;
}

HRESULT CMFTCudaDecoder::GetOutputStatus(DWORD* pdwFlags){

		TRACE_TRANSFORM((L"MFTCuda::GetOutputStatus"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		if(HaveSampleOut()){
				*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
		}
		else{
				*pdwFlags = 0;
		}

		return hr;
}

HRESULT CMFTCudaDecoder::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/){

		TRACE_TRANSFORM((L"MFTCuda::SetOutputBounds"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent */){

		TRACE_TRANSFORM((L"MFTCuda::ProcessEvent"));

		return E_NOTIMPL;
}

HRESULT CMFTCudaDecoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

		TRACE_TRANSFORM((L"MFTCuda::ProcessMessage : %s", MFTMessageString(eMessage)));

  HRESULT hr = S_OK;
		
		AutoLock lock(m_CriticSection);

		switch(eMessage){
		
		  case MFT_MESSAGE_COMMAND_FLUSH:
						hr = OnFlush();
						break;

				case MFT_MESSAGE_COMMAND_DRAIN:
						hr = OnDrain();
						break;

				case MFT_MESSAGE_SET_D3D_MANAGER:
						hr = E_NOTIMPL;
						break;

				case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
						hr = BeginStreaming();
						break;

				case MFT_MESSAGE_NOTIFY_END_STREAMING:
						EndStreaming();
						break;

				case MFT_MESSAGE_NOTIFY_START_OF_STREAM: 
				case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
						break;        
		}

		return hr;
}

HRESULT CMFTCudaDecoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTCuda::ProcessInput"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		if(!m_pInputType || !m_pOutputType || HaveSampleOut() || m_bDraining){
				// Client must set input and output types.
				return MF_E_NOTACCEPTING;
		}

		// Get the input buffer(s) from the sample.
		DWORD dwBufferCount = 0;
		IF_FAILED_RETURN(hr = pSample->GetBufferCount(&dwBufferCount));

		// dwBufferCount != 0

		IMFMediaBuffer* pInputBuffer = NULL;
		BYTE* pbInputData = NULL;
		DWORD dwInputLength;

		// Todo : do not ConvertToContiguousBuffer, there is always one buffer...
		IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&pInputBuffer));

		try{

				LONGLONG llPTS = -1;
				LONGLONG llDTS = -1;

				// Get the time stamp. It is OK if this call fails.
				if(SUCCEEDED(pSample->GetSampleTime(&llPTS))){
						
						pSample->GetUINT64(MFSampleExtension_DecodedTimeStamp, (UINT64*)&llDTS);
				}

    #ifdef TRACE_INPUT_REFERENCE
				  TRACE((L"Time = %I64d : DTS = %I64d", llPTS, llDTS));
    #endif

				if(llPTS != -1)
						m_cCudaFrame.SetTime(llPTS, llDTS);

				IF_FAILED_THROW(hr = pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

				IF_FAILED_THROW(hr = Slice(pbInputData, dwInputLength));
		}
		catch(HRESULT){}
		
		if(pInputBuffer && pbInputData){

				LOG_HRESULT(pInputBuffer->Unlock());
		}

		SAFE_RELEASE(pInputBuffer);

		return hr;
}

HRESULT CMFTCudaDecoder::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

		TRACE_TRANSFORM((L"MFTCuda::ProcessOutput"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		if(m_bDraining){

				if(!m_cCudaFrame.IsEmpty()){

						IF_FAILED_RETURN(hr = DecodePicture(m_cCudaFrame.GetNextFrame()));
				}
		}

		if(HaveSampleOut() == FALSE){
				return MF_E_TRANSFORM_NEED_MORE_INPUT;
		}
		
		IMFMediaBuffer* pOutputBuffer = NULL;
		BYTE* pOutputData = NULL;
		DWORD dwOutputLength = 0;

		IMFMediaBuffer* pCurOutputBuf = NULL;
		BYTE* pCurOutputData = NULL;
		DWORD dwCurOutputLength = 0;

		IMFSample* pSample = m_qSampleOut.front();

		try{

				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->GetBufferByIndex(0, &pOutputBuffer));
				IF_FAILED_THROW(hr = pOutputBuffer->GetMaxLength(&dwOutputLength));

				IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pCurOutputBuf));
				IF_FAILED_THROW(hr = pCurOutputBuf->Lock(&pCurOutputData, NULL, &dwCurOutputLength));

				IF_FAILED_THROW(hr = (dwOutputLength < dwCurOutputLength ? E_INVALIDARG : S_OK));

				IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pOutputData, NULL, &dwOutputLength));

				memcpy(pOutputData, pCurOutputData, dwCurOutputLength);

				IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(dwCurOutputLength));

				LOG_HRESULT(hr = pCurOutputBuf->Unlock());
				LOG_HRESULT(hr = pOutputBuffer->Unlock());

				pCurOutputData = NULL;
				pOutputData = NULL;

				REFERENCE_TIME rtTime;
				IF_FAILED_THROW(hr = pSample->GetSampleTime(&rtTime));

				if(rtTime == -1){
						m_rtTime += m_rtAvgPerFrame;
				}
				else{
						m_rtTime = rtTime;
				}

				#ifdef TRACE_OUTPUT_PTS
				  UINT32 uiPicture;
						IF_FAILED_THROW(hr = pSample->GetUINT32(MFSampleExtension_PictureType, &uiPicture));

		    switch(uiPicture){

		      case PICTURE_TYPE_I: TRACE((L"ITime : %I64d", m_rtTime)); break;
				    case PICTURE_TYPE_P: TRACE((L"PTime : %I64d", m_rtTime)); break;
				    case PICTURE_TYPE_B: TRACE((L"BTime : %I64d", m_rtTime)); break;
				    default: TRACE((L"UTime : %I64d", m_rtTime));
		    }
    #endif

				// Not needed... Refactor later
				IF_FAILED_THROW(hr = pSample->RemoveBufferByIndex(0));

				//IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

				if(m_bIsDiscontinuity){
						IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetUINT32(MFSampleExtension_Discontinuity, m_bIsDiscontinuity));
						m_bIsDiscontinuity = FALSE;
				}

				//IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleTime(m_rtFrame));

				//TRACE((L"%d", m_rtFrame));

				//REFERENCE_TIME last = m_rtFrame;
				//m_rtFrame += m_rtAvgPerFrame;
				//IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleDuration(m_rtFrame - last));

				//TRACE((L"OutputTime = %I64d", m_rtFrameTest));
				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleTime(m_rtTime));
				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleDuration(m_rtAvgPerFrame));
		}
		catch(HRESULT){}

		if(pOutputBuffer && pOutputData){
				LOG_HRESULT(pOutputBuffer->Unlock());
		}
		
		if(pCurOutputBuf && pCurOutputData){
				LOG_HRESULT(pCurOutputBuf->Unlock());
		}
		
		SAFE_RELEASE(pOutputBuffer);
		SAFE_RELEASE(pCurOutputBuf);

		SAFE_RELEASE(pSample);
		m_qSampleOut.pop();

		if(HaveSampleOut())
				pOutputSamples[0].dwStatus = MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;

		return hr;
}