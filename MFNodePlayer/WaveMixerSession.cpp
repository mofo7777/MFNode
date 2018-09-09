//----------------------------------------------------------------------------------------------
// WaveMixerSession.cpp
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

CWaveMixerSession::CWaveMixerSession(PCWSTR pUrl1, PCWSTR pUrl2)
		: m_nRefCount(1),
		  m_sourceSampleReadyEvent1(NULL),
		  m_sourceSampleReadyEvent2(NULL),
				m_syncMftWorkerQueue(MFASYNC_CALLBACK_QUEUE_LONG_FUNCTION),
				m_sessionStarted(false),
				m_pClock(NULL),
				m_pReadySourceSample1(NULL),
				m_pReadySourceSample2(NULL),
		  m_szFileUrl1(pUrl1),
				m_szFileUrl2(pUrl2),
				m_pEventQueue(NULL),
				m_pByteStreamHandler1(NULL),
				m_pByteStreamHandler2(NULL),
				m_pPresentation1(NULL),
				m_pPresentation2(NULL),
				m_pWaveMixer(NULL),
				m_pResampler(NULL),
				m_pSink(NULL),
				m_pStreamSink(NULL),
				m_pSource1(NULL),
				m_pSource2(NULL),
				m_pSourceStream1(NULL),
				m_pSourceStream2(NULL),
				bTest(FALSE)
{
		TRACE_SESSION((L"Session::CTOR"));
		
		// Todo check CreateEvent
		for(int i = 0; i < 5; i++)
				m_hEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CWaveMixerSession::~CWaveMixerSession(){

		TRACE_SESSION((L"Session::DTOR"));

		if(m_syncMftWorkerQueue != MFASYNC_CALLBACK_QUEUE_LONG_FUNCTION){
				
				LOG_HRESULT(MFUnlockWorkQueue(m_syncMftWorkerQueue));
				m_syncMftWorkerQueue = MFASYNC_CALLBACK_QUEUE_LONG_FUNCTION;
		}

		CLOSE_EVENT_IF(m_sourceSampleReadyEvent1);
		CLOSE_EVENT_IF(m_sourceSampleReadyEvent2);

		for(int i = 0; i < 5; i++)
				CLOSE_EVENT_IF(m_hEvents[i]);
}

ULONG CWaveMixerSession::AddRef(){
		
		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Session::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CWaveMixerSession::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Session::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CWaveMixerSession::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_SESSION((L"Session::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CWaveMixerSession, IMFMediaSession),
				QITABENT(CWaveMixerSession, IMFAsyncCallback),
				QITABENT(CWaveMixerSession, IMFMediaEventGenerator),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

HRESULT CWaveMixerSession::Invoke(IMFAsyncResult* pResult){
		
		TRACE_SESSION((L"Session::Invoke"));

		HRESULT hr = S_OK;
		IMFAsyncState* pAsyncState = NULL;
		MFNodeMediaEvent EventType;

		try{

				IF_FAILED_THROW(hr = pResult->GetStatus());

				pAsyncState = reinterpret_cast<IMFAsyncState*>(pResult->GetStateNoAddRef());
				IF_FAILED_THROW(hr = (pAsyncState == NULL ? E_POINTER : S_OK));
    
				EventType = pAsyncState->EventType();

    if(EventType == ME_MFNodeByteStreamHandler){
						
						assert(m_pByteStreamHandler1 != NULL);
						assert(m_pSource1 == NULL);

						IF_FAILED_THROW(hr = HandleByteStreamHandlerEvent(pResult, m_pByteStreamHandler1, &m_pSource1));
				}
				else if(EventType == ME_MFNodeByteStreamHandler2){
						
						assert(m_pByteStreamHandler2 != NULL);
						assert(m_pSource2 == NULL);

						IF_FAILED_THROW(hr = HandleByteStreamHandlerEvent(pResult, m_pByteStreamHandler2, &m_pSource2));
				}
				else if(EventType == ME_MFNodeStreamSink){
						
						IF_FAILED_THROW(hr = HandleStreamSinkEvent(pResult));
				}
				else if(EventType == ME_MFNodeSource){

						IF_FAILED_THROW(hr = HandleSourceEvent(pResult, m_pSource1, &m_pSourceStream1, ME_MFNodeSource, ME_MFNodeSourceStream, 0));
				}
				else if(EventType == ME_MFNodeSource2){

						IF_FAILED_THROW(hr = HandleSourceEvent(pResult, m_pSource2, &m_pSourceStream2, ME_MFNodeSource2, ME_MFNodeSourceStream2, 1));
				}
				else if(EventType == ME_MFNodeSourceStream){

						assert(m_pReadySourceSample1 == NULL);
						
						IF_FAILED_THROW(hr = HandleSourceStreamEvent(pResult, m_pSourceStream1, &m_pReadySourceSample1, m_sourceSampleReadyEvent1, ME_MFNodeSourceStream, 2));
				}
				else if(EventType == ME_MFNodeSourceStream2){
						
						assert(m_pReadySourceSample2 == NULL);

						IF_FAILED_THROW(hr = HandleSourceStreamEvent(pResult, m_pSourceStream2, &m_pReadySourceSample2, m_sourceSampleReadyEvent2, ME_MFNodeSourceStream2, 3));
				}
				else if(EventType == ME_MFNodeSampleRequest){
						
						IF_FAILED_THROW(hr = HandleSynchronousMftRequest());
				}
				else{
						TRACE((L"Session : %s", MFEventString(EventType)));
				}
		}
		catch(HRESULT){}

		if(FAILED(hr)){
				LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(MEError, GUID_NULL, hr, NULL));
		}

		SAFE_RELEASE(pAsyncState);

		return S_OK;
}

HRESULT CWaveMixerSession::Start(const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){
		
		TRACE_SESSION((L"Session::Start"));

		HRESULT hr;
		IMFPresentationTimeSource* pTimeSource = NULL;

		assert(m_pPresentation1);
		assert(m_pPresentation2);
		assert(m_pSink);
		assert(m_pStreamSink);
		assert(m_pSource1);
		assert(m_pSource2);

		IF_FAILED_RETURN(hr = m_pSink->QueryInterface(IID_IMFPresentationTimeSource, reinterpret_cast<void**>(&pTimeSource)));

		try{
				
				if(!m_sessionStarted){
						
						m_sessionStarted = true;

						//IF_FAILED_THROW(hr = m_pSource1->Start(m_pPresentation1, pguidTimeFormat, pvarStartPosition));
						//IF_FAILED_THROW(hr = m_pSource2->Start(m_pPresentation2, pguidTimeFormat, pvarStartPosition));

						// See this for release : LOG_HRESULT(MFShutdownObject(m_pClock));
						IF_FAILED_THROW(hr = MFCreatePresentationClock(&m_pClock));
						IF_FAILED_THROW(hr = m_pClock->SetTimeSource(pTimeSource));
						IF_FAILED_THROW(hr = m_pSink->SetPresentationClock(m_pClock));

						//MFTIME mfTime;
						//m_pClock->GetTime(&mfTime);
						//MFTimeString(mfTime);

						//hr = m_pClock->Start(0);
				}

				MFCLOCK_STATE StateClock;
				IF_FAILED_THROW(hr = m_pClock->GetState(0, &StateClock));

				//MFTIME mfTime;
				//LOG_HRESULT(m_pClock->GetTime(&mfTime));
				//MFTimeString(mfTime);

				switch(StateClock){

				  case MFCLOCK_STATE_INVALID:
						case MFCLOCK_STATE_STOPPED:
								IF_FAILED_THROW(hr = m_pSource1->Start(m_pPresentation1, pguidTimeFormat, pvarStartPosition));
								IF_FAILED_THROW(hr = m_pSource2->Start(m_pPresentation2, pguidTimeFormat, pvarStartPosition));
								IF_FAILED_THROW(hr = ListenEvents());
								IF_FAILED_THROW(hr = m_pClock->Start(0));
								break;

						case MFCLOCK_STATE_PAUSED:
								LOG_HRESULT(hr = m_pWaveMixer->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
								LOG_HRESULT(hr = m_pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
								IF_FAILED_THROW(hr = m_pClock->Start(PRESENTATION_CURRENT_POSITION));
								break;
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTimeSource);

		return hr;
}

HRESULT CWaveMixerSession::Pause(){
		
		TRACE_SESSION((L"Session::Pause"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (m_pClock == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(hr = m_pClock->Pause());

		return hr;
}

HRESULT CWaveMixerSession::Stop(){
		
		TRACE_SESSION((L"Session::Stop"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (m_pClock == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(hr = StopStreaming());

		return hr;
}

HRESULT CWaveMixerSession::Close(){

		TRACE_SESSION((L"Session::Close"));

		HRESULT hr = S_OK;
		PROPVARIANT variantStatus;
		IMFMediaEvent* pEvent = NULL;

		assert(m_pEventQueue);

		if(m_pClock){

				MFCLOCK_STATE StateClock;
				IF_FAILED_THROW(hr = m_pClock->GetState(0, &StateClock));

				if(StateClock != MFCLOCK_STATE_STOPPED)
						LOG_HRESULT(hr = CloseStreaming());
		}

		try{

				PropVariantInit(&variantStatus);

				IF_FAILED_THROW(hr = MFCreateMediaEvent(MESessionClosed, GUID_NULL, hr, &variantStatus, &pEvent));

				// Todo : check if variantStatus can be NULL. We don't really need it.
				//IF_FAILED_THROW(hr = MFCreateMediaEvent(MESessionClosed, GUID_NULL, hr, NULL, &pEvent));

				IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pEvent);
		PropVariantClear(&variantStatus);
		
		return hr;
}

HRESULT CWaveMixerSession::Shutdown(){

		TRACE_SESSION((L"Session::Shutdown"));

		HRESULT hr = S_OK;
		
		SAFE_RELEASE(m_pByteStreamHandler1);
		SAFE_RELEASE(m_pByteStreamHandler2);
		SAFE_RELEASE(m_pSourceStream1);
		SAFE_RELEASE(m_pSourceStream2);

		if(m_pSource1){
				LOG_HRESULT(hr = m_pSource1->Shutdown());
				SAFE_RELEASE(m_pSource1);
		}

		if(m_pSource2){
				LOG_HRESULT(hr = m_pSource2->Shutdown());
				SAFE_RELEASE(m_pSource2);
		}

		SAFE_RELEASE(m_pStreamSink);

		if(m_pSink){
				LOG_HRESULT(hr = m_pSink->Shutdown());
				SAFE_RELEASE(m_pSink);
		}

		SAFE_RELEASE(m_pWaveMixer);
		SAFE_RELEASE(m_pResampler);
		SAFE_RELEASE(m_pPresentation1);
		SAFE_RELEASE(m_pPresentation2);

		if(m_pEventQueue != NULL){
				
				LOG_HRESULT(hr = m_pEventQueue->Shutdown());
				SAFE_RELEASE(m_pEventQueue);
		}

		m_szFileUrl1 = L"";
		m_szFileUrl2 = L"";

		SAFE_RELEASE(m_pReadySourceSample1);
		SAFE_RELEASE(m_pReadySourceSample2);

		SAFE_RELEASE(m_pClock);

		return hr;
}

HRESULT CWaveMixerSession::GetClock(IMFClock** ppClock){
		
		TRACE_SESSION((L"Session::GetClock"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (ppClock == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (m_pClock == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(hr = m_pClock->QueryInterface(IID_IMFClock, reinterpret_cast<void**>(ppClock)));

		return hr;
}

HRESULT CWaveMixerSession::GetSessionCapabilities(DWORD* pdwCaps){
		
		TRACE_SESSION((L"Session::GetSessionCapabilities"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (pdwCaps == NULL ? E_POINTER : S_OK));

		*pdwCaps = MFSESSIONCAP_START | MFSESSIONCAP_PAUSE;

		return hr;
}

HRESULT CWaveMixerSession::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){
		
		TRACE_EVENT((L"Session::GetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

HRESULT CWaveMixerSession::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){
		
		TRACE_EVENT((L"Session::EndGetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

		return hr;
}

HRESULT CWaveMixerSession::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){
		
		TRACE_EVENT((L"Session::GetEvent"));

		HRESULT hr;

		IMFMediaEventQueue* pQueue = NULL;

		{
				AutoLock lock(m_CriticSection);

				pQueue = m_pEventQueue;
				pQueue->AddRef();
		}

		LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));

		SAFE_RELEASE(pQueue);

		return hr;
}

HRESULT CWaveMixerSession::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){
		
		TRACE_EVENT((L"Session::QueueEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}

HRESULT CWaveMixerSession::InitTopology(){
		
		TRACE_SESSION((L"Session::InitTopology"));

		HRESULT hr = S_OK;

		IF_FAILED_RETURN(hr = MFAllocateWorkQueue(&m_syncMftWorkerQueue));

		IF_FAILED_RETURN(hr = MFCreateEventQueue(&m_pEventQueue));

		m_sourceSampleReadyEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
		IF_FAILED_RETURN(hr = (m_sourceSampleReadyEvent1 == NULL ? E_UNEXPECTED : S_OK));

		m_sourceSampleReadyEvent2 = CreateEvent(NULL, TRUE, FALSE, NULL);
		IF_FAILED_RETURN(hr = (m_sourceSampleReadyEvent2 == NULL ? E_UNEXPECTED : S_OK));

		IF_FAILED_RETURN(hr = LoadCustomTopology());

		return hr;
}

HRESULT CWaveMixerSession::StopStreaming(){

		HRESULT hr;

		LOG_HRESULT(hr = m_pClock->Stop());

		LOG_HRESULT(hr = m_pSource1->Stop());
		LOG_HRESULT(hr = m_pSource2->Stop());

		LOG_HRESULT(hr = m_pWaveMixer->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));
		LOG_HRESULT(hr = m_pResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));

		LOG_HRESULT(hr = m_pWaveMixer->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0));
		LOG_HRESULT(hr = m_pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0));

		DWORD dwEvent = WaitForMultipleObjects(5, m_hEvents, TRUE, INFINITE);
		assert(dwEvent == WAIT_OBJECT_0);

		/*dwEvent = WaitForSingleObject(m_sourceSampleReadyEvent1, 2000);

		if(dwEvent != WAIT_OBJECT_0)
				TRACE((L"StopStreaming dwEvent1 !"));
		
		dwEvent = WaitForSingleObject(m_sourceSampleReadyEvent2, 2000);

		if(dwEvent != WAIT_OBJECT_0)
				TRACE((L"StopStreaming dwEvent2 !"));

		ResetEvent(m_sourceSampleReadyEvent1);
		ResetEvent(m_sourceSampleReadyEvent2);*/

		return hr;
}

HRESULT CWaveMixerSession::CloseStreaming(){

		HRESULT hr;

		LOG_HRESULT(hr = m_pClock->Stop());

		LOG_HRESULT(hr = m_pSource1->Stop());
		LOG_HRESULT(hr = m_pSource2->Stop());

		LOG_HRESULT(hr = m_pWaveMixer->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM , 0));
		LOG_HRESULT(hr = m_pResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM , 0));

		LOG_HRESULT(hr = m_pWaveMixer->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0));
		LOG_HRESULT(hr = m_pResampler->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0));

		DWORD dwEvent = WaitForMultipleObjects(5, m_hEvents, TRUE, INFINITE);
		assert(dwEvent == WAIT_OBJECT_0);

		return hr;
}

HRESULT CWaveMixerSession::ListenEvents(){

		HRESULT hr = S_OK;
		IMFAsyncState* pState = NULL;

		try{

				pState = new (std::nothrow) CMFAsyncState(ME_MFNodeStreamSink);
				IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
				pState->AddRef();

				IF_FAILED_THROW(hr = m_pStreamSink->BeginGetEvent(this, pState));

				SAFE_RELEASE(pState);

				pState = new (std::nothrow) CMFAsyncState(ME_MFNodeSource);
				IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
				pState->AddRef();

				IF_FAILED_THROW(hr = m_pSource1->BeginGetEvent(this, pState));

				SAFE_RELEASE(pState);

				pState = new (std::nothrow) CMFAsyncState(ME_MFNodeSource2);
				IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
				pState->AddRef();

				IF_FAILED_THROW(hr = m_pSource2->BeginGetEvent(this, pState));

				if(m_pSourceStream1){

						SAFE_RELEASE(pState);

						pState = new (std::nothrow) CMFAsyncState(ME_MFNodeSourceStream);
						IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
						pState->AddRef();

						IF_FAILED_THROW(hr = m_pSourceStream1->BeginGetEvent(this, pState));
				}

				if(m_pSourceStream2){

						SAFE_RELEASE(pState);

						pState = new (std::nothrow) CMFAsyncState(ME_MFNodeSourceStream2);
						IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
						pState->AddRef();

						IF_FAILED_THROW(hr = m_pSourceStream2->BeginGetEvent(this, pState));
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pState);

		return hr;
}

HRESULT CWaveMixerSession::HandleByteStreamHandlerEvent(IMFAsyncResult* pResult, IMFByteStreamHandler* pByteStreamHandler, IMFMediaSource** ppSource){
		
		TRACE_SESSION((L"Session::HandleByteStreamHandlerEvent"));

		HRESULT hr = S_OK;
		MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
		IUnknown* pUnkSource = NULL;

		try{
				
				IF_FAILED_THROW(hr = pByteStreamHandler->EndCreateObject(pResult, &objectType, &pUnkSource));

				IF_FAILED_THROW(hr = (objectType != MF_OBJECT_MEDIASOURCE ? E_UNEXPECTED : S_OK));

				IF_FAILED_THROW(hr = pUnkSource->QueryInterface(IID_IMFMediaSource, reinterpret_cast<void**>(ppSource)));

				if(m_pSource1 && m_pSource2){

						IF_FAILED_THROW(hr = NegotiateMediaTypes());

						IF_FAILED_THROW(hr = FireTopologyReadyEvent());
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pUnkSource);

		return hr;
}

HRESULT CWaveMixerSession::HandleSourceEvent(IMFAsyncResult* pResult, IMFMediaSource* pSource, IMFMediaStream** ppSourceStream,
		const MFNodeMediaEvent SourceEvent, const MFNodeMediaEvent StreamEvent, const int iIndex){
		
		TRACE_SESSION((L"Session::HandleSourceEvent"));
		
		HRESULT hr = S_OK;
		IMFMediaEvent* pEvent = NULL;
		MediaEventType eventType;
		IMFAsyncState* pState = NULL;
		PROPVARIANT eventVariant;

		try{
				
				PropVariantInit(&eventVariant);

				IF_FAILED_THROW(hr = pSource->EndGetEvent(pResult, &pEvent));

				IF_FAILED_THROW(hr = pEvent->GetType(&eventType));

				TRACE_PIPELINE_EVENT((L"Source%d %s", iIndex + 1, MFEventString(eventType)));

				if(eventType == MESourceStopped){

						SetEvent(m_hEvents[iIndex]);
				}
				else{

						pState = new (std::nothrow) CMFAsyncState(SourceEvent);
						IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
						pState->AddRef();

						IF_FAILED_THROW(hr = pSource->BeginGetEvent(this, pState));

						if(eventType == MENewStream){

								assert(*ppSourceStream == NULL);

								IF_FAILED_THROW(hr = pEvent->GetValue(&eventVariant));

								IF_FAILED_THROW(hr = eventVariant.punkVal->QueryInterface(IID_IMFMediaStream, reinterpret_cast<void**>(ppSourceStream)));

								SAFE_RELEASE(pState);

								pState = new (std::nothrow) CMFAsyncState(StreamEvent);
								IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
								pState->AddRef();

								IF_FAILED_THROW(hr = (*ppSourceStream)->BeginGetEvent(this, pState));
						}
				}
		}
		catch(HRESULT){}

		PropVariantClear(&eventVariant);
		SAFE_RELEASE(pEvent);
		SAFE_RELEASE(pState);

		return hr;
}

HRESULT CWaveMixerSession::HandleSourceStreamEvent(IMFAsyncResult* pResult, IMFMediaStream* pSourceStream, IMFSample** ppSample,
		HANDLE phEvent, const MFNodeMediaEvent StreamEvent, const int iIndex){
		
		TRACE_SESSION((L"Session::HandleSourceStreamEvent"));
		
		HRESULT hr = S_OK;
		IMFMediaEvent* pEvent = NULL;
		MediaEventType eventType;
		IMFAsyncState* pState = NULL;
		PROPVARIANT eventVariant;

		try{
				
				PropVariantInit(&eventVariant);

				IF_FAILED_THROW(hr = pSourceStream->EndGetEvent(pResult, &pEvent));

				IF_FAILED_THROW(hr = pEvent->GetType(&eventType));

				TRACE_PIPELINE_EVENT((L"Stream%d %s", iIndex - 1, MFEventString(eventType)));

				if(eventType == MEStreamStopped){

						SetEvent(m_hEvents[iIndex]);
				}
				else{

						pState = new (std::nothrow) CMFAsyncState(StreamEvent);
						IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
						pState->AddRef();

						IF_FAILED_THROW(hr = pSourceStream->BeginGetEvent(this, pState));

						if(eventType == MEMediaSample){

								IF_FAILED_THROW(hr = pEvent->GetValue(&eventVariant));

								IF_FAILED_THROW(hr = eventVariant.punkVal->QueryInterface(IID_IMFSample, reinterpret_cast<void**>(ppSample)));

								SetEvent(phEvent);
						}
						else{

								if(eventType == MEEndOfStream){

										IF_FAILED_THROW(hr = Stop());
								}
						}						
				}
		}
		catch(HRESULT){}

		PropVariantClear(&eventVariant);
		SAFE_RELEASE(pEvent);
		SAFE_RELEASE(pState);

		return hr;
}

HRESULT CWaveMixerSession::HandleStreamSinkEvent(IMFAsyncResult* pResult){
		
		TRACE_SESSION((L"Session::HandleStreamSinkEvent"));

		HRESULT hr = S_OK;
		IMFMediaEvent* pEvent = NULL;
		MediaEventType eventType;
		IMFAsyncState* pState = NULL;

		try{
				
				IF_FAILED_THROW(hr = m_pStreamSink->EndGetEvent(pResult, &pEvent));

				IF_FAILED_THROW(hr = pEvent->GetType(&eventType));

				TRACE_PIPELINE_EVENT((L"Sink %s", MFEventString(eventType)));

				if(eventType == MEStreamSinkStopped){

						SetEvent(m_hEvents[4]);
				}
				else{

						pState = new (std::nothrow) CMFAsyncState(ME_MFNodeStreamSink);
						IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
						pState->AddRef();

						IF_FAILED_THROW(hr = m_pStreamSink->BeginGetEvent(this, pState));

						if(eventType == MEStreamSinkRequestSample){

								SAFE_RELEASE(pState);

								pState = new (std::nothrow) CMFAsyncState(ME_MFNodeSampleRequest);
								IF_FAILED_THROW(hr = (pState == NULL ? E_OUTOFMEMORY : S_OK));
								pState->AddRef();

								IF_FAILED_THROW(hr = MFPutWorkItem(m_syncMftWorkerQueue, this, pState));
						}
						/*else{

								TRACE((L"Sink %s", MFEventString(eventType)));
						}*/
				}
		}
	catch(HRESULT){}

		SAFE_RELEASE(pState);
		SAFE_RELEASE(pEvent);

		return hr;
}

HRESULT CWaveMixerSession::HandleSynchronousMftRequest(){
		
		TRACE_SESSION((L"Session::HandleSynchronousMftRequest"));

		HRESULT hr = S_OK;
		IMFSample* pSample = NULL;
		//MFCLOCK_STATE ClockState;

		AutoLock lock(m_CriticSection);

		if(bTest)
				return hr;

		try{

				//IF_FAILED_THROW(hr = m_pClock->GetState(0, &ClockState));

				//if(ClockState == MFCLOCK_STATE_RUNNING){

						IF_FAILED_THROW(hr = PullDataFromMFT(m_pResampler, &pSample));

						IF_FAILED_THROW(hr = m_pStreamSink->ProcessSample(pSample));
				//}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSample);

		return hr;
}

HRESULT CWaveMixerSession::PullDataFromMFT(IMFTransform* pMft, IMFSample** ppNewSample){
		
		TRACE_SESSION((L"Session::PullDataFromMFT"));

		HRESULT hr;
		MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
		DWORD processOutputStatus = 0;
		IMFSample* pMftInputSample1 = NULL;
		IMFSample* pMftInputSample2 = NULL;
		DWORD inputStreamId = 0;

		IF_FAILED_RETURN(hr = (pMft == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (ppNewSample == NULL ? E_POINTER : S_OK));

		if(bTest)
				return hr;

		try{

				IF_FAILED_THROW(hr = InitOutputDataBuffer(pMft, &outputDataBuffer));

				while(true){
						
						hr = pMft->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);

						if(hr != S_OK && hr != MF_E_TRANSFORM_NEED_MORE_INPUT){

								if(pMft == m_pResampler){
										TRACE((L"Resampler ProcessOutput hr = %s", MFErrorString(hr)));
								}
								else{
										TRACE((L"WaveMixer ProcessOutput hr = %s", MFErrorString(hr)));
								}
						}

						if(hr != MF_E_TRANSFORM_NEED_MORE_INPUT){
								break;
						}

						if(pMft == m_pResampler){

								IF_FAILED_THROW(hr = PullDataFromMFT(m_pWaveMixer, &pMftInputSample1));
								IF_FAILED_THROW(hr = pMft->ProcessInput(inputStreamId, pMftInputSample1, 0));
						}
						else{
								
								IF_FAILED_THROW(hr = PullDataFromSource(&pMftInputSample1, &pMftInputSample2));
								
								IF_FAILED_THROW(hr = pMft->ProcessInput(inputStreamId, pMftInputSample1, 0));
								
								IF_FAILED_THROW(hr = pMft->ProcessInput(1, pMftInputSample2, 0));
						}
				}

				// Todo : dont't know why, but hr can be S_OK and pSample NULL.
				if(outputDataBuffer.pSample == NULL){
						
						AutoLock lock(m_CriticSection);
						bTest = TRUE;
						IF_FAILED_THROW(hr = E_POINTER);
				}
				else{
						
						*ppNewSample = outputDataBuffer.pSample;
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pMftInputSample1);
		SAFE_RELEASE(pMftInputSample2);

		return hr;
}

HRESULT CWaveMixerSession::InitOutputDataBuffer(IMFTransform* pMFTransform, MFT_OUTPUT_DATA_BUFFER* pOutputBuffer){
		
		TRACE_SESSION((L"Session::InitOutputDataBuffer"));

		HRESULT hr = S_OK;
		MFT_OUTPUT_STREAM_INFO outputStreamInfo;
		DWORD outputStreamId = 0;
		IMFSample* pOutputSample = NULL;
		IMFMediaBuffer* pMediaBuffer = NULL;

		try{
				
				ZeroMemory(&outputStreamInfo, sizeof(outputStreamInfo));
				ZeroMemory(pOutputBuffer, sizeof(*pOutputBuffer));

				IF_FAILED_THROW(hr = pMFTransform->GetOutputStreamInfo(outputStreamId, &outputStreamInfo));

				if((outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0 &&
						 (outputStreamInfo.dwFlags & MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES) == 0){
									
						IF_FAILED_THROW(hr = MFCreateSample(&pOutputSample));

						IF_FAILED_THROW(hr = MFCreateMemoryBuffer(1048576, &pMediaBuffer));
						
						IF_FAILED_THROW(hr = pOutputSample->AddBuffer(pMediaBuffer));

						pOutputBuffer->pSample = pOutputSample;
						pOutputBuffer->pSample->AddRef();
				}

				pOutputBuffer->dwStreamID = outputStreamId;
  }
		catch(HRESULT){}

		SAFE_RELEASE(pMediaBuffer);
		SAFE_RELEASE(pOutputSample);
		
		return hr;
}

HRESULT CWaveMixerSession::PullDataFromSource(IMFSample** ppNewSample1, IMFSample** ppNewSample2){
		
		TRACE_SESSION((L"Session::PullDataFromSource"));

		HRESULT hr = S_OK;

		try{
				
				// Todo : m_pSourceStream2 can be first to return MF_E_END_OF_STREAM...
				// We have to better handle errors here.
				hr = m_pSourceStream1->RequestSample(NULL);

				if(hr == MF_E_END_OF_STREAM)
						return S_OK;

				hr = m_pSourceStream2->RequestSample(NULL);

				if(hr == MF_E_END_OF_STREAM){

						DWORD dwEvent = WaitForSingleObject(m_sourceSampleReadyEvent1, 2000);

      #ifdef _DEBUG
						  if(dwEvent != WAIT_OBJECT_0)
								  TRACE((L"dwEvent !"));
						#endif

						return S_OK;
				}

				DWORD dwEvent1 = WaitForSingleObject(m_sourceSampleReadyEvent1, 2000);
				DWORD dwEvent2 = WaitForSingleObject(m_sourceSampleReadyEvent2, 2000);

				#ifdef _DEBUG
				  if(dwEvent1 != WAIT_OBJECT_0)
						  TRACE((L"dwEvent1 !"));

				  if(dwEvent2 != WAIT_OBJECT_0)
						  TRACE((L"dwEvent2 !"));
				#endif

				IF_FAILED_THROW(hr = (m_pReadySourceSample1 == NULL ? E_UNEXPECTED : S_OK));
				IF_FAILED_THROW(hr = (m_pReadySourceSample2 == NULL ? E_UNEXPECTED : S_OK));

				*ppNewSample1 = m_pReadySourceSample1;
				(*ppNewSample1)->AddRef();
				*ppNewSample2 = m_pReadySourceSample2;
				(*ppNewSample2)->AddRef();

				ResetEvent(m_sourceSampleReadyEvent1);
				ResetEvent(m_sourceSampleReadyEvent2);
		}
		catch(HRESULT){}

		// if FAILED stop streaming.

		SAFE_RELEASE(m_pReadySourceSample1);
		SAFE_RELEASE(m_pReadySourceSample2);

		return hr;
}