//----------------------------------------------------------------------------------------------
// MFMpeg2Source.cpp
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

CMFMpeg2Source::CMFMpeg2Source(HRESULT& hr)
	: m_nRefCount(1),
	CMFOperationQueue(m_CriticSection),
	m_pEventQueue(NULL),
	m_pPresentationDescriptor(NULL),
	m_pBeginOpenResult(NULL),
	m_pByteStream(NULL),
	m_State(SourceInvalid),
	m_pCurrentOp(NULL),
	m_pSampleRequest(NULL),
	m_cRestartCounter(0),
	m_cPendingEOS(0),
	m_OnByteStreamRead(this, &CMFMpeg2Source::OnByteStreamRead),
	m_hnsStart(0),
	m_bSkipInfo(TRUE)
{
	TRACE_SOURCE((L"Source::CTOR"));

	m_SysHeader.streams[0].type = StreamType_Unknown;
	m_SysHeader.streams[1].type = StreamType_Unknown;

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
}

CMFMpeg2Source::~CMFMpeg2Source(){

	TRACE_SOURCE((L"Source::DTOR"));
	if(m_State != SourceShutdown){
		Shutdown();
	}
}

HRESULT CMFMpeg2Source::CreateInstance(CMFMpeg2Source** ppSource){

	TRACE_SOURCE((L"Source::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));

	CMFMpeg2Source* pSource = new (std::nothrow)CMFMpeg2Source(hr);

	IF_FAILED_RETURN(pSource == NULL ? E_OUTOFMEMORY : S_OK);

	if(SUCCEEDED(hr)){

		*ppSource = pSource;
		(*ppSource)->AddRef();
	}

	SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CMFMpeg2Source::QueryInterface(REFIID riid, void** ppv){

	TRACE_SOURCE((L"Source::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CMFMpeg2Source, IMFMediaEventGenerator),
			QITABENT(CMFMpeg2Source, IMFMediaSource),
			{ 0 }
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMFMpeg2Source::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFMpeg2Source::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CMFMpeg2Source::BeginOpen(LPCWSTR pwszFile, IMFAsyncCallback* pCB, IUnknown* pState){

	TRACE_SOURCE((L"Source::BeginOpen"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pwszFile == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pCB == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (m_State != SourceInvalid ? MF_E_INVALIDREQUEST : S_OK));

	AutoLock lock(m_CriticSection);

	DWORD dwCaps = 0;

	IF_FAILED_RETURN(hr = CMFByteStream::CreateInstance(&m_pByteStream));
	IF_FAILED_RETURN(hr = m_pByteStream->Initialize(pwszFile, &dwCaps));

	if((dwCaps & MFBYTESTREAM_IS_SEEKABLE) == 0){
		IF_FAILED_RETURN(hr = MF_E_BYTESTREAM_NOT_SEEKABLE);
	}
	if((dwCaps & MFBYTESTREAM_IS_READABLE) == 0){
		IF_FAILED_RETURN(hr = E_FAIL);
	}

	IF_FAILED_RETURN(hr = m_cReadBuffer.Initialize());

	IF_FAILED_RETURN(hr = MFCreateAsyncResult(NULL, pCB, pState, &m_pBeginOpenResult));

	IF_FAILED_RETURN(hr = RequestData(READ_SIZE));

	m_State = SourceOpening;

	return hr;
}

HRESULT CMFMpeg2Source::EndOpen(IMFAsyncResult* pResult){

	TRACE_SOURCE((L"Source::EndOpen"));

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;

	hr = pResult->GetStatus();

	if(FAILED(hr)){

		Shutdown();
	}

	return hr;
}

HRESULT CMFMpeg2Source::OnByteStreamRead(IMFAsyncResult* pResult){

	TRACE_SOURCE((L"Source::OnByteStreamRead"));

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;

	if(m_State == SourceShutdown){
		return S_OK;
	}

	DWORD cbRead = 0;

	IUnknown* pState = NULL;

	try{

		IF_FAILED_THROW(hr = (pResult == NULL ? E_POINTER : S_OK));

		(void)pResult->GetState(&pState);

		IF_FAILED_THROW(hr = m_pByteStream->EndRead(pResult, &cbRead));

		if((pState == NULL) || (((CMFSourceOperation*)pState)->Data().ulVal == m_cRestartCounter)){

			if(cbRead == 0){

				IF_FAILED_THROW(hr = EndOfMPEGStream());
			}
			else{

				IF_FAILED_THROW(hr = m_cReadBuffer.SetEndPosition(cbRead));
				IF_FAILED_THROW(hr = ParseData());
			}
		}
	}
	catch(HRESULT){}

	if(FAILED(hr)){
		StreamingError(hr);
	}

	SAFE_RELEASE(pState);

	return hr;
}

HRESULT CMFMpeg2Source::QueueAsyncOperation(CMFSourceOperation::Operation OpType){

	HRESULT hr = S_OK;
	CMFSourceOperation* pOp = NULL;

	try{

		IF_FAILED_THROW(hr = CMFSourceOperation::CreateOp(OpType, &pOp));
		IF_FAILED_THROW(hr = QueueOperation(pOp));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOp);

	return hr;
}

HRESULT CMFMpeg2Source::CompleteOpen(HRESULT hrStatus){

	HRESULT hr = S_OK;

	m_pByteStream->Close();

	if(m_pBeginOpenResult){

		try{

			IF_FAILED_THROW(hr = m_pBeginOpenResult->SetStatus(hrStatus));
			IF_FAILED_THROW(hr = MFInvokeCallback(m_pBeginOpenResult));
		}
		catch(HRESULT){}

		SAFE_RELEASE(m_pBeginOpenResult);
	}

	return hr;
}