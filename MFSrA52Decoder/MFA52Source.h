//----------------------------------------------------------------------------------------------
// MFA52Source.h
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
#ifndef MFA52SOURCE_H
#define MFA52SOURCE_H

class CMFA52Source : BaseObject, public IMFMediaSource{

public:

	static HRESULT CreateInstance(CMFA52Source**);

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

	HRESULT BeginOpen(IMFByteStream*, IMFAsyncCallback*, IUnknown*);
	HRESULT EndOpen(IMFAsyncResult*);
	HRESULT GetSample(IMFSample**);

	SourceState GetState() const { /*AutoLock lock(m_CriticSection);*/ return m_State; }

private:

	CMFA52Source(HRESULT&);
	virtual ~CMFA52Source(){ TRACE_SOURCE((L"Source::DTOR")); Shutdown(); }

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	SourceState m_State;
	BOOL m_bEOF;

	CMFA52Stream* m_pStream;
	IMFMediaEventQueue* m_pEventQueue;
	IMFPresentationDescriptor* m_pPresentationDescriptor;
	IMFByteStream* m_pByteStream;
	IMFAsyncResult* m_pBeginOpenResult;

	CAsyncCallback<CMFA52Source> m_OnByteStreamRead;

	CMFBuffer m_cMFBuffer;
	a52_state_t* m_A52State;

	LONGLONG m_rtCurrentPosition;
	UINT m_uiAvgBytePerSec;

	HRESULT OnByteStreamRead(IMFAsyncResult*);
	HRESULT CreateAudioMediaType(IMFMediaType**, const int, const UINT16);
	HRESULT CreateAudioStream(IMFMediaType*, const int, const int);
	HRESULT InitPresentationDescriptor(const int, const int);
	HRESULT CompleteOpen(const HRESULT);
	HRESULT QueueNewStreamEvent(IMFPresentationDescriptor*);
	HRESULT DecodeA52(IMFSample**, BYTE*, const DWORD, DWORD*);
	HRESULT CreateSample(IMFSample**, const FLOAT);
	HRESULT ConvertOutput(IMFSample*);

	// Inline
	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
	LONGLONG GetCurrentPosition() const;
	void StreamingError(const HRESULT);
	UINT16 GetAC52Channels(const BYTE);

#ifdef WRITE_WAVE_FILE
	CMFWriteWaveFile cMFWriteWaveFile;
	DWORD m_dwTotalSize;
#endif

#ifdef WRITE_INPUT_FILE
	CMFWriteWaveFile cMFWriteInputFile;
#endif
};

inline void CMFA52Source::StreamingError(const HRESULT hr){

	TRACE_SOURCE((L"Source::StreamingError"));

	if(m_State == SourceOpening){

		CompleteOpen(hr);
	}
	else if(m_State != SourceShutdown){

		LOG_HRESULT(QueueEvent(MEError, GUID_NULL, hr, NULL));
	}
}

inline UINT16 CMFA52Source::GetAC52Channels(const BYTE bAcmode){

	UINT16 uiChannels;

	switch(bAcmode){

	case 0x00: uiChannels = 2; break;
	case 0x01: uiChannels = 1; break;
	case 0x02: uiChannels = 2; break;
	case 0x03: uiChannels = 3; break;
	case 0x04: uiChannels = 3; break;
	case 0x05: uiChannels = 4; break;
	case 0x06: uiChannels = 4; break;
	case 0x07: uiChannels = 5; break;
	default: uiChannels = 0;
	}

	return uiChannels;
}

#endif