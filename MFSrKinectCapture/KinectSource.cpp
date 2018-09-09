//----------------------------------------------------------------------------------------------
// KinectSource.cpp
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

CKinectSource::CKinectSource(HRESULT& hr)
		: m_pKinectDevice(NULL),
		  m_pEventQueue(NULL),
				m_pPresentationDescriptor(NULL),
				m_State(SourceInvalid){

		m_pKinectStream[0] = NULL;
		m_pKinectStream[1] = NULL;
		LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
		//TRACE((L"CKinectSource CTOR"));
}

HRESULT CKinectSource::CreateInstance(CKinectSource** ppSource){
		
		//TRACE((L"CKinectSource CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));
		
		CKinectSource* pSource = new (std::nothrow)CKinectSource(hr);
		
		IF_FAILED_RETURN(hr = (pSource == NULL ? E_OUTOFMEMORY : S_OK));

		if(SUCCEEDED(hr)){
				
				*ppSource = pSource;
				(*ppSource)->AddRef();
		}

		SAFE_RELEASE(pSource);

		return hr;
}

HRESULT CKinectSource::QueryInterface(REFIID riid, void** ppv){
		
		static const QITAB qit[] = {
				QITABENT(CKinectSource, IMFMediaEventGenerator),
				QITABENT(CKinectSource, IMFMediaSource),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP CKinectSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

		//TRACE((L"CKinectSource BeginGetEvent"));

		HRESULT hr = S_OK;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CKinectSource::EndGetEvent(IMFAsyncResult* pCallback, IMFMediaEvent** punkState){

		//TRACE((L"CKinectSource EndGetEvent"));

		HRESULT hr = S_OK;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
  
		LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CKinectSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

		//TRACE((L"CKinectSource GetEvent"));

		// NOTE: 
		// GetEvent can block indefinitely, so we don't hold the critical 
		// section. Therefore we need to use a local copy of the event queue 
		// pointer, to make sure the pointer remains valid.
		HRESULT hr;

		IMFMediaEventQueue* pQueue = NULL;

		{ // scope for lock
				AutoLock lock(m_CriticSection);

				LOG_HRESULT(hr = CheckShutdown());

				// Check shutdown
				if(SUCCEEDED(hr)){

						// Cache a local pointer to the queue.
						pQueue = m_pEventQueue;
						pQueue->AddRef();
				}   // release lock
		}

		if(SUCCEEDED(hr))
				// Use the local pointer to call GetEvent.
				LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));

		SAFE_RELEASE(pQueue);

		return hr;
}

STDMETHODIMP CKinectSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

		//TRACE((L"CKinectSource QueueEvent"));

		HRESULT hr = S_OK;

		AutoLock lock(m_CriticSection);
		
		IF_FAILED_RETURN(hr = CheckShutdown());

		LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}

STDMETHODIMP CKinectSource::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){
		
		//TRACE((L"CKinectSource CreatePresentationDescriptor"));
		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppPresentationDescriptor == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pPresentationDescriptor == NULL){
				IF_FAILED_RETURN(hr = CreateKinectPresentationDescriptor());
		}

		// Clone our default presentation descriptor.
		LOG_HRESULT(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

		return hr;
}

STDMETHODIMP CKinectSource::GetCharacteristics(DWORD* pdwCharacteristics){
		
		//TRACE((L"CKinectSource GetCharacteristics"));
		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pdwCharacteristics =  MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_IS_LIVE;

		return hr;
}

