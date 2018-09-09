//----------------------------------------------------------------------------------------------
// MFByteStream.cpp
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

CMFByteStream::CMFByteStream()
		: m_nRefCount(1),
		  m_wszFile(L""),
				m_hFile(INVALID_HANDLE_VALUE),
				m_pReadParam(NULL)
{
		TRACE_BYTESTREAM((L"MFByteStream::CTOR"));
		m_liFileSize.HighPart = 0;
		m_liFileSize.LowPart = 0;
}

HRESULT CMFByteStream::CreateInstance(CMFByteStream** ppByteStream){
		
		TRACE_BYTESTREAM((L"MFByteStream::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppByteStream == NULL ? E_POINTER : S_OK));

		CMFByteStream* pByteStream = new (std::nothrow)CMFByteStream();

		IF_FAILED_RETURN(pByteStream == NULL ? E_OUTOFMEMORY : S_OK);

		if(SUCCEEDED(hr)){
				
				*ppByteStream = pByteStream;
				(*ppByteStream)->AddRef();
		}

		SAFE_RELEASE(pByteStream);
		
		return hr;
}

HRESULT CMFByteStream::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_BYTESTREAM((L"MFByteStream::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CMFByteStream, IMFAsyncCallback),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CMFByteStream::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"MFByteStream::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFByteStream::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"MFByteStream::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CMFByteStream::Invoke(IMFAsyncResult* pResult){

		TRACE_BYTESTREAM((L"MFByteStream::Invoke"));

		HRESULT hr = S_OK;

		IUnknown* pState = NULL;
		IUnknown* pUnk = NULL;
		IMFAsyncResult* pCallerResult = NULL;

		try{

    IF_FAILED_THROW(hr = pResult->GetState(&pState));

    IF_FAILED_THROW(hr = pState->QueryInterface(IID_PPV_ARGS(&pCallerResult)));

    IF_FAILED_THROW(hr = pCallerResult->GetObject(&pUnk));

    CMFReadParam* pReadParam = static_cast<CMFReadParam*>(pUnk);

    IF_FAILED_THROW(hr = Read(pReadParam));
		}
		catch(HRESULT){}

		if(pCallerResult){
				
				LOG_HRESULT(pCallerResult->SetStatus(hr));
				LOG_HRESULT(MFInvokeCallback(pCallerResult));
		}

		SAFE_RELEASE(pState);
		SAFE_RELEASE(pUnk);
		SAFE_RELEASE(pCallerResult);
		
		return hr;
}

HRESULT CMFByteStream::Initialize(LPCWSTR pwszFile, DWORD* pdwFlags){

		TRACE_BYTESTREAM((L"MFByteStream::Initialize"));

		HRESULT hr;
	
		IF_FAILED_RETURN(hr = (pwszFile == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pdwFlags == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile != INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		m_pReadParam = new (std::nothrow)CMFReadParam();

		IF_FAILED_RETURN(hr = (m_pReadParam == NULL ? E_OUTOFMEMORY : S_OK));

		if(!PathFileExists(pwszFile)){

				// With TopoEdit, we need to do this...
				m_wszFile = pwszFile + 8;

		  size_t pos = 0;
		
		  while((pos = m_wszFile.find(L"%20", pos)) != std::wstring::npos){
				
				  m_wszFile.replace(pos, 3, L" ");
				  pos++;
		  }
		}
		else{
				m_wszFile = pwszFile;
		}

		m_hFile = CreateFile(m_wszFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		if(!GetFileSizeEx(m_hFile, &m_liFileSize)){

				m_wszFile = L"";
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				IF_FAILED_RETURN(hr = E_UNEXPECTED);
		}

		*pdwFlags = MFBYTESTREAM_IS_SEEKABLE | MFBYTESTREAM_IS_READABLE;

		return hr;
}

HRESULT CMFByteStream::Start(){

		TRACE_BYTESTREAM((L"MFByteStream::Start"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile != INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));
		IF_FAILED_RETURN(hr = (m_pReadParam != NULL ? E_UNEXPECTED : S_OK));

		m_pReadParam = new (std::nothrow)CMFReadParam();

		IF_FAILED_RETURN(hr = (m_pReadParam == NULL ? E_OUTOFMEMORY : S_OK));

		m_hFile = CreateFile(m_wszFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		if(!GetFileSizeEx(m_hFile, &m_liFileSize)){

				m_wszFile = L"";
				CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
				SAFE_RELEASE(m_pReadParam);
				IF_FAILED_RETURN(hr = E_UNEXPECTED);
		}

		return hr;
}

void CMFByteStream::Close(){

		TRACE_BYTESTREAM((L"MFByteStream::Close"));

		AutoLock lock(m_CriticSection);

		SAFE_RELEASE(m_pReadParam);

		if(m_hFile == INVALID_HANDLE_VALUE)
				return;

		//m_wszFile = L"";

		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;

		m_liFileSize.HighPart = 0;
		m_liFileSize.LowPart = 0;
}

HRESULT CMFByteStream::Read(CMFReadParam* pReadParam){

		TRACE_BYTESTREAM((L"MFByteStream::Read"));

		HRESULT hr;

		if(ReadFile(m_hFile, pReadParam->GetDataPtr(), pReadParam->GetByteToRead(), pReadParam->GetpByteRead(), 0)){
				hr = S_OK;
		}
		else{
				hr = E_FAIL;
		}

		return hr;
}

HRESULT CMFByteStream::BeginRead(BYTE* pData, ULONG ulToRead, IMFAsyncCallback* pCallback){

		TRACE_BYTESTREAM((L"MFByteStream::BeginRead"));

		HRESULT hr;

		// ToDo : Check BeginRead parameters and inside variables

		AutoLock lock(m_CriticSection);

		//CMFReadParam* pReadParam = new (std::nothrow)CMFReadParam(pData, ulToRead);

		//IF_FAILED_RETURN(hr = (pReadParam == NULL ? E_OUTOFMEMORY : S_OK));

		m_pReadParam->SetDataPtr(pData);
		m_pReadParam->SetByteToRead(ulToRead);

		IMFAsyncResult* pResult = NULL;

		// Do MFCreateAsyncResult Release pReadParam if Failed ?
		IF_FAILED_RETURN(hr = MFCreateAsyncResult(m_pReadParam, pCallback, NULL, &pResult));

		LOG_HRESULT(hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, this, pResult));

		pResult->Release();

		return hr;
}

HRESULT CMFByteStream::EndRead(IMFAsyncResult* pResult, ULONG* pulRead){

		TRACE_BYTESTREAM((L"MFByteStream::EndRead"));

		HRESULT hr = S_OK;
		*pulRead = 0;
		IUnknown* pUnk = NULL;
		
		// ToDo : Check EndRead parameters and inside variables

		AutoLock lock(m_CriticSection);

		try{

				IF_FAILED_THROW(hr = pResult->GetStatus());

    IF_FAILED_THROW(hr = pResult->GetObject(&pUnk));

				CMFReadParam* pReadParam = static_cast<CMFReadParam*>(pUnk);
				
				*pulRead = pReadParam->GetByteRead();
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pUnk);

		return hr;
}

HRESULT CMFByteStream::Seek(const LONG lDistance){

		TRACE_BYTESTREAM((L"MFByteStream::Seek"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		DWORD dwPosition = SetFilePointer(m_hFile, lDistance, NULL, FILE_CURRENT);

		IF_FAILED_RETURN(hr = (dwPosition == INVALID_SET_FILE_POINTER ? E_FAIL : S_OK));

		return hr;
}

HRESULT CMFByteStream::SeekEnd(const LONG lDistance){

		TRACE_BYTESTREAM((L"MFByteStream::SeekEnd"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		DWORD dwPosition = SetFilePointer(m_hFile, -lDistance, NULL, FILE_END);

		IF_FAILED_RETURN(hr = (dwPosition == INVALID_SET_FILE_POINTER ? E_FAIL : S_OK));

		return hr;
}

HRESULT CMFByteStream::SeekFile(LARGE_INTEGER liDistance){

		TRACE_BYTESTREAM((L"MFByteStream::SeekFile"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		DWORD dwPosition = SetFilePointer(m_hFile, liDistance.LowPart, &liDistance.HighPart, FILE_BEGIN);

		IF_FAILED_RETURN(hr = (dwPosition == INVALID_SET_FILE_POINTER || GetLastError() != NO_ERROR ? E_FAIL : S_OK));

		return hr;
}

HRESULT CMFByteStream::Reset(){

		TRACE_BYTESTREAM((L"MFByteStream::Reset"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_hFile == INVALID_HANDLE_VALUE ? E_UNEXPECTED : S_OK));

		DWORD dwPosition = SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);

		IF_FAILED_RETURN(hr = (dwPosition == INVALID_SET_FILE_POINTER ? E_FAIL : S_OK));

		return hr;
}