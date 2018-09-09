//----------------------------------------------------------------------------------------------
// MFNodePlayer.cpp
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
#include "../MFSkVideoRenderer/VideoShaderEffect_i.c"
#include "../MFSkDxva2Renderer/Dxva2RendererSettings_i.c"

CMFNodePlayer::CMFNodePlayer(const HWND hWnd)
	: m_pSession(NULL),
	m_pSequencerSource(NULL),
	m_pSource(NULL),
	m_pSource2(NULL),
	m_pSink(NULL),
	m_pVideoDisplay(NULL),
	m_pVideoShaderEffect(NULL),
	m_pDxva2RendererSettings(NULL),
	m_state(SessionClosed),
	m_nRefCount(1),
	m_closeCompleteEvent(NULL),
	m_llDuration(0),
	m_hWindow(hWnd),
	m_CurrentSession(SESSION_NONE),
	m_dwSegmentId1((DWORD)-1),
	m_dwSegmentId2((DWORD)-1)
#ifdef MF_TRACE_PLAYER_EVENT
	, m_dwBegin(0),
	m_dwEnd(0)
#endif
{
	TRACE_PLAYER((L"Player::CTOR"));
	m_closeCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

CMFNodePlayer::~CMFNodePlayer(){

	TRACE_PLAYER((L"Player::DTOR"));

	CLOSE_EVENT_IF(m_closeCompleteEvent);

#ifdef MF_TRACE_PLAYER_EVENT
	assert(m_dwBegin == m_dwEnd);
#endif
}

HRESULT CMFNodePlayer::QueryInterface(REFIID riid, void** ppv){

	TRACE_PLAYER((L"Player::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CMFNodePlayer, IMFAsyncCallback),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMFNodePlayer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Player::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFNodePlayer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Player::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMFNodePlayer::Invoke(IMFAsyncResult* pAsyncResult){

	TRACE_PLAYER((L"Player::Invoke"));

	IMFMediaEvent* pEvent = NULL;
	HRESULT hr = S_OK;
	HRESULT hrStatus;
	MediaEventType EventType;

	AutoLock lock(m_CriticSection);

	try{

		IF_FAILED_THROW(hr = m_pSession->EndGetEvent(pAsyncResult, &pEvent));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwEnd++;
#endif

		IF_FAILED_THROW(hr = pEvent->GetType(&EventType));

		TRACE_PLAYER_EVENT((L"Player %s", MFEventString(EventType)));

		IF_FAILED_THROW(hr = pEvent->GetStatus(&hrStatus));

		if(FAILED(hrStatus)){

			// Todo after test EventType == MESessionClosed
			LOG_HRESULT(hr = hrStatus);

			IF_FAILED_THROW(hr = m_pSession->BeginGetEvent(this, NULL));
			SAFE_RELEASE(pEvent);

#ifdef MF_TRACE_PLAYER_EVENT
			m_dwBegin++;
#endif
			// Todo check hrStatus (MF_E_TOPO_CODEC_NOT_FOUND)
			{
				AutoLock lock(m_CriticSection);
				m_state = SessionAbort;
				PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
			}

			return hr;
		}

		if(EventType == MESessionClosed){

			SetEvent(m_closeCompleteEvent);

			{
				AutoLock lock(m_CriticSection);
				m_state = SessionClosed;
				m_CurrentSession = SESSION_NONE;
				PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
			}
		}
		else{

			IF_FAILED_THROW(hr = m_pSession->BeginGetEvent(this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
			m_dwBegin++;
#endif

			// if you are using HttpStreamer Session, this Occurs when you close VLC.
			if(EventType == MEError){

				LOG_HRESULT(hr = hrStatus);

				SAFE_RELEASE(pEvent);

				{
					AutoLock lock(m_CriticSection);
					m_state = SessionStopped;
					PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
				}
				return hr;
			}
		}

		if(m_state != SessionClosing){

			// We should refactor with one switch... no if/else, since code is growing
			if(EventType == MESessionStarted || EventType == MESessionPaused || EventType == MESessionStopped || EventType == MEEndOfPresentation){

				{
					AutoLock lock(m_CriticSection);

					switch(EventType){

					case MESessionStarted: m_state = SessionStarted; break;
					case MESessionPaused: m_state = SessionPaused; break;
					case MESessionStopped: m_state = SessionStopped; break;
					case MEEndOfPresentation: m_state = SessionStopped; break;
					}

					LPARAM lParam;

					if(IsVideoSession())
						lParam = 1;
					else
						lParam = 0;

					PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, lParam);
				}
			}
			else if(EventType == MESessionTopologyStatus){

				MF_TOPOSTATUS TopoStatus = MF_TOPOSTATUS_INVALID;

				IF_FAILED_THROW(hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, (UINT32*)&TopoStatus));

				TRACE_PLAYER_EVENT((L"Player TopoStatus %s", MFTopologyStatusString(TopoStatus)));

				if(TopoStatus == MF_TOPOSTATUS_READY){

					{
						AutoLock lock(m_CriticSection);
						m_state = SessionReady;

						if(m_CurrentSession == SESSION_SEQUENCER){

							SAFE_RELEASE(m_pVideoDisplay);
							MFGetService(m_pSession, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), reinterpret_cast<void**>(&m_pVideoDisplay));
						}
						else if(m_CurrentSession == SESSION_SCREENSHOT || m_CurrentSession == SESSION_FLV || m_CurrentSession == SESSION_AVCAPTURE){

							assert(m_pVideoDisplay == NULL);
							MFGetService(m_pSession, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFVideoDisplayControl), reinterpret_cast<void**>(&m_pVideoDisplay));
						}
						else if(m_CurrentSession == SESSION_DXVA2){

							assert(m_pSink != NULL);
							assert(m_pDxva2RendererSettings == NULL);
							IF_FAILED_THROW(hr = m_pSink->QueryInterface(IID_IMFDxva2RendererSettings, reinterpret_cast<void**>(&m_pDxva2RendererSettings)));
						}
						else if(m_CurrentSession == SESSION_RENDERER){

							assert(m_pSink != NULL);
							assert(m_pVideoShaderEffect == NULL);
							IF_FAILED_THROW(hr = m_pSink->QueryInterface(IID_IMFVideoShaderEffect, reinterpret_cast<void**>(&m_pVideoShaderEffect)));
						}

						PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
					}
				}
			}
			else if(EventType == MENewPresentation && m_CurrentSession == SESSION_SEQUENCER){

				IMFPresentationDescriptor* pPD = NULL;
				IMFMediaSourceTopologyProvider* pTopoProvider = NULL;
				IMFTopology* pTopology = NULL;

				try{

					IF_FAILED_THROW(hr = GetDescriptor(pEvent, &pPD));
					IF_FAILED_THROW(hr = m_pSequencerSource->QueryInterface(IID_PPV_ARGS(&pTopoProvider)));
					IF_FAILED_THROW(hr = pTopoProvider->GetMediaSourceTopology(pPD, &pTopology));
					IF_FAILED_THROW(hr = m_pSession->SetTopology(NULL, pTopology));
				}
				catch(HRESULT){}

				SAFE_RELEASE(pTopology);
				SAFE_RELEASE(pTopoProvider);
				SAFE_RELEASE(pPD);
			}
		}
	}
	catch(HRESULT){}

	SAFE_RELEASE(pEvent);

	return S_OK;
}

HRESULT CMFNodePlayer::OpenWaveMixer(PCWSTR sURL1, PCWSTR sURL2){

	TRACE_PLAYER((L"Player::OpenWaveMixer"));

	HRESULT hr = S_OK;
	CWaveMixerSession* pSession = NULL;

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	try{

		// Todo : check session and MFNodeForm behaviour if this failed before BeginGetEvent...
		pSession = new (std::nothrow) CWaveMixerSession(sURL1, sURL2);
		IF_FAILED_THROW(hr = (pSession == NULL ? E_OUTOFMEMORY : S_OK));

		IF_FAILED_THROW(hr = pSession->InitTopology());

		IF_FAILED_THROW(hr = pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		// Should never failed, just AddRef
		IF_FAILED_THROW(hr = pSession->QueryInterface(IID_IMFMediaSession, reinterpret_cast<void**>(&m_pSession)));

		m_CurrentSession = SESSION_WAVEMIXER;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSession);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenScreenShot(){

	TRACE_PLAYER((L"Player::OpenScreenShot"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, L"Screen:", MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE));
		IF_FAILED_THROW(hr = CreateScreenShotTopology(&pTopology));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_SCREENSHOT;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenHttpStreamer(){

	TRACE_PLAYER((L"Player::OpenHttpStreamer"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, L"Screen:", MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE));
		IF_FAILED_THROW(hr = CreateHttpTopology(&pTopology));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_HTTP;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenFlvFile(PCWSTR wszFile){

	TRACE_PLAYER((L"Player::OpenFlvFile"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, wszFile, MF_RESOLUTION_MEDIASOURCE));
		IF_FAILED_THROW(hr = CreateTopology(&pTopology, m_pSource));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_FLV;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenFileRenderer(PCWSTR wszFile, const CUDA_DECODER decoder){

	TRACE_PLAYER((L"Player::OpenFileRenderer"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;
	GUID decoderGuid = CLSID_MFTMpeg2Decoder;

	if(decoder == CUDA_DECODER_NONE){
		decoderGuid = CLSID_MFTMpeg2Decoder;
	}
	else if(decoder == CUDA_DECODER_CUDA){
		decoderGuid = CLSID_MFTCudaDecoder;
	}
	else{
		IF_FAILED_RETURN(hr = E_FAIL);
	}

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, wszFile, MF_RESOLUTION_MEDIASOURCE));
		IF_FAILED_THROW(hr = CreateRendererTopology(&pTopology, m_pSource, decoderGuid, CLSID_MFSkVideoRenderer));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_RENDERER;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenSequencer(PCWSTR wszUrl1, PCWSTR wszUrl2){

	TRACE_PLAYER((L"Player::OpenSequencer"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology1 = NULL;
	IMFTopology* pTopology2 = NULL;
	IMFSequencerSource* pSequencerSource = NULL;

	IMFMediaSource* pSource1 = NULL;
	IMFMediaSource* pSource2 = NULL;
	IMFMediaSource* pSource = NULL;
	IMFPresentationDescriptor* pPD = NULL;
	IMFMediaSourceTopologyProvider* pTopoProvider = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&pSource1, wszUrl1, MF_RESOLUTION_MEDIASOURCE));
		IF_FAILED_THROW(hr = CreateMediaSource(&pSource2, wszUrl2, MF_RESOLUTION_MEDIASOURCE));

		IF_FAILED_THROW(hr = CreateTopology(&pTopology1, pSource1));
		IF_FAILED_THROW(hr = CreateTopology(&pTopology2, pSource2));

		IF_FAILED_THROW(hr = MFCreateSequencerSource(NULL, &pSequencerSource));

		IF_FAILED_THROW(hr = pSequencerSource->AppendTopology(pTopology1, SequencerTopologyFlags_Last, &m_dwSegmentId1));
		IF_FAILED_THROW(hr = pSequencerSource->AppendTopology(pTopology2, SequencerTopologyFlags_Last, &m_dwSegmentId2));

		SAFE_RELEASE(pTopology1);

		// It seems that we need to query source from Sequencer... not m_pSource
		IF_FAILED_THROW(hr = pSequencerSource->QueryInterface(IID_IMFMediaSource, reinterpret_cast<void**>(&pSource)));

		IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pPD));
		IF_FAILED_THROW(hr = pSequencerSource->QueryInterface(IID_PPV_ARGS(&pTopoProvider)));
		IF_FAILED_THROW(hr = pTopoProvider->GetMediaSourceTopology(pPD, &pTopology1));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology1));

		m_CurrentSession = SESSION_SEQUENCER;

		m_pSource = pSource1;
		m_pSource->AddRef();

		m_pSource2 = pSource2;
		m_pSource2->AddRef();

		m_pSequencerSource = pSequencerSource;
		m_pSequencerSource->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopoProvider);
	SAFE_RELEASE(pPD);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pSource1);
	SAFE_RELEASE(pSource2);

	SAFE_RELEASE(pSequencerSource);
	SAFE_RELEASE(pTopology2);
	SAFE_RELEASE(pTopology1);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenAVCapture(){

	TRACE_PLAYER((L"Player::OpenAVCapture"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateCaptureSource(&m_pSource));
		IF_FAILED_THROW(hr = CreateTopology(&pTopology, m_pSource));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_AVCAPTURE;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::OpenFileDxva2(PCWSTR wszFile){

	TRACE_PLAYER((L"Player::OpenFileDxva2"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, wszFile, MF_RESOLUTION_MEDIASOURCE));
		// CLSID_MFTCudaDecoder CLSID_MFTMpeg2Decoder
		IF_FAILED_THROW(hr = CreateRendererTopology(&pTopology, m_pSource, CLSID_MFTCudaDecoder, CLSID_MFSkDxva2Renderer));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_DXVA2;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}

HRESULT CMFNodePlayer::Play(){

	TRACE_PLAYER((L"Player::Play"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = ((m_state != SessionPaused && m_state != SessionStopped && m_state != SessionReady) ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = (m_pSession == NULL ? E_UNEXPECTED : S_OK));

	PROPVARIANT varStart;
	PropVariantInit(&varStart);

	varStart.vt = VT_EMPTY;

	LOG_HRESULT(hr = m_pSession->Start(&GUID_NULL, &varStart));

	PropVariantClear(&varStart);

	return hr;
}

HRESULT CMFNodePlayer::Pause(){

	TRACE_PLAYER((L"Player::Pause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_state != SessionStarted ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = (m_pSession == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = m_pSession->Pause());

	return hr;
}

HRESULT CMFNodePlayer::Stop(){

	TRACE_PLAYER((L"Player::Stop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = ((m_state != SessionStarted && m_state != SessionPaused) ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = (m_pSession == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = m_pSession->Stop());

	return hr;
}

HRESULT CMFNodePlayer::CloseSession(){

	TRACE_PLAYER((L"Player::CloseSession"));

	HRESULT hr = S_OK;
	DWORD dwWaitResult = 0;

	{
		AutoLock lock(m_CriticSection);

		if(m_state == SessionClosed){
			PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
			return hr;
		}

		m_state = SessionClosing;
		//PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)SessionClosing, (LPARAM)0);
	}

	//if(m_CurrentSession == SESSION_SCREENSHOT)
	SAFE_RELEASE(m_pVideoDisplay);

	//if(m_CurrentSession == SESSION_RENDERER)
	SAFE_RELEASE(m_pVideoShaderEffect);

	//if(m_CurrentSession == SESSION_DXVA2)
	SAFE_RELEASE(m_pDxva2RendererSettings);

	if(m_pSession != NULL){

		LOG_HRESULT(hr = m_pSession->Close());

		if(SUCCEEDED(hr)){

			dwWaitResult = WaitForSingleObject(m_closeCompleteEvent, 5000);

			if(dwWaitResult == WAIT_TIMEOUT){

				assert(FALSE);
				//TRACE((L"WAIT_TIMEOUT !!"));
			}
		}
		/*else{
				// Big problem... but sometimes we can bypass errors...
		}*/
	}

	if(m_pSink){

		LOG_HRESULT(m_pSink->Shutdown());
		SAFE_RELEASE(m_pSink);
	}

	if(m_pSource){

		LOG_HRESULT(m_pSource->Shutdown());
		SAFE_RELEASE(m_pSource);
	}

	if(m_pSource2){

		LOG_HRESULT(m_pSource2->Shutdown());
		SAFE_RELEASE(m_pSource2);
	}

	if(m_pSequencerSource != NULL){

		LOG_HRESULT(m_pSequencerSource->DeleteTopology(m_dwSegmentId1));
		LOG_HRESULT(m_pSequencerSource->DeleteTopology(m_dwSegmentId2));

		SAFE_RELEASE(m_pSequencerSource);
	}

	if(m_pSession != NULL){

		LOG_HRESULT(hr = m_pSession->Shutdown());

		ULONG ulTest = m_pSession->Release();
		m_pSession = NULL;

		assert(ulTest == 0);
	}
	else{

		// We will go here if CreateMediaSource or CreateTopology failed and session is aborted.
		// SessionAbort lets a chance to source or sink to be shutdown and release : we should check if it behaves as expected...

		// Not sure if lock is needed
		AutoLock lock(m_CriticSection);

		m_state = SessionClosed;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	/*{
			AutoLock lock(m_CriticSection);
			m_state = SessionClosed;
			m_CurrentSession = SESSION_NONE;
	}*/

	m_llDuration = 0;

	return hr;
}

HRESULT CMFNodePlayer::RedrawVideo(){

	TRACE_PLAYER((L"Player::RedrawVideo"));

	HRESULT hr = S_OK;
	AutoLock lock(m_CriticSection);

	if(m_state != SessionClosing && m_pVideoDisplay){
		hr = m_pVideoDisplay->RepaintVideo();
	}

	return hr;
}

HRESULT CMFNodePlayer::ResizeVideo(WORD width, WORD height){

	TRACE_PLAYER((L"Player::ResizeVideo"));

	HRESULT hr = S_OK;
	AutoLock lock(m_CriticSection);

	if(m_state != SessionClosing && m_pVideoDisplay){

		RECT rcDest = {0, 0, width, height};

		hr = m_pVideoDisplay->SetVideoPosition(NULL, &rcDest);
	}

	return hr;
}

HRESULT CMFNodePlayer::GetDescriptor(IMFMediaEvent* pEvent, IMFPresentationDescriptor** ppDescriptor){

	TRACE_PLAYER((L"Player::GetDescriptor"));

	PROPVARIANT var;
	HRESULT hr = pEvent->GetValue(&var);

	if(SUCCEEDED(hr)){

		if(var.vt == VT_UNKNOWN){
			hr = var.punkVal->QueryInterface(ppDescriptor);
		}
		else{
			hr = MF_E_INVALIDTYPE;
		}

		PropVariantClear(&var);
	}

	return hr;
}

#ifdef TEST_CUDA_DECODER
HRESULT CMFNodePlayer::OpenCudaRenderer(PCWSTR wszFile){

	TRACE_PLAYER((L"Player::OpenCudaRenderer"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, wszFile, MF_RESOLUTION_MEDIASOURCE));
		IF_FAILED_THROW(hr = CreateCudaTopology(&pTopology, m_pSource));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_FLV;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}
#elif defined TEST_DXVA2_DECODER
HRESULT CMFNodePlayer::OpenDxva2Decoder(PCWSTR wszFile){

	TRACE_PLAYER((L"Player::OpenDxva2Decoder"));

	AutoLock lock(m_CriticSection);

	assert(m_pSession == NULL && m_CurrentSession == SESSION_NONE);

	m_state = SessionOpenPending;
	PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);

	HRESULT hr = S_OK;
	IMFTopology* pTopology = NULL;

	try{

		IF_FAILED_THROW(hr = CreateMediaSource(&m_pSource, wszFile, MF_RESOLUTION_MEDIASOURCE));
		IF_FAILED_THROW(hr = CreateDxva2Topology(&pTopology, m_pSource));

		IF_FAILED_THROW(hr = MFCreateMediaSession(NULL, &m_pSession));
		IF_FAILED_THROW(hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL));

#ifdef MF_TRACE_PLAYER_EVENT
		m_dwBegin++;
#endif

		IF_FAILED_THROW(hr = m_pSession->SetTopology(0, pTopology));

		m_CurrentSession = SESSION_FLV;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pTopology);

	if(FAILED(hr)){

		m_state = SessionAbort;
		PostMessage(m_hWindow, WM_APP_PLAYER_EVENT, (WPARAM)m_state, (LPARAM)0);
	}

	return hr;
}
#endif