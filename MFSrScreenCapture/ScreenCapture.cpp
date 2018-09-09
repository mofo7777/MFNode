//----------------------------------------------------------------------------------------------
// ScreenCapture.cpp
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

CScreenCapture::CScreenCapture(HRESULT& hr)
		: m_nRefCount(1),
		  m_pScreenCaptureStream(NULL),
				m_pEventQueue(NULL),
				m_pPresentationDescriptor(NULL),
				m_State(SourceInvalid){

		LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
		TRACE_SOURCE((L"Capture::CTOR"));
}

HRESULT CScreenCapture::CreateInstance(CScreenCapture** ppSource){
		
		TRACE_SOURCE((L"Capture::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));
		
		CScreenCapture* pSource = new (std::nothrow)CScreenCapture(hr);
		
		IF_FAILED_RETURN(pSource == NULL ? E_OUTOFMEMORY : S_OK);

		*ppSource = pSource;
		(*ppSource)->AddRef();

		SAFE_RELEASE(pSource);

		return hr;
}

HRESULT CScreenCapture::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_SOURCE((L"Capture::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CScreenCapture, IMFMediaEventGenerator),
				QITABENT(CScreenCapture, IMFMediaSource),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CScreenCapture::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Capture::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CScreenCapture::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Capture::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

STDMETHODIMP CScreenCapture::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

		TRACE_EVENTSOURCE((L"Capture::BeginGetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
  
		LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CScreenCapture::EndGetEvent(IMFAsyncResult* pCallback, IMFMediaEvent** punkState){

		TRACE_EVENTSOURCE((L"Capture::EndGetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
  
		LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CScreenCapture::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

		TRACE_EVENTSOURCE((L"Capture::GetEvent"));

		HRESULT hr;

		IMFMediaEventQueue* pQueue = NULL;

		{
				AutoLock lock(m_CriticSection);

				LOG_HRESULT(hr = CheckShutdown());

				if(SUCCEEDED(hr)){

						pQueue = m_pEventQueue;
						pQueue->AddRef();
				}
		}

		if(SUCCEEDED(hr))
				LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));

		SAFE_RELEASE(pQueue);

		return hr;
}

STDMETHODIMP CScreenCapture::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

		TRACE_EVENTSOURCE((L"Capture::QueueEvent : %s", MFEventString(met)));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}

STDMETHODIMP CScreenCapture::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){
		
		TRACE_SOURCE((L"Capture::CreatePresentationDescriptor"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppPresentationDescriptor == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pPresentationDescriptor == NULL){
				IF_FAILED_RETURN(hr = CreateScreenCapturePresentationDescriptor());
		}

		LOG_HRESULT(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

		return hr;
}

STDMETHODIMP CScreenCapture::GetCharacteristics(DWORD* pdwCharacteristics){
		
		TRACE_SOURCE((L"Capture::GetCharacteristics"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pdwCharacteristics =  MFMEDIASOURCE_CAN_PAUSE;

		return hr;
}

STDMETHODIMP CScreenCapture::Pause(){

		TRACE_SOURCE((L"Capture::Pause"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_State != SourceStarted ? MF_E_INVALID_STATE_TRANSITION : S_OK));

		if(m_pScreenCaptureStream){
				IF_FAILED_RETURN(hr = m_pScreenCaptureStream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
		}

		IF_FAILED_RETURN(hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, NULL));

		m_State = SourcePaused;

		return hr;
}

STDMETHODIMP CScreenCapture::Shutdown(){

		TRACE_SOURCE((L"Capture::Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pScreenCaptureStream){
				LOG_HRESULT(m_pScreenCaptureStream->Shutdown());
		}

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		SAFE_RELEASE(m_pScreenCaptureStream);
		SAFE_RELEASE(m_pEventQueue);
		SAFE_RELEASE(m_pPresentationDescriptor);

		m_State = SourceShutdown;

		return hr;
}

STDMETHODIMP CScreenCapture::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){

		TRACE_SOURCE((L"Capture::Start"));

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

				IF_FAILED_THROW(hr = CheckShutdown());

				IF_FAILED_THROW(hr = (m_State == SourceInvalid ? E_FAIL: S_OK));

    IF_FAILED_THROW(hr = ValidatePresentationDescriptor(pPresentationDescriptor));

				IF_FAILED_THROW(hr = QueueNewStreamEvent(pPresentationDescriptor));
				
				var.vt = VT_I8;
				var.hVal.QuadPart = 0;

				IF_FAILED_THROW(hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent));

				IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));

				bQueuedStartEvent = TRUE;

				IF_FAILED_THROW(hr = (m_pScreenCaptureStream == NULL ? E_POINTER : S_OK));

				IF_FAILED_THROW(hr = m_pScreenCaptureStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));

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

STDMETHODIMP CScreenCapture::Stop(){

		TRACE_SOURCE((L"Capture::Stop"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_State = SourceStopped;

		if(m_pScreenCaptureStream)
				IF_FAILED_RETURN(hr = m_pScreenCaptureStream->Flush());

		if(m_pScreenCaptureStream){
				IF_FAILED_RETURN(hr = m_pScreenCaptureStream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
		}

		LOG_HRESULT(hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, NULL));

		return hr;
}

HRESULT CScreenCapture::CreateScreenCapturePresentationDescriptor(){

		TRACE_SOURCE((L"Capture::CreateScreenCapturePresentationDescriptor"));

		HRESULT hr = S_OK;

		IMFMediaType* pVideoType = NULL;
		IMFStreamDescriptor* pStreamDescriptor = NULL;
		IMFMediaTypeHandler* pVideoHandler = NULL;

		try{

				IF_FAILED_THROW(hr = CreateVideoMediaType(&pVideoType));

				IF_FAILED_THROW(hr = MFCreateStreamDescriptor(0, 1, &pVideoType, &pStreamDescriptor));

				IF_FAILED_THROW(hr = pStreamDescriptor->GetMediaTypeHandler(&pVideoHandler));
				IF_FAILED_THROW(hr = pVideoHandler->SetCurrentMediaType(pVideoType));

				IF_FAILED_THROW(hr = MFCreatePresentationDescriptor(1, &pStreamDescriptor, &m_pPresentationDescriptor));

				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(0));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoType);
		SAFE_RELEASE(pStreamDescriptor);
		SAFE_RELEASE(pVideoHandler);

		return hr;
}

HRESULT CScreenCapture::CreateVideoMediaType(IMFMediaType** ppType){

		TRACE_SOURCE((L"Capture::CreateVideoMediaType"));

		HRESULT hr = S_OK;
		
		IMFMediaType* pVideoType = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pVideoType));

				IF_FAILED_THROW(hr = pVideoType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));

				IF_FAILED_THROW(hr = pVideoType->SetGUID(MF_MT_SUBTYPE, VideoFormatProject));

				IF_FAILED_THROW(hr = MFSetAttributeSize(pVideoType, MF_MT_FRAME_SIZE, VIDEO_WIDTH, VIDEO_HEIGHT));
				IF_FAILED_THROW(hr = pVideoType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
				IF_FAILED_THROW(hr = MFSetAttributeRatio(pVideoType, MF_MT_FRAME_RATE, VIDEO_FRAME_RATE_NUM, VIDEO_FRAME_RATE_DEN));
				IF_FAILED_THROW(hr = pVideoType->SetUINT32(MF_MT_AVG_BITRATE, VIDEO_WIDTH * VIDEO_HEIGHT * VIDEO_BYTE_RGB * VIDEO_FRAME_RATE));
				IF_FAILED_THROW(hr = MFSetAttributeRatio(pVideoType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

    *ppType = pVideoType;
				(*ppType)->AddRef();

				//LogMediaType(pVideoType);
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoType);

		return hr;
}

HRESULT CScreenCapture::ValidatePresentationDescriptor(IMFPresentationDescriptor* pPD){

		TRACE_SOURCE((L"Capture::ValidatePresentationDescriptor"));

		HRESULT hr = S_OK;
		DWORD cStreams = 0;

		assert(pPD != NULL);

		BOOL fVideoSelected = FALSE;
		IMFStreamDescriptor* pVideoSD = NULL;
		IMFMediaTypeHandler* pVideoHandler = NULL;
		IMFMediaType*        pVideoMediaType = NULL;

		try{

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&cStreams));

				IF_FAILED_THROW(hr = (cStreams != 1 ? MF_E_UNSUPPORTED_REPRESENTATION : S_OK));

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(0, &fVideoSelected, &pVideoSD));
				IF_FAILED_THROW(hr = (fVideoSelected == FALSE ? MF_E_UNSUPPORTED_REPRESENTATION : S_OK));
				IF_FAILED_THROW(hr = pVideoSD->GetMediaTypeHandler(&pVideoHandler));
				IF_FAILED_THROW(hr = pVideoHandler->GetCurrentMediaType(&pVideoMediaType));

				// todo : Check pVideoMediaType
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoMediaType);
		SAFE_RELEASE(pVideoHandler);
		SAFE_RELEASE(pVideoSD);

		return hr;
}

HRESULT CScreenCapture::QueueNewStreamEvent(IMFPresentationDescriptor* pPD){
		
		TRACE_SOURCE((L"Capture::QueueNewStreamEvent"));

		assert(pPD != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pVideoSD = NULL;
		BOOL fVideoSelected = FALSE;
		DWORD cStreams = 0;

		try{

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&cStreams));

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(0, &fVideoSelected, &pVideoSD));
				
				assert(fVideoSelected);

				if(m_pScreenCaptureStream){

						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pScreenCaptureStream));
				}
				else{

						IF_FAILED_THROW(hr = CScreenCaptureStream::CreateInstance(&m_pScreenCaptureStream, this, pVideoSD, VIDEO_WIDTH * VIDEO_HEIGHT * VIDEO_OCTET_RGB, hr));

						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pScreenCaptureStream));
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoSD);

		return hr;
}