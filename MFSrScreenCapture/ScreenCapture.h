//----------------------------------------------------------------------------------------------
// ScreenCapture.h
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
#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

class CScreenCapture : public IMFMediaSource{

public:

	static HRESULT CreateInstance(CScreenCapture**);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaSource
	STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor**);
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP Pause();
	STDMETHODIMP Shutdown();
	STDMETHODIMP Start(IMFPresentationDescriptor*, const GUID*, const PROPVARIANT*);
	STDMETHODIMP Stop();

	HRESULT Initialize(){ AutoLock lock(m_CriticSection); m_State = SourceStopped; return S_OK; }
	SourceState GetState() const { return m_State; }

private:

	CScreenCapture(HRESULT&);
	virtual ~CScreenCapture(){ TRACE_SOURCE((L"Capture::DTOR")); Shutdown(); }

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	CScreenCaptureStream* m_pScreenCaptureStream;

	IMFMediaEventQueue* m_pEventQueue;
	IMFPresentationDescriptor* m_pPresentationDescriptor;;

	SourceState m_State;

	HRESULT CreateScreenCapturePresentationDescriptor();
	HRESULT CreateVideoMediaType(IMFMediaType**);
	HRESULT ValidatePresentationDescriptor(IMFPresentationDescriptor*);
	HRESULT QueueNewStreamEvent(IMFPresentationDescriptor*);

	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif
