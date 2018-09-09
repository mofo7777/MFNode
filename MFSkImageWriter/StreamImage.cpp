//----------------------------------------------------------------------------------------------
// StreamImage.cpp
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

CStreamImage::CStreamImage(IMFMediaSink* pSink, HRESULT& hr)
	: m_nRefCount(1),
	m_State(StreamTypeNotSet),
	m_pEventQueue(NULL),
	m_pMediaType(NULL),
	m_pSink(NULL),
	m_uiWidth(0),
	m_uiHeight(0)
{

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));

	m_pSink = pSink;
	m_pSink->AddRef();
}

CStreamImage::~CStreamImage(){

	TRACE_STREAM((L"ImageStream::DTOR"));

	Shutdown();
}

HRESULT CStreamImage::CreateInstance(CSkinkImageWriter* pSink, CStreamImage** ppStream, HRESULT& hr){

	TRACE_SINK((L"ImageStream::CreateInstance"));

	IF_FAILED_RETURN(hr = (ppStream == NULL ? E_INVALIDARG : S_OK));

	CStreamImage* pStream = new (std::nothrow)CStreamImage(pSink, hr);

	IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

	*ppStream = pStream;
	(*ppStream)->AddRef();

	SAFE_RELEASE(pStream);

	return hr;
}

HRESULT CStreamImage::QueryInterface(REFIID riid, void** ppv){

	TRACE_STREAM((L"ImageStream::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CStreamImage, IMFStreamSink),
			QITABENT(CStreamImage, IMFMediaEventGenerator),
			QITABENT(CStreamImage, IMFMediaTypeHandler),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CStreamImage::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ImageStream::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CStreamImage::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ImageStream::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CStreamImage::Shutdown(){

	TRACE_STREAM((L"ImageStream::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pMediaType);
	SAFE_RELEASE(m_pSink);
	SAFE_RELEASE(m_pEventQueue);

	m_State = StreamFinalized;

	return S_OK;
}

HRESULT CStreamImage::Start(MFTIME){

	TRACE_STREAM((L"ImageStream::Start"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	if(m_State == StreamStarted){

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));
		return hr;
	}

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));
	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	m_State = StreamStarted;

	return hr;
}

HRESULT CStreamImage::Stop(){

	TRACE_STREAM((L"ImageStream::Stop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStopped, GUID_NULL, hr, NULL));

	m_State = StreamStopped;

	return hr;
}

HRESULT CStreamImage::Pause(){

	TRACE_STREAM((L"ImageStream::Pause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State == StreamTypeNotSet || m_State == StreamFinalized || m_State == StreamStopped ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkPaused, GUID_NULL, hr, NULL));

	m_State = StreamPaused;

	return hr;
}

HRESULT CStreamImage::Restart(){

	TRACE_STREAM((L"ImageStream::Restart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State != StreamPaused ? MF_E_INVALIDREQUEST : S_OK));

	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkStarted, GUID_NULL, hr, NULL));
	IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

	m_State = StreamStarted;

	return hr;
}