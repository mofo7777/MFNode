//----------------------------------------------------------------------------------------------
// MFByteStream.h
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
#ifndef MFBYTESTREAM_H
#define MFBYTESTREAM_H

class CMFByteStream : public IMFAsyncCallback{

public:

	static HRESULT CreateInstance(CMFByteStream**);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFAsyncCallback
	STDMETHODIMP GetParameters(DWORD*, DWORD*){ TRACE_BYTESTREAM((L"MFByteStream::GetParameters")); return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult*);

	HRESULT Initialize(LPCWSTR, DWORD*);
	HRESULT Start();
	void Close();
	HRESULT CMFByteStream::Read(BYTE*, const DWORD, DWORD*);
	HRESULT BeginRead(BYTE*, ULONG, IMFAsyncCallback*);
	HRESULT EndRead(IMFAsyncResult*, ULONG*);
	HRESULT Seek(const LONGLONG);
	HRESULT SeekFile(LARGE_INTEGER);
	HRESULT Reset();
	const BOOL IsInitialized() const{ return (m_hFile != INVALID_HANDLE_VALUE && m_pReadParam != NULL); }

private:

	CMFByteStream();
	virtual ~CMFByteStream(){ TRACE_BYTESTREAM((L"MFByteStream::DTOR")); Close(); }

	CriticSection m_CriticSection;

	volatile long m_nRefCount;

	wstring m_wszFile;
	HANDLE m_hFile;
	LARGE_INTEGER m_liFileSize;

	CMFReadParam* m_pReadParam;

	HRESULT Read(CMFReadParam*);
};

#endif