//----------------------------------------------------------------------------------------------
// MFTVideoMpeg2_transform.cpp
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

HRESULT CMFTVideoMpeg2::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

		// This MFT has a fixed number of streams.
		*pdwInputMinimum = 1;
		*pdwInputMaximum = 1;
		*pdwOutputMinimum = 1;
		*pdwOutputMaximum = 1;

		return hr;
}

HRESULT CMFTVideoMpeg2::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

		// This MFT has a fixed number of streams.
		*pcInputStreams = 1;
		*pcOutputStreams = 1;

		return hr;
}

HRESULT CMFTVideoMpeg2::GetStreamIDs(DWORD /*dwInputIDArraySize*/, DWORD* /*pdwInputIDs*/, DWORD /*dwOutputIDArraySize*/, DWORD* /*pdwOutputIDs*/){

		// Do not need to implement, because this MFT has a fixed number of streams and the stream IDs match the stream indexes.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		
		// No lock, because no members class.

		// Learn more about this flags...
		//  We can process data on any boundary.
		pStreamInfo->dwFlags        = 0;
		pStreamInfo->hnsMaxLatency  = 0;
		pStreamInfo->cbSize         = 1;
		pStreamInfo->cbMaxLookahead = 0;
		pStreamInfo->cbAlignment    = 1;

		return hr;
}

HRESULT CMFTVideoMpeg2::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		// NOTE: This method should succeed even when there is no media type on the stream. If there is no media type,
		// we only need to fill in the dwFlags member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a valid media type.

		// Learn more about this flags...
		pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER | MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE;

		if(m_pOutputType == NULL){
				
				pStreamInfo->cbSize = 0;
				pStreamInfo->cbAlignment = 0;
		}
		else{
		
				pStreamInfo->cbSize = m_dwSampleSize2;
				pStreamInfo->cbAlignment = 1;
		}
    
		return hr;
}

HRESULT CMFTVideoMpeg2::GetAttributes(IMFAttributes** /*pAttributes*/){

		// This MFT does not support any attributes, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::GetInputStreamAttributes(DWORD /*dwInputStreamID*/, IMFAttributes** /*ppAttributes*/){

		// This MFT does not support any attributes, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::GetOutputStreamAttributes(DWORD /*dwOutputStreamID*/, IMFAttributes** /*ppAttributes*/){

		// This MFT does not support any attributes, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::DeleteInputStream(DWORD /*dwStreamID*/){

		// This MFT has a fixed number of input streams, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::AddInputStreams(DWORD /*cStreams*/, DWORD* /*adwStreamIDs*/){

		// This MFT has a fixed number of input streams, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::GetInputAvailableType(DWORD /*dwInputStreamID*/, DWORD /*dwTypeIndex*/, IMFMediaType** /*ppType*/){

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

HRESULT CMFTVideoMpeg2::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

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

HRESULT CMFTVideoMpeg2::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		// Validate flags.
		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		// If we have an input sample, the client cannot change the type now.
		IF_FAILED_RETURN(hr = (m_pInputBuffer != NULL ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		// Validate the type, if non-NULL.
		if(pType){
				hr = OnCheckInputType(pType);
		}

		if(SUCCEEDED(hr)){

				// The type is OK. Set the type, unless the caller was just testing.
				if(bReallySet){
						hr = OnSetInputType(pType);
				}
		}

		return hr;
}

HRESULT CMFTVideoMpeg2::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);
		//DWORD dwNumBuffer = 0;

		// If we have an output sample, the client cannot change the type now.
		IF_FAILED_RETURN(hr = (HaveSampleOut() ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		if(pType){
				hr = OnCheckOutputType(pType);
		}

		if(SUCCEEDED(hr)){

				if(bReallySet){
						
						// The type is OK. Set the type, unless the caller was just testing.
						SAFE_RELEASE(m_pOutputType);

						m_pOutputType = pType;
						m_pOutputType->AddRef();
				}
		}

		return hr;
}

HRESULT CMFTVideoMpeg2::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

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

HRESULT CMFTVideoMpeg2::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

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

HRESULT CMFTVideoMpeg2::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		// If we already have an input sample, we don't accept another one until the client calls ProcessOutput or Flush.
		if(m_pInputBuffer != NULL){
				*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
		}
		else{
				*pdwFlags = 0;
		}

		return hr;
}

HRESULT CMFTVideoMpeg2::GetOutputStatus(DWORD* pdwFlags){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		// We can produce an output sample if we have an output sample.
		if(HaveSampleOut()){
				*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
		}
		else{
				*pdwFlags = 0;
		}

		return hr;
}

HRESULT CMFTVideoMpeg2::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/){

		// Implementation of this method is optional.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent */){

		// This MFT does not handle any stream events, so the method can return E_NOTIMPL.
		// This tells the pipeline that it can stop sending any more events to this MFT.
		return E_NOTIMPL;
}

