//----------------------------------------------------------------------------------------------
// WaveMixerSession_Topology.cpp
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
#include "Stdafx.h"

HRESULT CWaveMixerSession::LoadCustomTopology(){

	TRACE_SESSION((L"Session::LoadCustomTopology"));

	HRESULT hr;

	assert(m_pWaveMixer == NULL);
	assert(m_pResampler == NULL);
	assert(m_pSink == NULL);
	assert(m_pByteStreamHandler1 == NULL);
	assert(m_pByteStreamHandler2 == NULL);

	IF_FAILED_RETURN(hr = CoCreateInstance(CLSID_MFTWaveMixer, NULL, CLSCTX_INPROC, __uuidof(IMFTransform), reinterpret_cast<void**>(&m_pWaveMixer)));

	IF_FAILED_RETURN(hr = CoCreateInstance(CLSID_CResamplerMediaObject, NULL, CLSCTX_INPROC, __uuidof(IMFTransform), reinterpret_cast<void**>(&m_pResampler)));

	IF_FAILED_RETURN(hr = MFCreateAudioRenderer(NULL, &m_pSink));

	IF_FAILED_RETURN(hr = BeginCreateMediaSource(&m_pByteStreamHandler1, m_szFileUrl1.c_str(), ME_MFNodeByteStreamHandler));

	IF_FAILED_RETURN(hr = BeginCreateMediaSource(&m_pByteStreamHandler2, m_szFileUrl2.c_str(), ME_MFNodeByteStreamHandler2));

	return hr;
}

HRESULT CWaveMixerSession::BeginCreateMediaSource(IMFByteStreamHandler** ppByteStreamHandler, const WCHAR* pFileUrl,
	const MFNodeMediaEvent EventType){

	TRACE_SESSION((L"Session::BeginCreateMediaSource"));

	HRESULT hr = S_OK;
	IMFByteStream* pByteStream = NULL;
	IMFAsyncState* pState = NULL;

	try{

		IF_FAILED_THROW(hr = CoCreateInstance(CLSID_WAVEByteStreamPlugin, NULL, CLSCTX_INPROC, __uuidof(IMFByteStreamHandler),
			reinterpret_cast<void**>(ppByteStreamHandler)));

		IF_FAILED_THROW(hr = MFCreateFile(MF_ACCESSMODE_READ, MF_OPENMODE_FAIL_IF_NOT_EXIST, MF_FILEFLAGS_NONE, pFileUrl, &pByteStream));

		pState = new (std::nothrow) CMFAsyncState(EventType);
		IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
		pState->AddRef();

		IF_FAILED_THROW(hr = (*ppByteStreamHandler)->BeginCreateObject(pByteStream, pFileUrl, MF_RESOLUTION_MEDIASOURCE, NULL, NULL, this, pState));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pState);
	SAFE_RELEASE(pByteStream);

	return hr;
}

HRESULT CWaveMixerSession::NegotiateMediaTypes(){

	TRACE_SESSION((L"Session::NegotiateMediaTypes"));

	HRESULT hr;

	assert(m_pPresentation1 == NULL);
	assert(m_pPresentation2 == NULL);
	assert(m_pWaveMixer != NULL);
	assert(m_pResampler != NULL);
	assert(m_pSink != NULL);
	assert(m_pSource1 != NULL);
	assert(m_pSource2 != NULL);

	IF_FAILED_RETURN(hr = ConnectSourceToMft(&m_pPresentation1, m_pSource1, m_pWaveMixer, 0));

	IF_FAILED_RETURN(hr = ConnectSourceToMft(&m_pPresentation2, m_pSource2, m_pWaveMixer, 1));

	IF_FAILED_RETURN(hr = ConnectMftToMft(m_pWaveMixer, m_pResampler));

	IF_FAILED_RETURN(hr = ConnectMftToSink(m_pResampler));

	return hr;
}

