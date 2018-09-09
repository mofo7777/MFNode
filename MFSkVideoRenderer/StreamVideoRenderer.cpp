//----------------------------------------------------------------------------------------------
// StreamVideoRenderer.cpp
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

CStreamVideoRenderer::CStreamVideoRenderer(CSinkVideoRenderer* pSinkVideoRenderer, HRESULT& hr)
	: m_nRefCount(1),
	m_State(StreamTypeNotSet),
	m_pEventQueue(NULL),
	m_pMediaType(NULL),
	m_pSinkVideoRenderer(NULL)
{

	TRACE_STREAM((L"StreamRenderer::CTOR"));

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));

	m_pSinkVideoRenderer = pSinkVideoRenderer;
	m_pSinkVideoRenderer->AddRef();
}

CStreamVideoRenderer::~CStreamVideoRenderer(){

	TRACE_STREAM((L"StreamRenderer::DTOR"));

	Shutdown();
}

HRESULT CStreamVideoRenderer::CreateInstance(CSinkVideoRenderer* pSink, CStreamVideoRenderer** ppStream, HRESULT& hr){

	TRACE_SINK((L"StreamRenderer::CreateInstance"));

	IF_FAILED_RETURN(hr = (ppStream == NULL ? E_INVALIDARG : S_OK));

	CStreamVideoRenderer* pStream = new (std::nothrow)CStreamVideoRenderer(pSink, hr);

	IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

	*ppStream = pStream;
	(*ppStream)->AddRef();

	SAFE_RELEASE(pStream);

	return hr;
}

HRESULT CStreamVideoRenderer::QueryInterface(REFIID riid, void** ppv){

	TRACE_STREAM((L"StreamRenderer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CStreamVideoRenderer, IMFStreamSink),
			QITABENT(CStreamVideoRenderer, IMFMediaEventGenerator),
			QITABENT(CStreamVideoRenderer, IMFMediaTypeHandler),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CStreamVideoRenderer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"StreamRenderer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CStreamVideoRenderer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"StreamRenderer::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CStreamVideoRenderer::Shutdown(){

	TRACE_STREAM((L"StreamRenderer::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pMediaType);
	SAFE_RELEASE(m_pSinkVideoRenderer);
	SAFE_RELEASE(m_pEventQueue);

	m_State = StreamFinalized;

	return S_OK;
}

HRESULT CStreamVideoRenderer::Start(MFTIME){

	TRACE_STREAM((L"StreamRenderer::Start"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));

	if(m_State != StreamStarted){

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));
		m_State = StreamStarted;
	}

	return hr;
}

HRESULT CStreamVideoRenderer::Stop(){

	TRACE_STREAM((L"StreamRenderer::Stop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL));

	m_State = StreamStopped;

	return hr;
}

HRESULT CStreamVideoRenderer::Pause(){

	TRACE_STREAM((L"StreamRenderer::Pause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized || m_State == StreamStopped ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL));

	m_State = StreamPaused;

	return hr;
}

HRESULT CStreamVideoRenderer::Restart(){

	TRACE_STREAM((L"StreamRenderer::Restart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State != StreamPaused ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));
	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	m_State = StreamStarted;

	return hr;
}

HRESULT CStreamVideoRenderer::RequestSample(){

	TRACE_STREAM((L"StreamRenderer::RequestSample"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	return hr;
}

HRESULT CStreamVideoRenderer::Preroll(){

	TRACE_STREAM((L"StreamRenderer::Restart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPrerolled, GUID_NULL, hr, NULL));

	return hr;
}