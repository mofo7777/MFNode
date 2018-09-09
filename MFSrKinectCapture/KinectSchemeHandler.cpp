//----------------------------------------------------------------------------------------------
// KinectSchemeHandler.cpp
// Copyright (C) 2012 Dumonteil David
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

HRESULT CKinectSchemeHandler::CreateInstance(IUnknown *pUnkOuter, REFIID iid, void** ppv){
		
		//TRACE((L"CKinectSchemeHandler CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));

		// This object does not support aggregation.
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CKinectSchemeHandler* pHandler = new (std::nothrow)CKinectSchemeHandler();

		IF_FAILED_RETURN(pHandler == NULL ? E_OUTOFMEMORY : S_OK);

		LOG_HRESULT(hr = pHandler->QueryInterface(iid, ppv));

		SAFE_RELEASE(pHandler);
		
		return hr;
}

HRESULT CKinectSchemeHandler::QueryInterface(REFIID riid, void** ppv){
		
		static const QITAB qit[] = {
				QITABENT(CKinectSchemeHandler, IMFSchemeHandler),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

HRESULT CKinectSchemeHandler::BeginCreateObject(LPCWSTR pwszURL, DWORD dwFlags, IPropertyStore*, IUnknown** ppIUnknownCancelCookie,
		                                              IMFAsyncCallback* pCallback, IUnknown* punkState){
				
		//TRACE((L"CKinectSchemeHandler BeginCreateObject"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pwszURL == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pCallback == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(hr = ((dwFlags & MF_RESOLUTION_MEDIASOURCE) == 0 ? E_INVALIDARG : S_OK));

		// Todo : pwszURL must be : "kinect:"

		if(ppIUnknownCancelCookie){
				// We don't return a cancellation cookie.
				*ppIUnknownCancelCookie = NULL;
		}

		IMFAsyncResult* pResult = NULL;
		CKinectSource* pSource = NULL;

		try{

				// Create an instance of the media source.
				IF_FAILED_THROW(hr = CKinectSource::CreateInstance(&pSource));

    IF_FAILED_THROW(hr = pSource->LoadKinect(pwszURL));

    // Create a result object for the caller's async callback.
    IF_FAILED_THROW(hr = MFCreateAsyncResult(pSource, pCallback, punkState, &pResult));

				IF_FAILED_THROW(hr = MFInvokeCallback(pResult));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pResult);
		SAFE_RELEASE(pSource);

		return hr;
}
    
HRESULT CKinectSchemeHandler::EndCreateObject(IMFAsyncResult* pResult, MF_OBJECT_TYPE* pObjectType, IUnknown** ppObject){
		
		//TRACE((L"CKinectSchemeHandler EndCreateObject"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pResult == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pObjectType == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (ppObject == NULL ? E_POINTER : S_OK));

		IMFMediaSource* pSource = NULL;
		IUnknown* pUnk = NULL;

		try{

				IF_FAILED_THROW(hr = pResult->GetObject(&pUnk));

				// Minimal sanity check - is it really a media source?
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