STDMETHODIMP CKinectSource::Pause(){

		//TRACE((L"CKinectSource Pause"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		// Pause is only allowed from started state.
		if(m_State != SourceStarted){
				IF_FAILED_RETURN(hr = MF_E_INVALID_STATE_TRANSITION);
		}

		// Send the appropriate events.
		if(m_pKinectStream[0]){
				IF_FAILED_RETURN(hr = m_pKinectStream[0]->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
		}
		if(m_pKinectStream[1]){
				IF_FAILED_RETURN(hr = m_pKinectStream[1]->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
		}

		IF_FAILED_RETURN(hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, NULL));

		// Update our state. 
		m_State = SourcePaused;

		return hr;
}

STDMETHODIMP CKinectSource::Shutdown(){

		//TRACE((L"CKinectSource Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		// Shut down the stream object.
		if(m_pKinectStream[0]){
				LOG_HRESULT(m_pKinectStream[0]->Shutdown());
		}
		if(m_pKinectStream[1]){
				LOG_HRESULT(m_pKinectStream[1]->Shutdown());
		}

		// Shut down the event queue.
  if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		// Release objects.
		SAFE_RELEASE(m_pKinectStream[0]);
		SAFE_RELEASE(m_pKinectStream[1]);
		SAFE_RELEASE(m_pEventQueue);
		SAFE_RELEASE(m_pPresentationDescriptor);

		SAFE_DELETE(m_pKinectDevice);

		m_State = SourceShutdown;

		return hr;
}

STDMETHODIMP CKinectSource::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){

		//TRACE((L"CKinectSource Start"));

		// Start or Seek (we can't seek)
		HRESULT hr;
		
		IF_FAILED_RETURN(hr = (pvarStartPosition == NULL ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (pPresentationDescriptor == NULL ? E_INVALIDARG : S_OK));

		// Check the time format. We support only reference time, which is indicated by a NULL parameter or by time format = GUID_NULL.
		IF_FAILED_RETURN(hr = ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		// Check the data type of the start position.
		IF_FAILED_RETURN(hr = ((pvarStartPosition->vt != VT_I8) && (pvarStartPosition->vt != VT_EMPTY) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		// Check if this is a seek request. Currently, this sample does not support seeking.
		if(pvarStartPosition->vt == VT_I8){

				// If the current state is STOPPED, then position 0 is valid.
				// If the current state is anything else, then the start position must be VT_EMPTY (current position).
				IF_FAILED_RETURN(hr = ((m_State != SourceStopped) || (pvarStartPosition->hVal.QuadPart != 0) ? MF_E_INVALIDREQUEST : S_OK));
		}

		AutoLock lock(m_CriticSection);
		
		IMFMediaEvent* pEvent = NULL;

		PROPVARIANT var;
		PropVariantInit(&var);

		BOOL bQueuedStartEvent = FALSE;

		try{

				// Fail if the source is shut down.
				IF_FAILED_THROW(hr = CheckShutdown());
				
				// Redundant with STATE_INVALID but possible
    IF_FAILED_THROW(hr = (m_pKinectDevice == NULL ? MF_E_HW_MFT_FAILED_START_STREAMING: S_OK));

				// Fail if the source was not initialized yet.
    IF_FAILED_THROW(hr = (m_State == SourceInvalid ? E_FAIL: S_OK));// invalid state

    // Perform a sanity check on the caller's presentation descriptor.
    IF_FAILED_THROW(hr = ValidatePresentationDescriptor(pPresentationDescriptor));

				// Sends the MENewStream or MEUpdatedStream event.
    IF_FAILED_THROW(hr = QueueNewStreamEvent(pPresentationDescriptor));
				
				var.vt = VT_I8;
				var.hVal.QuadPart = 0;

				// The operation looks OK.
				IF_FAILED_THROW(hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent));

				/*if(bIsRestartFromCurrentPosition){
						hr = pEvent->SetUINT64(MF_EVENT_SOURCE_ACTUAL_START, llStartOffset);
				}*/

				// Now  queue the event.
				IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));

				bQueuedStartEvent = TRUE;

				IF_FAILED_THROW(hr = (m_pKinectStream == NULL ? E_POINTER : S_OK));

				IF_FAILED_THROW(hr = m_pKinectStream[0]->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));
				IF_FAILED_THROW(hr = m_pKinectStream[1]->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));

				m_State = SourceStarted;
		}
		catch(HRESULT){}

		if(FAILED(hr) && bQueuedStartEvent){
				LOG_HRESULT(hr = QueueEvent(MEError, GUID_NULL, hr, &var));
		}

		PropVariantClear(&var);
		SAFE_RELEASE(pEvent);

		return hr;
}