HRESULT CMFTVideoMpeg2::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

  HRESULT hr = S_OK;
		
		AutoLock lock(m_CriticSection);

		switch(eMessage){
		
		  case MFT_MESSAGE_COMMAND_FLUSH:
						// Flush the MFT.
						//TRACE((L"\nFLUSH : MFNode frames : %d\n", m_dwNumVideoFrame));
						hr = OnFlush();
						break;

				case MFT_MESSAGE_COMMAND_DRAIN:
						// Set the discontinuity flag on all of the input.
						hr = OnDrain();
						break;

				case MFT_MESSAGE_SET_D3D_MANAGER:
						// The pipeline should never send this message unless the MFT
						// has the MF_SA_D3D_AWARE attribute set to TRUE. However, if we
						// do get this message, it's invalid and we don't implement it.
						hr = E_NOTIMPL;
						break;

				case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
						hr = AllocateStreamingResources();
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

HRESULT CMFTVideoMpeg2::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

		//TRACE((L"ProcessInput"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		if(!m_pInputType || !m_pOutputType){
				return MF_E_NOTACCEPTING;   // Client must set input and output types.
		}
		else if(m_pInputBuffer != NULL){
				return MF_E_NOTACCEPTING;   // We already have an input sample.
		}

		// Allocate resources, in case the client did not send the MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.
		IF_FAILED_RETURN(hr = AllocateStreamingResources());

		// Get the input buffer(s) from the sample.
		DWORD dwBufferCount = 0;
		IF_FAILED_RETURN(hr = pSample->GetBufferCount(&dwBufferCount));

		// Convert to a single contiguous buffer.
		// NOTE: This does not cause a copy unless there are multiple buffers
  IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&m_pInputBuffer));

		// One day try this buffer
		//IMF2DBuffer* pBufTest = NULL;
		//hr = m_pInputBuffer->QueryInterface(IID_IMF2DBuffer, reinterpret_cast<void**>(&pBufTest));

		hr = m_pInputBuffer->Lock(&m_pbInputData, NULL, &m_dwInputLength);

		if(SUCCEEDED(hr)){

				LONGLONG rtTimestamp = 0;
				
				// Get the time stamp. It is OK if this call fails.
				if(SUCCEEDED(pSample->GetSampleTime(&rtTimestamp))){
						
						/*if(m_rtFrame > rtTimestamp)
								m_bDiscontinuity = TRUE;
						else{
								m_bDiscontinuity = FALSE;
						}*/
						
						m_rtFrame = rtTimestamp;
				}

				hr = Mpeg2ProcessInput();
		}
		else{
				ReleaseInputBuffer();
		}

		return hr;
}

HRESULT CMFTVideoMpeg2::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

		//TRACE((L"ProcessOutput"));
		// There are no flags that we accept in this MFT. The only defined flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER.
		// This flag only applies when the MFT marks an output stream as lazy or optional.
		// However there are no lazy or optional streams on this MFT, so the flag is not valid.

		/*if(dwFlags == MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER){

				// check it later
		}*/

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));

		// Must be exactly one output buffer.
		if(cOutputBufferCount != 1){

				// check it later
				return E_INVALIDARG;
		}
		
		// It must contain a sample.
		IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		// If we don't have an output sample, we need some input before we can generate any output.
		if(HaveSampleOut() == FALSE){
				hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
		}
		else{

				IF_FAILED_RETURN(hr = Mpeg2ProcessOutput(pOutputSamples[0].pSample, pdwStatus));
		}

		return hr;
}