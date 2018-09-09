//----------------------------------------------------------------------------------------------
// SinkImageWriter.cpp
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

CSkinkImageWriter::CSkinkImageWriter(HRESULT& hr)
	: m_nRefCount(1),
	m_IsShutdown(FALSE),
	m_pStreamImage(NULL),
	m_pPresentationClock(NULL),
	m_pClock(NULL),
	m_ClockState(MFCLOCK_STATE_INVALID)
{
	TRACE_SINK((L"ImageSink::CTOR"));

	CStreamImage* pStream = NULL;
	hr = CStreamImage::CreateInstance(this, &pStream, hr);

	if(SUCCEEDED(hr)){

		m_pStreamImage = pStream;
		m_pStreamImage->AddRef();
	}
	else{
		m_IsShutdown = TRUE;
	}

	SAFE_RELEASE(pStream);
}

CSkinkImageWriter::~CSkinkImageWriter(){

	TRACE_SINK((L"ImageSink::DTOR"));
	Shutdown();
}

HRESULT CSkinkImageWriter::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_SINK((L"ImageSink::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CSkinkImageWriter* pMFSk = new (std::nothrow)CSkinkImageWriter(hr);

	IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
	IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

	LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFSk);

	return hr;
}

HRESULT CSkinkImageWriter::QueryInterface(REFIID riid, void** ppv){

	TRACE_SINK((L"ImageSink::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CSkinkImageWriter, IMFMediaSink),
			QITABENT(CSkinkImageWriter, IMFClockStateSink),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CSkinkImageWriter::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ImageSink::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CSkinkImageWriter::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ImageSink::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}