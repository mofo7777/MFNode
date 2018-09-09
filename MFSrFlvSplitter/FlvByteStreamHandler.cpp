//----------------------------------------------------------------------------------------------
// FlvByteStreamHandler.cpp
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

HRESULT CFlvByteStreamHandler::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_HANDLER((L"ByteStream::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));

	// This object does not support aggregation.
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CFlvByteStreamHandler* pHandler = new (std::nothrow)CFlvByteStreamHandler();

	IF_FAILED_RETURN(pHandler == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pHandler->QueryInterface(iid, ppv));

	SAFE_RELEASE(pHandler);

	return hr;
}

HRESULT CFlvByteStreamHandler::QueryInterface(REFIID riid, void** ppv){

	TRACE_HANDLER((L"ByteStream::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CFlvByteStreamHandler, IMFByteStreamHandler),
			QITABENT(CFlvByteStreamHandler, IMFAsyncCallback),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CFlvByteStreamHandler::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ByteStream::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CFlvByteStreamHandler::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"ByteStream::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CFlvByteStreamHandler::BeginCreateObject(IMFByteStream* /*pByteStream*/, LPCWSTR pwszURL, DWORD dwFlags, IPropertyStore* /*pProps*/,
	IUnknown** ppIUnknownCancelCookie, IMFAsyncCallback* pCallback, IUnknown* punkState){

	TRACE_HANDLER((L"ByteStream::BeginCreateObject"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pwszURL == NULL ? E_POINTER : S_OK));
	//IF_FAILED_RETURN(hr = (pByteStream == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pCallback == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0 ? E_INVALIDARG : S_OK));

	if(ppIUnknownCancelCookie){
		*ppIUnknownCancelCookie = NULL;
	}

	IMFAsyncResult* pResult = NULL;
	CFlvSource*   pSource = NULL;

	try{

		IF_FAILED_THROW(hr = CFlvSource::CreateInstance(&pSource));

		IF_FAILED_THROW(hr = MFCreateAsyncResult(NULL, pCallback, punkState, &pResult));

		IF_FAILED_THROW(hr = pSource->BeginOpen(pwszURL, this, NULL));

		m_pResult = pResult;
		m_pSource = pSource;

		m_pResult->AddRef();
		m_pSource->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pResult);
	SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CFlvByteStreamHandler::EndCreateObject(IMFAsyncResult* pResult, MF_OBJECT_TYPE* pObjectType, IUnknown** ppObject){

	TRACE_HANDLER((L"ByteStream::EndCreateObject"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pResult == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pObjectType == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (ppObject == NULL ? E_POINTER : S_OK));

	*pObjectType = MF_OBJECT_INVALID;
	*ppObject = NULL;

	try{

		IF_FAILED_THROW(hr = pResult->GetStatus());

		*pObjectType = MF_OBJECT_MEDIASOURCE;

		assert(m_pSource != NULL);

		IF_FAILED_THROW(hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppObject)));
	}
	catch(HRESULT){}

	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pResult);

	return hr;
}

HRESULT CFlvByteStreamHandler::Invoke(IMFAsyncResult* pResult){

	TRACE_HANDLER((L"ByteStream::Invoke"));

	HRESULT hr = S_OK;

	if(m_pSource){
		hr = m_pSource->EndOpen(pResult);
	}
	else{
		hr = E_UNEXPECTED;
	}

	m_pResult->SetStatus(hr);

	hr = MFInvokeCallback(m_pResult);

	return hr;
}