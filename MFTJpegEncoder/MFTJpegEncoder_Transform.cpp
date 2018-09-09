//----------------------------------------------------------------------------------------------
// MFTJpegEncoder_Transform.cpp
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

HRESULT CMFTJpegEncoder::GetStreamLimits(DWORD* pdwInputMinimum, DWORD* pdwInputMaximum, DWORD* pdwOutputMinimum, DWORD* pdwOutputMaximum){

		TRACE_TRANSFORM((L"MFTJpeg::GetStreamLimits"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pdwInputMinimum == NULL || pdwInputMaximum == NULL || pdwOutputMinimum == NULL || pdwOutputMaximum == NULL) ? E_POINTER : S_OK));

		*pdwInputMinimum = 1;
		*pdwInputMaximum = 1;
		*pdwOutputMinimum = 1;
		*pdwOutputMaximum = 1;

		return hr;
}

HRESULT CMFTJpegEncoder::GetStreamCount(DWORD* pcInputStreams, DWORD* pcOutputStreams){

		TRACE_TRANSFORM((L"MFTJpeg::GetStreamCount"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = ((pcInputStreams == NULL || pcOutputStreams == NULL) ? E_POINTER : S_OK));

		*pcInputStreams = 1;
		*pcOutputStreams = 1;

		return hr;
}

HRESULT CMFTJpegEncoder::GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){

		TRACE_TRANSFORM((L"MFTJpeg::GetStreamIDs"));

		// Do not need to implement, because this MFT has a fixed number of streams and the stream IDs match the stream indexes.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::GetInputStreamInfo(DWORD dwInputStreamID, MFT_INPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"MFTJpeg::GetInputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		
		//  We can process data on any boundary.
		pStreamInfo->dwFlags        = MFT_INPUT_STREAM_WHOLE_SAMPLES |
				                            MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
																																MFT_INPUT_STREAM_FIXED_SAMPLE_SIZE |
				                            MFT_INPUT_STREAM_DOES_NOT_ADDREF;
		pStreamInfo->hnsMaxLatency  = 0;
		pStreamInfo->cbSize         = 0;
		pStreamInfo->cbMaxLookahead = 0;
		pStreamInfo->cbAlignment    = 0;

		if(m_pInputType){

				// Todo set the size
				// pStreamInfo->cbSize = sample input size;
		}

		return hr;
}

