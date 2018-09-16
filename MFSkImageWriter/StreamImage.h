//----------------------------------------------------------------------------------------------
// StreamImage.h
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
#ifndef STREAMIMAGE_H
#define STREAMIMAGE_H

class CStreamImage : public IMFStreamSink, public IMFMediaTypeHandler{

public:

	// StreamImage.cpp
	static HRESULT CreateInstance(CSkinkImageWriter*, CStreamImage**, HRESULT&);

	// IUnknown - StreamImage.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFStreamSink - StreamImage_Sink.cpp
	STDMETHODIMP GetMediaSink(IMFMediaSink** ppMediaSink);
	STDMETHODIMP GetIdentifier(DWORD* pdwIdentifier);
	STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler);
	STDMETHODIMP ProcessSample(IMFSample* pSample);
	STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT* pvarMarkerValue, const PROPVARIANT* pvarContextValue);
	STDMETHODIMP Flush();

	// IMFMediaEventGenerator - StreamImage_Event.cpp
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaTypeHandler - StreamImage_Type.cpp
	STDMETHODIMP IsMediaTypeSupported(IMFMediaType*, IMFMediaType**);
	STDMETHODIMP GetMediaTypeCount(DWORD*);
	STDMETHODIMP GetMediaTypeByIndex(DWORD, IMFMediaType**);
	STDMETHODIMP SetCurrentMediaType(IMFMediaType*);
	STDMETHODIMP GetCurrentMediaType(IMFMediaType**);
	STDMETHODIMP GetMajorType(GUID*);

	// StreamImage.cpp
	HRESULT Start(MFTIME);
	HRESULT Stop();
	HRESULT Pause();
	HRESULT Restart();
	HRESULT Shutdown();

private:

	// StreamImage.cpp
	CStreamImage(IMFMediaSink*, HRESULT&);
	virtual ~CStreamImage();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	StreamState m_State;

	IMFMediaEventQueue* m_pEventQueue;
	IMFMediaType* m_pMediaType;
	IMFMediaSink* m_pSink;

	UINT32 m_uiWidth;
	UINT32 m_uiHeight;

	HRESULT CreateBmpFile(const BYTE*, const UINT32, const UINT32, const DWORD);

	// Inline
	HRESULT CheckShutdown() const{ return (m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK); }
};

#endif
