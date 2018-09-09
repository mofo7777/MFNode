//----------------------------------------------------------------------------------------------
// JpegStream.h
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
#ifndef JPEGSTREAM_H
#define JPEGSTREAM_H

class CJpegStream : public IStream{

  public:

				static HRESULT CreateInstance(CJpegStream**);

				// IUnknown
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

				// IStream
				STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*);
				STDMETHODIMP SetSize(ULARGE_INTEGER);
				STDMETHODIMP CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*){ return E_NOTIMPL; }
				STDMETHODIMP Commit(DWORD){ return E_NOTIMPL; }
				STDMETHODIMP Revert(){ return E_NOTIMPL; }
				STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD){ return E_NOTIMPL; }
				STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD){ return E_NOTIMPL; }
				STDMETHODIMP Stat(STATSTG*, DWORD){ return E_NOTIMPL; }
				STDMETHODIMP Clone(IStream**){ return E_NOTIMPL; }

				// IID_ISequentialStream
				STDMETHODIMP Read(void*, ULONG, ULONG*){ return E_NOTIMPL; }
				STDMETHODIMP Write(void const*, ULONG, ULONG*);

				BYTE* GetBufferAndSize(DWORD& dwSize){ dwSize = m_JpegBuffer.GetBufferSize(); return m_JpegBuffer.GetStartBuffer(); }
				void Initialize(const DWORD dwSize){ m_JpegBuffer.Initialize(dwSize); }

  private:

				CJpegStream() : m_nRefCount(1){}
				virtual ~CJpegStream(){}

				volatile long m_nRefCount;

				CMFBuffer m_JpegBuffer;
};

#endif