HRESULT CWaveMixerSession::ConnectSourceToMft(IMFPresentationDescriptor** ppPresentation, IMFMediaSource* pSource,
	IMFTransform* pMft, const DWORD dwStreamIndex){

	TRACE_SESSION((L"Session::ConnectSourceToMft"));

	HRESULT hr;
	IMFStreamDescriptor* pStreamDescriptor = NULL;
	IMFMediaTypeHandler* pMediaTypeHandler = NULL;
	IMFMediaType* pMediaType = NULL;
	BOOL streamSelected = FALSE;

	DWORD sourceTypesCount = 0;

	IF_FAILED_RETURN(hr = (pMft == NULL ? E_UNEXPECTED : S_OK));
	IF_FAILED_RETURN(hr = (pSource == NULL ? E_UNEXPECTED : S_OK));

	try{

		IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(ppPresentation));

		IF_FAILED_THROW(hr = (*ppPresentation)->GetStreamDescriptorByIndex(0, &streamSelected, &pStreamDescriptor));

		IF_FAILED_THROW(hr = pStreamDescriptor->GetMediaTypeHandler(&pMediaTypeHandler));

		IF_FAILED_THROW(hr = pMediaTypeHandler->GetMediaTypeCount(&sourceTypesCount));

		for(DWORD x = 0; x < sourceTypesCount; x++){

			IF_FAILED_THROW(hr = pMediaTypeHandler->GetMediaTypeByIndex(x, &pMediaType));

			IF_FAILED_THROW(hr = pMft->SetInputType(dwStreamIndex, pMediaType, 0));

			if(SUCCEEDED(hr)){

				hr = pMediaTypeHandler->SetCurrentMediaType(pMediaType);
				break;
			}

			SAFE_RELEASE(pMediaType);
		}

		IF_FAILED_THROW(hr);

		if(!streamSelected){
			IF_FAILED_THROW(hr = (*ppPresentation)->SelectStream(0));
		}
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);
	SAFE_RELEASE(pMediaTypeHandler);
	SAFE_RELEASE(pStreamDescriptor);

	return hr;
}

HRESULT CWaveMixerSession::ConnectMftToMft(IMFTransform* pMft1, IMFTransform* pMft2){

	TRACE_SESSION((L"Session::ConnectMftToMft"));

	HRESULT hr;
	IMFMediaType* pMediaType = NULL;
	DWORD mft1OutputStreamId = 0;
	DWORD mft2InputStreamId = 0;
	DWORD mft1TypeIndex = 0;

	IF_FAILED_RETURN(hr = (pMft1 == NULL ? E_UNEXPECTED : S_OK));
	IF_FAILED_RETURN(hr = (pMft2 == NULL ? E_UNEXPECTED : S_OK));

	try{

		while(true){

			IF_FAILED_THROW(hr = pMft1->GetOutputAvailableType(mft1OutputStreamId, mft1TypeIndex++, &pMediaType));

			hr = pMft2->SetInputType(mft2InputStreamId, pMediaType, 0);

			if(SUCCEEDED(hr)){

				hr = pMft1->SetOutputType(mft1OutputStreamId, pMediaType, 0);
				break;
			}

			SAFE_RELEASE(pMediaType);
		}
		IF_FAILED_THROW(hr);
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);

	return hr;
}

HRESULT CWaveMixerSession::ConnectMftToSink(IMFTransform* pMFTransform){

	TRACE_SESSION((L"Session::ConnectMftToSink"));

	HRESULT hr = S_OK;
	IMFMediaTypeHandler* pMediaTypeHandler = NULL;
	IMFMediaType* pMediaType = NULL;

	DWORD mftOutputStreamId = 0;
	DWORD mftTypeIndex = 0;

	assert(m_pStreamSink == NULL);

	IF_FAILED_RETURN(hr = (pMFTransform == NULL ? E_UNEXPECTED : S_OK));
	IF_FAILED_RETURN(hr = (m_pSink == NULL ? E_UNEXPECTED : S_OK));

	try{

		IF_FAILED_THROW(hr = m_pSink->GetStreamSinkByIndex(0, &m_pStreamSink));

		IF_FAILED_THROW(hr = m_pStreamSink->GetMediaTypeHandler(&pMediaTypeHandler));

		while(true){

			IF_FAILED_THROW(hr = pMFTransform->GetOutputAvailableType(mftOutputStreamId, mftTypeIndex++, &pMediaType));

			hr = pMediaTypeHandler->SetCurrentMediaType(pMediaType);

			if(SUCCEEDED(hr)){

				hr = pMFTransform->SetOutputType(mftOutputStreamId, pMediaType, 0);
				break;
			}

			SAFE_RELEASE(pMediaType);
		}
		IF_FAILED_THROW(hr);
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);
	SAFE_RELEASE(pMediaTypeHandler);

	return hr;
}

HRESULT CWaveMixerSession::FireTopologyReadyEvent(){

	TRACE_SESSION((L"Session::FireTopologyReadyEvent"));

	HRESULT hr = S_OK;
	PROPVARIANT variantStatus;
	IMFMediaEvent* pEvent = NULL;

	try{

		PropVariantInit(&variantStatus);

		IF_FAILED_THROW(hr = MFCreateMediaEvent(MESessionTopologyStatus, GUID_NULL, hr, &variantStatus, &pEvent));

		IF_FAILED_THROW(hr = pEvent->SetUINT32(MF_EVENT_TOPOLOGY_STATUS, MF_TOPOSTATUS_READY));

		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));
	}
	catch(HRESULT){}

	PropVariantClear(&variantStatus);
	SAFE_RELEASE(pEvent);

	return hr;
}