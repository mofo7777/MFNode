//----------------------------------------------------------------------------------------------
// StreamDxva2Renderer.cpp
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

CStreamDxva2Renderer::CStreamDxva2Renderer(CSinkDxva2Renderer* pSinkDxva2Renderer, HRESULT& hr)
		: m_nRefCount(1),
		  m_State(StreamTypeNotSet),
				m_pEventQueue(NULL),
				m_pMediaType(NULL),
				m_pSinkDxva2Renderer(NULL)
{

		TRACE_STREAM((L"StreamRenderer::CTOR"));

		LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));

		m_pSinkDxva2Renderer = pSinkDxva2Renderer;
		m_pSinkDxva2Renderer->AddRef();
}

CStreamDxva2Renderer::~CStreamDxva2Renderer(){
		
		TRACE_STREAM((L"StreamRenderer::DTOR"));

		Shutdown();
}

HRESULT CStreamDxva2Renderer::CreateInstance(CSinkDxva2Renderer* pSink, CStreamDxva2Renderer** ppStream, HRESULT& hr){

		TRACE_SINK((L"StreamRenderer::CreateInstance"));

		IF_FAILED_RETURN(hr = (ppStream == NULL ? E_INVALIDARG : S_OK));

		CStreamDxva2Renderer* pStream = new (std::nothrow)CStreamDxva2Renderer(pSink, hr);
		
		IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

		*ppStream = pStream;
		(*ppStream)->AddRef();

		SAFE_RELEASE(pStream);
		
		return hr;
}

HRESULT CStreamDxva2Renderer::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_STREAM((L"StreamRenderer::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CStreamDxva2Renderer, IMFStreamSink),
				QITABENT(CStreamDxva2Renderer, IMFMediaEventGenerator),
				QITABENT(CStreamDxva2Renderer, IMFMediaTypeHandler),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CStreamDxva2Renderer::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"StreamRenderer::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CStreamDxva2Renderer::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"StreamRenderer::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CStreamDxva2Renderer::Shutdown(){

		TRACE_STREAM((L"StreamRenderer::Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}
		
		SAFE_RELEASE(m_pMediaType);
		SAFE_RELEASE(m_pSinkDxva2Renderer);
		SAFE_RELEASE(m_pEventQueue);

		m_State = StreamFinalized;

		return S_OK;
}

HRESULT CStreamDxva2Renderer::Start(MFTIME){

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

HRESULT CStreamDxva2Renderer::Stop(){

		TRACE_STREAM((L"StreamRenderer::Stop"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL));

		m_State = StreamStopped;

		return hr;
}

HRESULT CStreamDxva2Renderer::Pause(){

		TRACE_STREAM((L"StreamRenderer::Pause"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized || m_State == StreamStopped ? MF_E_INVALIDREQUEST : S_OK));

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL));

		m_State = StreamPaused;

		return hr;
}

HRESULT CStreamDxva2Renderer::Restart(){

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

HRESULT CStreamDxva2Renderer::RequestSample(){

		TRACE_STREAM((L"StreamRenderer::RequestSample"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

		return hr;
}

HRESULT CStreamDxva2Renderer::Preroll(){

		TRACE_STREAM((L"StreamRenderer::Restart"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPrerolled, GUID_NULL, hr, NULL));

		return hr;
}