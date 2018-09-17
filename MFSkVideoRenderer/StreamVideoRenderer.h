//----------------------------------------------------------------------------------------------
// StreamVideoRenderer.h
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
#ifndef STREAMVIDEORENDERER_H
#define STREAMVIDEORENDERER_H

class CStreamVideoRenderer : BaseObject, public IMFStreamSink, public IMFMediaTypeHandler{

public:

	// StreamVideoRenderer.cpp
	static HRESULT CreateInstance(CSinkVideoRenderer*, CStreamVideoRenderer**, HRESULT&);

	// IUnknown - StreamVideoRenderer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFStreamSink - StreamVideoRenderer_Sink.cpp
	STDMETHODIMP GetMediaSink(IMFMediaSink**);
	STDMETHODIMP GetIdentifier(DWORD*);
	STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler**);
	STDMETHODIMP ProcessSample(IMFSample*);
	STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE, const PROPVARIANT*, const PROPVARIANT*);
	STDMETHODIMP Flush();

	// IMFMediaEventGenerator - StreamVideoRenderer_Event.cpp
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaTypeHandler - StreamVideoRenderer_Type.cpp
	STDMETHODIMP IsMediaTypeSupported(IMFMediaType*, IMFMediaType**);
	STDMETHODIMP GetMediaTypeCount(DWORD*);
	STDMETHODIMP GetMediaTypeByIndex(DWORD, IMFMediaType**);
	STDMETHODIMP SetCurrentMediaType(IMFMediaType*);
	STDMETHODIMP GetCurrentMediaType(IMFMediaType**);
	STDMETHODIMP GetMajorType(GUID*);

	// StreamVideoRenderer.cpp
	HRESULT Start(MFTIME);
	HRESULT Stop();
	HRESULT Pause();
	HRESULT Restart();
	HRESULT Shutdown();
	HRESULT RequestSample();
	HRESULT Preroll();

private:

	// StreamVideoRenderer.cpp
	CStreamVideoRenderer(CSinkVideoRenderer*, HRESULT&);
	virtual ~CStreamVideoRenderer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	StreamState m_State;

	IMFMediaEventQueue* m_pEventQueue;
	IMFMediaType* m_pMediaType;
	CSinkVideoRenderer* m_pSinkVideoRenderer;

	// Inline
	HRESULT CheckShutdown() const{ return (m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK); }
};

#endif