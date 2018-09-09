//----------------------------------------------------------------------------------------------
// MFMpeg2Stream.cpp
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

CMFMpeg2Stream::CMFMpeg2Stream(CMFMpeg2Source* pSource, IMFStreamDescriptor* pSD, HRESULT& hr)
		: m_nRefCount(1),
		  m_pEventQueue(NULL),
		  m_State(StreamStopped),
				m_bActive(FALSE),
				m_bEOS(FALSE){
						
  TRACE_STREAM((L"Stream::CTOR"));
		
		assert(pSource != NULL);
		assert(pSD != NULL);

		m_pSource = pSource;
		m_pSource->AddRef();

		m_pStreamDescriptor = pSD;
		m_pStreamDescriptor->AddRef();

		LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
}

HRESULT CMFMpeg2Stream::QueryInterface(REFIID riid, void** ppv){

		TRACE_SOURCE((L"Stream::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CMFMpeg2Stream, IMFMediaEventGenerator),
				QITABENT(CMFMpeg2Stream, IMFMediaStream),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CMFMpeg2Stream::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Stream::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFMpeg2Stream::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Stream::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CMFMpeg2Stream::Activate(BOOL bActive){
		
		TRACE_STREAM((L"Stream::Activate"));

		SourceLock lock(m_pSource);

		if(bActive == m_bActive){
				return S_OK;
		}

		m_bActive = bActive;

		if(!bActive){
				m_Samples.Clear();
				m_Requests.Clear();
		}
		
		return S_OK;
}

HRESULT CMFMpeg2Stream::Start(const PROPVARIANT& varStart){
		
		TRACE_STREAM((L"Stream::Start"));

		HRESULT hr;

		SourceLock lock(m_pSource);

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamStarted, GUID_NULL, S_OK, &varStart));

		m_State = StreamStarted;

		IF_FAILED_RETURN(hr = DispatchSamples());

		return hr;
}

HRESULT CMFMpeg2Stream::Pause(){
		
		TRACE_STREAM((L"Stream::Pause"));

		HRESULT hr;
		
		SourceLock lock(m_pSource);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_State = StreamPaused;

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));

		return hr;
}

HRESULT CMFMpeg2Stream::Stop(){
		
		TRACE_STREAM((L"Stream::Stop"));

		HRESULT hr;
		
		SourceLock lock(m_pSource);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_Requests.Clear();
		m_Samples.Clear();

		m_State = StreamStopped;

		IF_FAILED_RETURN(hr = QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));

		return hr;
}

HRESULT CMFMpeg2Stream::EndOfStream(){
		
		TRACE_STREAM((L"Stream::EndOfStream"));

		SourceLock lock(m_pSource);

		m_bEOS = TRUE;

		return DispatchSamples();
}

HRESULT CMFMpeg2Stream::Shutdown(){
		
		TRACE_STREAM((L"Stream::Shutdown"));

		HRESULT hr;
		
		SourceLock lock(m_pSource);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_State = StreamFinalized;

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		m_Samples.Clear();
		m_Requests.Clear();

		SAFE_RELEASE(m_pStreamDescriptor);
		SAFE_RELEASE(m_pEventQueue);

		return hr;
}

HRESULT CMFMpeg2Stream::DeliverPayload(IMFSample* pSample){

		TRACE_STREAM((L"Stream::DeliverPayload"));

		HRESULT hr;
		
		SourceLock lock(m_pSource);

		IF_FAILED_RETURN(hr = m_Samples.InsertBack(pSample));
		IF_FAILED_RETURN(hr = DispatchSamples());

		return hr;
}

HRESULT CMFMpeg2Stream::DispatchSamples(){

		TRACE_STREAM((L"Stream::DispatchSamples"));
		
		HRESULT hr = S_OK;

		SourceLock lock(m_pSource);

		if (m_State != StreamStarted){
				return hr;
		}

		IMFSample* pSample = NULL;
		IUnknown* pToken = NULL;

		try{
				
				while(!m_Samples.IsEmpty() && !m_Requests.IsEmpty()){

						IF_FAILED_THROW(hr = m_Samples.RemoveFront(&pSample));

						IF_FAILED_THROW(hr = m_Requests.RemoveFront(&pToken));

						if(pToken){
								IF_FAILED_THROW(hr = pSample->SetUnknown(MFSampleExtension_Token, pToken));
						}

						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, pSample));

						SAFE_RELEASE(pSample);
						SAFE_RELEASE(pToken);
				}

				if(m_Samples.IsEmpty() && m_bEOS){
						
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamVar(MEEndOfStream, GUID_NULL, S_OK, NULL));

						IF_FAILED_THROW(hr = m_pSource->QueueAsyncOperation(CMFSourceOperation::OP_END_OF_STREAM));
				}
				else if(NeedsData()){
						
						IF_FAILED_THROW(hr = m_pSource->QueueAsyncOperation(CMFSourceOperation::OP_REQUEST_DATA));
				}
		}
		catch(HRESULT){}

		if(FAILED(hr) && (m_State != StreamFinalized)){
				
				LOG_HRESULT(m_pSource->QueueEvent(MEError, GUID_NULL, hr, NULL));
		}

		SAFE_RELEASE(pSample);
		SAFE_RELEASE(pToken);
		
		// To see
		return S_OK;
}

CMFMpeg2Stream::SourceLock::SourceLock(CMFMpeg2Source* pSource) : m_pSource(NULL){

		if(pSource){

				m_pSource = pSource;
				m_pSource->AddRef();
				m_pSource->Lock();
		}
}

CMFMpeg2Stream::SourceLock::~SourceLock(){

		if(m_pSource){
				m_pSource->Unlock();
				m_pSource->Release();
		}
}