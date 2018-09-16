//----------------------------------------------------------------------------------------------
// MFMpeg12Source.h
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
#ifndef MFMPEG12SOURCE_H
#define MFMPEG12SOURCE_H

class CMFMpeg12Source : public IMFMediaSource{

public:

	static HRESULT CreateInstance(CMFMpeg12Source**);

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

	CMFMpeg12Source(HRESULT&);
	virtual ~CMFMpeg12Source(){ TRACE_SOURCE((L"Source::DTOR")); Shutdown(); }

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	SourceState m_State;
	BOOL m_bEOF;
	BOOL m_bSkipInfo;

	CMFMpeg12Stream* m_pStream;
	IMFMediaEventQueue* m_pEventQueue;
	IMFPresentationDescriptor* m_pPresentationDescriptor;
	IMFByteStream* m_pByteStream;
	IMFAsyncResult* m_pBeginOpenResult;

	CAsyncCallback<CMFMpeg12Source> m_OnByteStreamRead;

	CMFBuffer m_cMFBuffer;
	CMpeg12Decoder m_cMpeg12Decoder;

	short m_sAudioBuffer[4608];
	LONGLONG m_rtCurrentPosition;
	UINT m_uiAvgBytePerSec;

	HRESULT OnByteStreamRead(IMFAsyncResult*);
	HRESULT CreateAudioStream(IMFMediaType*, const UINT);
	HRESULT InitPresentationDescriptor(const UINT);
	HRESULT CompleteOpen(const HRESULT);
	HRESULT QueueNewStreamEvent(IMFPresentationDescriptor*);

	// MFMpeg12Source_Audio.cpp
	HRESULT ReadAudioHeader(Mpeg12AudioFrameHeader&);
	HRESULT GetAudioBitRateMpeg1(Mpeg12AudioLayer, BYTE, UINT*);
	HRESULT GetAudioBitRateMpeg2(Mpeg12AudioLayer, BYTE, UINT*);
	HRESULT GetSamplingFrequency(const BYTE, const BYTE, UINT*);
	HRESULT CreateAudioMediaType(IMFMediaType**, Mpeg12AudioFrameHeader&);
	HRESULT CreateSample(IMFSample**, const UINT);
	HRESULT SkipInfo(BOOL&);

	// Inline
	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
	LONGLONG GetCurrentPosition() const;
	void StreamingError(const HRESULT);
};

inline void CMFMpeg12Source::StreamingError(const HRESULT hr){

	TRACE_SOURCE((L"Source::StreamingError"));

	if(m_State == SourceOpening){

		CompleteOpen(hr);
	}
	else if(m_State != SourceShutdown){

		LOG_HRESULT(QueueEvent(MEError, GUID_NULL, hr, NULL));
	}
}

#endif