HRESULT CMFTJpegEncoder::GetOutputStreamInfo(DWORD dwOutputStreamID, MFT_OUTPUT_STREAM_INFO* pStreamInfo){

		TRACE_TRANSFORM((L"MFTJpeg::GetOutputStreamInfo"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pStreamInfo == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		pStreamInfo->dwFlags = MFT_OUTPUT_STREAM_WHOLE_SAMPLES | MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER;
		// We need to set a correct size...
  pStreamInfo->cbSize = 300000;
		pStreamInfo->cbAlignment = 0;
    
		return hr;
}

HRESULT CMFTJpegEncoder::GetAttributes(IMFAttributes** ppAttributes){

		TRACE_TRANSFORM((L"MFTJpeg::GetAttributes"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppAttributes == NULL ? E_POINTER : S_OK));

		if(*ppAttributes){

				// Use MF_MP2DLNA_ENCODE_QUALITY, but we could create a private GUID.
				IF_FAILED_RETURN(hr = (*ppAttributes)->SetUINT32(MF_MP2DLNA_ENCODE_QUALITY, static_cast<UINT32>(m_iJpegQuality)));
		}
		else{

				IMFAttributes* pAttributes = NULL;

				try{

						IF_FAILED_THROW(hr = MFCreateAttributes(&pAttributes, 1));
						IF_FAILED_THROW(hr = pAttributes->SetUINT32(MF_MP2DLNA_ENCODE_QUALITY, static_cast<UINT32>(m_iJpegQuality)));

						*ppAttributes = pAttributes;
						(*ppAttributes)->AddRef();
				}
				catch(HRESULT){}

				SAFE_RELEASE(pAttributes);
		}

		return hr;
}

HRESULT CMFTJpegEncoder::GetInputStreamAttributes(DWORD /*dwInputStreamID*/, IMFAttributes** /*ppAttributes*/){

		TRACE_TRANSFORM((L"MFTJpeg::GetInputStreamAttributes"));
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::GetOutputStreamAttributes(DWORD /*dwOutputStreamID*/, IMFAttributes** /*ppAttributes*/){

		TRACE_TRANSFORM((L"MFTJpeg::GetOutputStreamAttributes"));
		// Should be the same as GetAttributes, somewhat redondant.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::DeleteInputStream(DWORD /*dwStreamID*/){

		TRACE_TRANSFORM((L"MFTJpeg::DeleteInputStream"));
		// This MFT has a fixed number of input streams, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::AddInputStreams(DWORD /*cStreams*/, DWORD* /*adwStreamIDs*/){

		TRACE_TRANSFORM((L"MFTJpeg::AddInputStreams"));
		// This MFT has a fixed number of input streams, so the method is not implemented.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::GetInputAvailableType(DWORD /*dwInputStreamID*/, DWORD /*dwTypeIndex*/, IMFMediaType** /*ppType*/){

		TRACE_TRANSFORM((L"MFTJpeg::GetInputAvailableType"));

		// TODO : offer a mediatype - major VIDEO - sub RGB24 or RGB32
		return MF_E_NO_MORE_TYPES;

		/*
		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppType == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		// If offer only two mediatype
		if(dwTypeIndex != 0 || dwTypeIndex != 1){
		  return MF_E_NO_MORE_TYPES;
  }

		AutoLock lock(m_critSec);

		ppType ---> offer an inputtype

		return hr;*/
}

HRESULT CMFTJpegEncoder::GetOutputAvailableType(DWORD dwOutputStreamID, DWORD dwTypeIndex, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTJpeg::GetOutputAvailableType"));

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

HRESULT CMFTJpegEncoder::SetInputType(DWORD dwInputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTJpeg::SetInputType"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		// If we are processing, the client cannot change the type now.
		IF_FAILED_RETURN(hr = (m_pSampleOut != NULL ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		// Validate the type, if non-NULL.
		if(pType){

				hr = OnCheckInputType(pType);

				if(SUCCEEDED(hr) && bReallySet){

						SAFE_RELEASE(m_pInputType);
						m_pInputType = pType;
						m_pInputType->AddRef();
				}
		}
		/*
		else{
		  // In fact, for now, i'm wondering why the type could be NULL. Check this.
				// The function is called SetInputType. So a type has to be provide...
		}
		*/

		return hr;
}

HRESULT CMFTJpegEncoder::SetOutputType(DWORD dwOutputStreamID, IMFMediaType* pType, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTJpeg::SetOutputType"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwOutputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags & ~MFT_SET_TYPE_TEST_ONLY ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		// Does the caller want us to set the type, or just test it?
		BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

		// If we are processing, the client cannot change the type now.
		IF_FAILED_RETURN(hr = (m_pSampleOut != NULL ? MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING : S_OK));

		if(pType){

				hr = OnCheckOutputType(pType);

				if(SUCCEEDED(hr) && bReallySet){

						// The type is OK. Set the type, unless the caller was just testing.
						SAFE_RELEASE(m_pOutputType);

						m_pOutputType = pType;
						m_pOutputType->AddRef();
				}
		}
		/*
		else{
		  // In fact, for now, i'm wondering why the type could be NULL. Check this.
				// The function is called SetOutputType. So a type has to be provide...
		}
		*/

		return hr;
}

HRESULT CMFTJpegEncoder::GetInputCurrentType(DWORD dwInputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTJpeg::GetInputCurrentType"));

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

HRESULT CMFTJpegEncoder::GetOutputCurrentType(DWORD dwOutputStreamID, IMFMediaType** ppType){

		TRACE_TRANSFORM((L"MFTJpeg::GetOutputCurrentType"));

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

HRESULT CMFTJpegEncoder::GetInputStatus(DWORD dwInputStreamID, DWORD* pdwFlags){

		TRACE_TRANSFORM((L"MFTJpeg::GetInputStatus"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		if(m_bHaveImage){
				*pdwFlags = 0;
		}
		else{
				*pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
		}

		return hr;
}

HRESULT CMFTJpegEncoder::GetOutputStatus(DWORD* pdwFlags){

		TRACE_TRANSFORM((L"MFTJpeg::GetOutputStatus"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		if(m_bHaveImage){
				*pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
		}
		else{
				*pdwFlags = 0;
		}

		return hr;
}

HRESULT CMFTJpegEncoder::SetOutputBounds(LONGLONG /*hnsLowerBound*/, LONGLONG /*hnsUpperBound*/){

		TRACE_TRANSFORM((L"MFTJpeg::SetOutputBounds"));
		// Implementation of this method is optional.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::ProcessEvent(DWORD /*dwInputStreamID*/, IMFMediaEvent* /*pEvent */){

		TRACE_TRANSFORM((L"MFTJpeg::ProcessEvent"));
		// This MFT does not handle any stream events, so the method can return E_NOTIMPL.
		// This tells the pipeline that it can stop sending any more events to this MFT.
		return E_NOTIMPL;
}

HRESULT CMFTJpegEncoder::ProcessMessage(MFT_MESSAGE_TYPE eMessage, ULONG_PTR /*ulParam*/){

		TRACE_TRANSFORM((L"MFTJpeg::ProcessMessage : %s", MFTMessageString(eMessage)));

  HRESULT hr = S_OK;
		
		AutoLock lock(m_CriticSection);

		switch(eMessage){
		
		  case MFT_MESSAGE_COMMAND_FLUSH:
						//hr = OnFlush();
						break;

				case MFT_MESSAGE_COMMAND_DRAIN:
						//hr = OnDrain();
						break;

				case MFT_MESSAGE_SET_D3D_MANAGER:
						hr = E_NOTIMPL;
						break;

				case MFT_MESSAGE_DROP_SAMPLES:
						//hr = OnDropSamples();
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

				case MFT_MESSAGE_COMMAND_MARKER:
						//hr = OnMarker();
						break;        
		}

		return hr;
}

HRESULT CMFTJpegEncoder::ProcessInput(DWORD dwInputStreamID, IMFSample* pSample, DWORD dwFlags){

		TRACE_TRANSFORM((L"MFTJpeg::ProcessInput"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (dwInputStreamID != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		if(!m_pInputType || !m_pOutputType || !m_pSampleOut || m_bHaveImage){
				return MF_E_NOTACCEPTING;
		}

		// Get the input buffer(s) from the sample.
		DWORD dwBufferCount = 0;
		IF_FAILED_RETURN(hr = pSample->GetBufferCount(&dwBufferCount));
		IF_FAILED_RETURN(hr = (dwBufferCount == 0 ? E_UNEXPECTED : S_OK));

		IMFMediaBuffer* pInputBuffer = NULL;
		BYTE* pbInputData = NULL;
		DWORD dwInputLength;

		IF_FAILED_RETURN(hr = pSample->ConvertToContiguousBuffer(&pInputBuffer));

		BitmapData bitmapData;
		Status GdiStatus;
		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;
		GUID SubType = {0};
		int iPixelFormat;
		int iRgbByte;
		Rect RgbRect(0, 0, 0, 0);

		try{

				IF_FAILED_THROW(hr = pInputBuffer->Lock(&pbInputData, NULL, &dwInputLength));

				LONGLONG rtTimestamp = 0;

				// Get the time stamp. It is OK if this call fails.
				if(SUCCEEDED(pSample->GetSampleTime(&rtTimestamp))){
						
						m_rtFrame = rtTimestamp;
				}

				IF_FAILED_THROW(hr = MFGetAttributeSize(m_pInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
				IF_FAILED_THROW(hr = m_pInputType->GetGUID(MF_MT_SUBTYPE, &SubType));

				RgbRect.Width = uiWidth;
				RgbRect.Height = uiHeight;

				if(SubType == MFVideoFormat_RGB24){
						iRgbByte = 3;
						iPixelFormat = PixelFormat24bppRGB;
				}
				else{
						iRgbByte = 4;
						iPixelFormat = PixelFormat32bppRGB;
				}

				IF_FAILED_THROW(hr = (dwInputLength != (uiWidth * uiHeight * iRgbByte) ? E_UNEXPECTED : S_OK));

				GdiStatus = m_pBmpImageFile->LockBits(&RgbRect, ImageLockModeWrite, iPixelFormat, &bitmapData);
				IF_FAILED_THROW(hr = (GdiStatus != Ok ? E_FAIL : S_OK));

				memcpy(bitmapData.Scan0, pbInputData, dwInputLength);

				GdiStatus = m_pBmpImageFile->UnlockBits(&bitmapData);
				IF_FAILED_THROW(hr = (GdiStatus != Ok ? E_FAIL : S_OK));

				m_bHaveImage = TRUE;
		}
		catch(HRESULT){}
		
		if(pInputBuffer && pbInputData){

				LOG_HRESULT(pInputBuffer->Unlock());
		}

		SAFE_RELEASE(pInputBuffer);

		return hr;
}

HRESULT CMFTJpegEncoder::ProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, MFT_OUTPUT_DATA_BUFFER* pOutputSamples, DWORD* pdwStatus){

		TRACE_TRANSFORM((L"MFTJpeg::ProcessOutput"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (dwFlags != 0 ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = ((pOutputSamples == NULL || pdwStatus == NULL) ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (cOutputBufferCount != 1 ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (pOutputSamples[0].pSample == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		if(m_bHaveImage == FALSE){
				return MF_E_TRANSFORM_NEED_MORE_INPUT;
		}

		IMFMediaBuffer* pOutputBuffer = NULL;
		BYTE* pOutputData = NULL;
		DWORD dwOutputLength = 0;

		Status GdiStatus;
		BYTE* pData = NULL;
		DWORD dwSize;

		try{

				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->GetBufferByIndex(0, &pOutputBuffer));
				IF_FAILED_THROW(hr = pOutputBuffer->GetMaxLength(&dwOutputLength));

				EncoderParameters encoderParameters;
				encoderParameters.Count = 1;
				encoderParameters.Parameter[0].Guid = EncoderQuality;
				encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
				encoderParameters.Parameter[0].NumberOfValues = 1;
				encoderParameters.Parameter[0].Value = &m_iJpegQuality;

				GdiStatus = m_pBmpImageFile->Save(m_iJpegStream, &m_JpgClsid, &encoderParameters);

				IF_FAILED_THROW(hr = (GdiStatus != Ok ? E_FAIL : S_OK));

				pData = m_cJpegStream->GetBufferAndSize(dwSize);

				IF_FAILED_THROW(hr = ((pData == NULL || dwSize == 0 || dwOutputLength < dwSize) ? E_FAIL : S_OK));

				IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pOutputData, NULL, &dwOutputLength));

				memcpy(pOutputData, pData, dwSize);

				IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(dwSize));

				LOG_HRESULT(hr = pOutputBuffer->Unlock());
				pOutputData = NULL;

				// In fact i'm not sure if useful here...
				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleTime(m_rtFrame));
				IF_FAILED_THROW(hr = pOutputSamples[0].pSample->SetSampleDuration(m_ui64AvgPerFrame));

				m_rtFrame += m_ui64AvgPerFrame;

				m_bHaveImage = FALSE;
				*pdwStatus = 0;
		}
		catch(HRESULT){}

		if(pOutputBuffer && pOutputData){
				LOG_HRESULT(pOutputBuffer->Unlock());
		}
		
		SAFE_RELEASE(pOutputBuffer);

		return hr;
}