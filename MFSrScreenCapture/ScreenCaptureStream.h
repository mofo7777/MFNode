//----------------------------------------------------------------------------------------------
// ScreenCaptureStream.h
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
#ifndef SCREENCAPTURESTREAM_H
#define SCREENCAPTURESTREAM_H

class CScreenCapture;

class CScreenCaptureStream : public IMFMediaStream{

public:

	static HRESULT CreateInstance(CScreenCaptureStream**, CScreenCapture*, IMFStreamDescriptor*, const DWORD, HRESULT&);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaStream
	STDMETHODIMP GetMediaSource(IMFMediaSource**);
	STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor**);
	STDMETHODIMP RequestSample(IUnknown*);

	// CKinectStream
	HRESULT Shutdown();
	HRESULT Flush(){ return S_OK; }

private:

	CScreenCaptureStream(CScreenCapture*, IMFStreamDescriptor*, const DWORD, HRESULT&);
	virtual ~CScreenCaptureStream(){ TRACE_STREAMSOURCE((L"Stream::DTOR")); Shutdown(); }

	CScreenCapture* m_pScreenCapture;

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	SourceState m_State;

	LONGLONG m_rtCurrentPosition;

	IMFMediaEventQueue* m_pEventQueue;
	IMFStreamDescriptor* m_pStreamDescriptor;

	IMFMediaBuffer* m_pMediaBuffer;
	BYTE* m_pVideoBuffer;
	DWORD m_dwVideoSize;

#ifdef REVERSE_GDI_IMAGE
	IMFMediaBuffer* m_pMediaGdiBuffer;
	BYTE* m_pVideoGdiBuffer;
#endif

	HRESULT CreateScreenVideoSample(IMFSample**);
	HRESULT CaptureGDIScreen();

	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif
