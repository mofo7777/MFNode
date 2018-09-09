//----------------------------------------------------------------------------------------------
// JpegStream.cpp
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

HRESULT CJpegStream::CreateInstance(CJpegStream** ppStream){

		CJpegStream* pStream = new (std::nothrow)CJpegStream;

		if(pStream == NULL)
				return E_OUTOFMEMORY;

		*ppStream = pStream;

		return S_OK;
}

HRESULT CJpegStream::QueryInterface(REFIID riid, void** ppv){

		static const QITAB qit[] = { QITABENT(CJpegStream, IStream), QITABENT(CJpegStream, ISequentialStream), {0} };

		return QISearch(this, qit, riid, ppv);
}

ULONG CJpegStream::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		return lRef;
}

ULONG CJpegStream::Release(){

		ULONG uCount = InterlockedDecrement(&m_nRefCount);
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CJpegStream::Seek(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/, ULARGE_INTEGER *plibNewPosition){

		// dwOrigin STREAM_SEEK_SET 0
		// dwOrigin STREAM_SEEK_CUR 1
		// dwOrigin STREAM_SEEK_END 2

		// The call of Seek is always 0.
		m_JpegBuffer.Reset();
		plibNewPosition->QuadPart = 0;

		return S_OK;
}

HRESULT CJpegStream::SetSize(ULARGE_INTEGER libNewSize){

		HRESULT hr;
		IF_FAILED_RETURN(hr = m_JpegBuffer.Reserve(libNewSize.LowPart));
		return hr;
}

HRESULT CJpegStream::Write(void const *pv, ULONG cb, ULONG *pcbWritten){

		HRESULT hr;

		IF_FAILED_RETURN(hr = m_JpegBuffer.Reserve(cb));

		memcpy(m_JpegBuffer.GetReadStartBuffer(), pv, cb);

		IF_FAILED_RETURN(hr = m_JpegBuffer.SetEndPosition(cb));

		*pcbWritten = cb;

		return hr;
}