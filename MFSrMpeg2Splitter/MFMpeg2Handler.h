//----------------------------------------------------------------------------------------------
// MFMpeg2Handler.h
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
#ifndef MFMPEG2HANDLER_H
#define MFMPEG2HANDLER_H

class CMFMpeg2Handler : public IMFByteStreamHandler, public IMFAsyncCallback{

public:

	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFAsyncCallback
	STDMETHODIMP GetParameters(DWORD*, DWORD*){ return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult*);

	// IMFByteStreamHandler
	STDMETHODIMP BeginCreateObject(IMFByteStream*, LPCWSTR, DWORD, IPropertyStore*, IUnknown**, IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndCreateObject(IMFAsyncResult*, MF_OBJECT_TYPE*, IUnknown**);
	STDMETHODIMP CancelObjectCreation(IUnknown*){ return E_NOTIMPL; }
	STDMETHODIMP GetMaxNumberOfBytesRequiredForResolution(QWORD*){ return E_NOTIMPL; }

private:

	CMFMpeg2Handler() : m_nRefCount(1), m_pSource(NULL), m_pResult(NULL){ TRACE_HANDLER((L"ByteStream::CTOR")); }
	virtual ~CMFMpeg2Handler(){ TRACE_HANDLER((L"ByteStream::DTOR")); SAFE_RELEASE(m_pSource); SAFE_RELEASE(m_pResult); }

	volatile long m_nRefCount;

	CMFMpeg2Source* m_pSource;
	IMFAsyncResult* m_pResult;
};

#endif