STDMETHODIMP CKinectSource::Stop(){

		//TRACE((L"CKinectSource Stop"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		// Update our state. 
		m_State = SourceStopped;

		// Flush all queued samples. Pas besoin si pas de SampleQueue
		if(m_pKinectStream[0]){
				IF_FAILED_RETURN(hr = m_pKinectStream[0]->Flush());
		}

		if(m_pKinectStream[1]){
				IF_FAILED_RETURN(hr = m_pKinectStream[1]->Flush());
		}

		// Queue events.				
		if(m_pKinectStream[0]){
				IF_FAILED_RETURN(hr = m_pKinectStream[0]->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
		}
		
		if(m_pKinectStream[1]){
				IF_FAILED_RETURN(hr = m_pKinectStream[1]->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
		}

		IF_FAILED_RETURN(hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, NULL));

		return hr;
}

HRESULT CKinectSource::LoadKinect(LPCWSTR pwszURL){

		//TRACE((L"CKinectSource OpenKinect"));
		HRESULT hr;
		
		IF_FAILED_RETURN(hr = (pwszURL == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_pKinectDevice != NULL ? MF_E_ALREADY_INITIALIZED : S_OK));

		m_pKinectDevice = new (std::nothrow)CKinectDevice;

		IF_FAILED_RETURN(hr = (m_pKinectDevice == NULL ? E_OUTOFMEMORY : S_OK));

		LOG_HRESULT(hr = m_pKinectDevice->Initialize());

		if(FAILED(hr)){
				LOG_HRESULT(Shutdown());
		}
		else{

				m_State = SourceStopped;
		}

		return hr;
}

HRESULT CKinectSource::CreateKinectPresentationDescriptor(){

		//TRACE((L"CKinectSource CreateKinectPresentationDescriptor"));

		HRESULT hr = S_OK;

		IMFMediaType* pVideoType = NULL;
		IMFMediaType* pAudioType = NULL;
		IMFStreamDescriptor* pStreamDescriptor[2] = {NULL, NULL};
		IMFMediaTypeHandler* pVideoHandler = NULL;
		IMFMediaTypeHandler* pAudioHandler = NULL;

		try{

				// VIDEO
				// Initialize the media type from the Kinect Value.
				IF_FAILED_THROW(hr = m_pKinectDevice->GetVideoKinectType(&pVideoType));

				// Create the stream descriptor.
				// stream identifier, Number of media types, Array of media types
				IF_FAILED_THROW(hr = MFCreateStreamDescriptor(0, 1, &pVideoType, &pStreamDescriptor[0]));

				// Set the default media type on the media type handler.
				IF_FAILED_THROW(hr = pStreamDescriptor[0]->GetMediaTypeHandler(&pVideoHandler));
				IF_FAILED_THROW(hr = pVideoHandler->SetCurrentMediaType(pVideoType));

				// AUDIO
				// Initialize the media type from the Kinect Value.
				IF_FAILED_THROW(hr = m_pKinectDevice->GetAudioKinectType(&pAudioType));

				// Create the stream descriptor.
				// stream identifier, Number of media types, Array of media types
				IF_FAILED_THROW(hr = MFCreateStreamDescriptor(1, 1, &pAudioType, &pStreamDescriptor[1]));

				// Set the default media type on the media type handler.
				IF_FAILED_THROW(hr = pStreamDescriptor[1]->GetMediaTypeHandler(&pAudioHandler));
				IF_FAILED_THROW(hr = pAudioHandler->SetCurrentMediaType(pAudioType));

				// DESCRIPTOR
				// Create the presentation descriptor.
				// Number of stream descriptors, Array of stream descriptors
				IF_FAILED_THROW(hr = MFCreatePresentationDescriptor(2, pStreamDescriptor, &m_pPresentationDescriptor));

				// Select the first stream
				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(0));
				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(1));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoType);
		SAFE_RELEASE(pAudioType);
		SAFE_RELEASE(pStreamDescriptor[0]);
		SAFE_RELEASE(pStreamDescriptor[1]);
		SAFE_RELEASE(pVideoHandler);
		SAFE_RELEASE(pAudioHandler);

		return hr;
}

