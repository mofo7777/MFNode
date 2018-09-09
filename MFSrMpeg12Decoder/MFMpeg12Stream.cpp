//----------------------------------------------------------------------------------------------
// MFMpeg12Stream.cpp
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

CMFMpeg12Stream::CMFMpeg12Stream(CMFMpeg12Source* pSource, IMFStreamDescriptor* pSD, HRESULT& hr)
	: m_nRefCount(1),
	m_pEventQueue(NULL),
	m_State(StreamStopped),
	m_bEOS(FALSE)
{
	TRACE_STREAM((L"Stream::CTOR"));

	assert(pSource != NULL);
	assert(pSD != NULL);

	m_pSource = pSource;
	m_pSource->AddRef();

	m_pStreamDescriptor = pSD;
	m_pStreamDescriptor->AddRef();

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
}

HRESULT CMFMpeg12Stream::CreateInstance(CMFMpeg12Stream** ppStream, CMFMpeg12Source* pSource, IMFStreamDescriptor* pSD, HRESULT& hr){

	TRACE_STREAM((L"Stream::CreateInstance"));

	IF_FAILED_RETURN(hr = (ppStream == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pSource == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pSD == NULL ? E_POINTER : S_OK));

	CMFMpeg12Stream* pStream = new (std::nothrow)CMFMpeg12Stream(pSource, pSD, hr);

	IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

	*ppStream = pStream;
	(*ppStream)->AddRef();

	SAFE_RELEASE(pStream);

	return hr;
}

HRESULT CMFMpeg12Stream::QueryInterface(REFIID riid, void** ppv){

	TRACE_STREAM((L"Stream::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CMFMpeg12Stream, IMFMediaEventGenerator),
			QITABENT(CMFMpeg12Stream, IMFMediaStream),
			{ 0 }
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMFMpeg12Stream::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Stream::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFMpeg12Stream::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Stream::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMFMpeg12Stream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

	TRACE_STREAM((L"Stream::BeginGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

	return hr;
}

HRESULT CMFMpeg12Stream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){

	TRACE_STREAM((L"Stream::EndGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

	return hr;
}

HRESULT CMFMpeg12Stream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

	TRACE_STREAM((L"Stream::GetEvent"));

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

	if(SUCCEEDED(hr)){
		LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));
	}

	SAFE_RELEASE(pQueue);

	return hr;
}

HRESULT CMFMpeg12Stream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

	TRACE_STREAM((L"Stream::QueueEvent : %s", MFEventString(met)));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

	return hr;
}

HRESULT CMFMpeg12Stream::GetMediaSource(IMFMediaSource** ppMediaSource){

	TRACE_STREAM((L"Stream::GetMediaSource"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppMediaSource == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	// If called after shutdown, then m_pSource is NULL. Otherwise, m_pSource should not be NULL.
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pSource == NULL ? E_UNEXPECTED : S_OK));

	LOG_HRESULT(hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppMediaSource)));

	return hr;
}

HRESULT CMFMpeg12Stream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor){

	TRACE_STREAM((L"Stream::GetStreamDescriptor"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamDescriptor == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pStreamDescriptor == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamDescriptor = m_pStreamDescriptor;
	(*ppStreamDescriptor)->AddRef();

	return hr;
}

HRESULT CMFMpeg12Stream::RequestSample(IUnknown* pToken){

	TRACE_STREAM((L"Stream::RequestSample"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pSource == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pSource->GetState() == SourceStopped ? MF_E_INVALIDREQUEST : S_OK));

	if(m_pSource->GetState() == SourcePaused){
		return S_OK;
	}

	if(m_bEOS){
		IF_FAILED_RETURN(hr = MF_E_END_OF_STREAM);
	}

	IMFSample* pSample = NULL;

	try{

		IF_FAILED_THROW(hr = m_pSource->GetSample(&pSample));

		if(pSample != NULL){

			if(pToken){
				IF_FAILED_THROW(hr = pSample->SetUnknown(MFSampleExtension_Token, pToken));
			}

			IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, hr, pSample));
		}
	}
	catch(HRESULT){}

	if(SUCCEEDED(hr) && m_bEOS){

		// ?? check this see EndOfStream
		//LOG_HRESULT(hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL));
		LOG_HRESULT(hr = m_pSource->QueueEvent(MEEndOfPresentation, GUID_NULL, S_OK, NULL));
	}

	SAFE_RELEASE(pSample);

	return hr;
}

HRESULT CMFMpeg12Stream::Shutdown(){

	TRACE_STREAM((L"Stream::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pEventQueue);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pStreamDescriptor);

	m_State = StreamFinalized;
	m_bEOS = FALSE;

	return S_OK;
}