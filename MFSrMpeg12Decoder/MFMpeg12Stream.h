//----------------------------------------------------------------------------------------------
// MFMpeg12Stream.h
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
#ifndef MFMPEG12STREAM_H
#define MFMPEG12STREAM_H

class CMFMpeg12Stream : BaseObject, public IMFMediaStream{

  public:

				static HRESULT CreateInstance(CMFMpeg12Stream**, CMFMpeg12Source*, IMFStreamDescriptor*, HRESULT&);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
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

				//HRESULT Flush(){ return S_OK; }
				HRESULT Shutdown();
				void StartStream(){ AutoLock lock(m_CriticSection); m_bEOS = FALSE; }
				void EndOfStream(){ AutoLock lock(m_CriticSection); HRESULT hr; LOG_HRESULT(hr = QueueEvent(MEEndOfStream, GUID_NULL, S_OK, NULL)); m_bEOS = TRUE; }

  private:

				CMFMpeg12Stream(CMFMpeg12Source*, IMFStreamDescriptor*, HRESULT&);
				virtual ~CMFMpeg12Stream(){ TRACE_STREAM((L"Stream::DTOR")); Shutdown(); }
				
				CriticSection m_CriticSection;
				volatile long m_nRefCount;
    StreamState m_State;

    IMFMediaEventQueue* m_pEventQueue;
    IMFStreamDescriptor* m_pStreamDescriptor;

				CMFMpeg12Source* m_pSource;
				BOOL m_bEOS;

				HRESULT CheckShutdown() const{ return ( m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK ); }
};

#endif