HRESULT CKinectSource::ValidatePresentationDescriptor(IMFPresentationDescriptor* pPD){

		//TRACE((L"CKinectSource ValidatePresentationDescriptor"));

		HRESULT hr;
		DWORD cStreams = 0;
		
		IF_FAILED_RETURN(hr = (pPD == NULL ? E_POINTER : S_OK));

		// Perhaps not need for all
		BOOL fVideoSelected = FALSE;
		BOOL fAudioSelected = FALSE;
		IMFStreamDescriptor* pVideoSD = NULL;
		IMFMediaTypeHandler* pVideoHandler = NULL;
		IMFMediaType*        pVideoMediaType = NULL;
		IMFStreamDescriptor* pAudioSD = NULL;
		IMFMediaTypeHandler* pAudioHandler = NULL;
		IMFMediaType*        pAudioMediaType = NULL;

		try{

				// The caller's PD must have the same number of streams as ours.
				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&cStreams));

				IF_FAILED_THROW(hr = (cStreams != 2 ? MF_E_UNSUPPORTED_REPRESENTATION : S_OK));

				// Video
				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(0, &fVideoSelected, &pVideoSD));
				IF_FAILED_THROW(hr = (fVideoSelected == FALSE ? MF_E_UNSUPPORTED_REPRESENTATION : S_OK));
				IF_FAILED_THROW(hr = pVideoSD->GetMediaTypeHandler(&pVideoHandler));
				IF_FAILED_THROW(hr = pVideoHandler->GetCurrentMediaType(&pVideoMediaType));

				// todo : Check pMediaType as Video Kinect Media Type MF_E_INVALIDMEDIATYPE

				// Audio
				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(1, &fAudioSelected, &pAudioSD));
				IF_FAILED_THROW(hr = (fAudioSelected == FALSE ? MF_E_UNSUPPORTED_REPRESENTATION : S_OK));
				IF_FAILED_THROW(hr = pAudioSD->GetMediaTypeHandler(&pAudioHandler));
				IF_FAILED_THROW(hr = pAudioHandler->GetCurrentMediaType(&pAudioMediaType));

				// todo : Check pMediaType as Audio Kinect Media Type MF_E_INVALIDMEDIATYPE
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoMediaType);
		SAFE_RELEASE(pVideoHandler);
		SAFE_RELEASE(pVideoSD);
		SAFE_RELEASE(pAudioMediaType);
		SAFE_RELEASE(pAudioHandler);
		SAFE_RELEASE(pAudioSD);

		return hr;
}

HRESULT CKinectSource::QueueNewStreamEvent(IMFPresentationDescriptor* pPD){
		
		//TRACE((L"CKinectSource QueueNewStreamEvent"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pPD == NULL ? E_POINTER : S_OK));

		// Peut-être pas tout util ici
		IMFStreamDescriptor* pVideoSD = NULL;
		IMFStreamDescriptor* pAudioSD = NULL;
		BOOL fVideoSelected = FALSE;
		BOOL fAudioSelected = FALSE;
		DWORD cStreams = 0;

		try{

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&cStreams));

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(0, &fVideoSelected, &pVideoSD));
				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(1, &fAudioSelected, &pAudioSD));
				
				assert(fVideoSelected);
				assert(fAudioSelected);

				if(m_pKinectStream[0]){

						// The stream already exists, and is still selected.
						// Send the MEUpdatedStream event.
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pKinectStream[0]));
				}
				else{

						// The stream does not exist, and is now selected. Create a new stream.
						IF_FAILED_THROW(hr = CKinectStream::CreateInstance(&m_pKinectStream[0], this, pVideoSD, KINECT_VIDEO_WIDTH * KINECT_VIDEO_HEIGHT * 4, 0, hr));

						// CreateWavStream creates the stream, so m_pStream is no longer NULL.
						assert(m_pKinectStream[0] != NULL);

						// Send the MENewStream event.
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pKinectStream[0]));
				}

				if(m_pKinectStream[1]){

						// The stream already exists, and is still selected.
						// Send the MEUpdatedStream event.
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pKinectStream[1]));
				}
				else{

						// The stream does not exist, and is now selected. Create a new stream.
						IF_FAILED_THROW(hr = CKinectStream::CreateInstance(&m_pKinectStream[1], this, pAudioSD, KINECT_VIDEO_WIDTH * KINECT_VIDEO_HEIGHT * 4, 1, hr));

						// CreateWavStream creates the stream, so m_pStream is no longer NULL.
						assert(m_pKinectStream[1] != NULL);

						// Send the MENewStream event.
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pKinectStream[1]));
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoSD);
		SAFE_RELEASE(pAudioSD);

		return hr;
}