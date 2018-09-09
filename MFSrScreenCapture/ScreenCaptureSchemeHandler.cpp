//----------------------------------------------------------------------------------------------
// ScreenCaptureSchemeHandler.cpp
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

HRESULT CScreenCaptureSchemeHandler::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){
		
		TRACE_SCHEME((L"Scheme::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));

		// This object does not support aggregation.
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CScreenCaptureSchemeHandler* pHandler = new (std::nothrow)CScreenCaptureSchemeHandler();

		IF_FAILED_RETURN(pHandler == NULL ? E_OUTOFMEMORY : S_OK);

		LOG_HRESULT(hr = pHandler->QueryInterface(iid, ppv));

		SAFE_RELEASE(pHandler);
		
		return hr;
}

HRESULT CScreenCaptureSchemeHandler::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_SCHEME((L"Scheme::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CScreenCaptureSchemeHandler, IMFSchemeHandler),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CScreenCaptureSchemeHandler::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Scheme::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CScreenCaptureSchemeHandler::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Scheme::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CScreenCaptureSchemeHandler::BeginCreateObject(LPCWSTR pwszURL, DWORD dwFlags, IPropertyStore*, IUnknown** ppIUnknownCancelCookie,
		                                                     IMFAsyncCallback* pCallback, IUnknown* punkState){
				
		TRACE_SCHEME((L"Scheme::BeginCreateObject"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pwszURL == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pCallback == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(hr = ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0 ? E_INVALIDARG : S_OK));

		// Todo : pwszURL must be : "screen:"

		if(ppIUnknownCancelCookie){
				*ppIUnknownCancelCookie = NULL;
		}

		IMFAsyncResult* pResult = NULL;
		CScreenCapture* pSource = NULL;

		try{

				IF_FAILED_THROW(hr = CScreenCapture::CreateInstance(&pSource));

				pSource->Initialize();

    IF_FAILED_THROW(hr = MFCreateAsyncResult(pSource, pCallback, punkState, &pResult));

				IF_FAILED_THROW(hr = MFInvokeCallback(pResult));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pResult);
		SAFE_RELEASE(pSource);

		return hr;
}
    
HRESULT CScreenCaptureSchemeHandler::EndCreateObject(IMFAsyncResult* pResult, MF_OBJECT_TYPE* pObjectType, IUnknown** ppObject){
		
		TRACE_SCHEME((L"Scheme::EndCreateObject"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pResult == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pObjectType == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (ppObject == NULL ? E_POINTER : S_OK));

		IMFMediaSource* pSource = NULL;
		IUnknown* pUnk = NULL;

		try{

				IF_FAILED_THROW(hr = pResult->GetObject(&pUnk));

				IF_FAILED_THROW(hr = pUnk->QueryInterface(IID_PPV_ARGS(&pSource)));

				*ppObject = pUnk;
				(*ppObject)->AddRef();
				*pObjectType = MF_OBJECT_MEDIASOURCE;
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSource);
		SAFE_RELEASE(pUnk);

		return hr;
}