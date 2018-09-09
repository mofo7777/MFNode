//----------------------------------------------------------------------------------------------
// MFSkJpegHttpStreamer.cpp
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

CMFSkJpegHttpStreamer::CMFSkJpegHttpStreamer(HRESULT& hr)
		: m_nRefCount(1),
		  m_IsShutdown(FALSE),
		  m_pJpegStream(NULL),
		  m_pClock(NULL)
{
		TRACE_SINK((L"JpegSink::CTOR"));

		CMFSkStreamJpeg* pStream = NULL;
		hr = CMFSkStreamJpeg::CreateInstance(this, &pStream, hr);

		if(SUCCEEDED(hr)){

				m_pJpegStream = pStream;
				m_pJpegStream->AddRef();
		}
		else{
				m_IsShutdown = TRUE;
		}

		SAFE_RELEASE(pStream);
}

CMFSkJpegHttpStreamer::~CMFSkJpegHttpStreamer(){
		
		TRACE_SINK((L"JpegSink::DTOR"));
		Shutdown();
		//SAFE_RELEASE(m_pJpegStream);
		//SAFE_RELEASE(m_pClock);
}

HRESULT CMFSkJpegHttpStreamer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		TRACE_SINK((L"JpegSink::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CMFSkJpegHttpStreamer* pMFSk = new (std::nothrow)CMFSkJpegHttpStreamer(hr);

		IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
		IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

		LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

		SAFE_RELEASE(pMFSk);

		return hr;
}

HRESULT CMFSkJpegHttpStreamer::QueryInterface(REFIID riid, void** ppv){

		TRACE_SINK((L"JpegSink::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CMFSkJpegHttpStreamer, IMFMediaSink),
				QITABENT(CMFSkJpegHttpStreamer, IMFClockStateSink),
				{0}
		};

		return QISearch(this, qit, riid, ppv);
}

ULONG CMFSkJpegHttpStreamer::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"JpegSink::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFSkJpegHttpStreamer::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"JpegSink::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}