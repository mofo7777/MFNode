//----------------------------------------------------------------------------------------------
// MFMpeg2Stream.h
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
#ifndef MFMPEG2STREAM_H
#define MFMPEG2STREAM_H

class CMFMpeg2Stream : BaseObject, public IMFMediaStream{

  public:

				CMFMpeg2Stream(CMFMpeg2Source*, IMFStreamDescriptor*, HRESULT& hr);
				~CMFMpeg2Stream(){ TRACE_STREAM((L"Stream::DTOR")); assert(m_State == StreamFinalized); SAFE_RELEASE(m_pSource); }

    // IUnknown - MFMpeg2Stream.cpp
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFMediaEventGenerator - MFMpeg2Stream_Event.cpp
    STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
    STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
    STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
    STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

    // IMFMediaStream - MFMpeg2Stream_Stream.cpp
    STDMETHODIMP GetMediaSource(IMFMediaSource**);
    STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor**);
    STDMETHODIMP RequestSample(IUnknown*);

    // Other methods (called by source) - MFMpeg2Stream.cpp
				HRESULT Activate(BOOL);
    HRESULT Start(const PROPVARIANT&);
    HRESULT Pause();
    HRESULT Stop();
    HRESULT EndOfStream();
    HRESULT Shutdown();

				HRESULT DeliverPayload(IMFSample*);
    //HRESULT OnDispatchSamples(IMFAsyncResult*);
				void ResetSample(){ m_Samples.Clear(); m_Requests.Clear(); m_bEOS = FALSE; }

				// inline
				BOOL IsActive() const{ return m_bActive; }
    BOOL NeedsData(){ SourceLock lock(m_pSource); return (m_bActive && !m_bEOS && (m_Samples.GetCount() < SAMPLE_QUEUE)); }

  private:

				volatile long m_nRefCount;

    // SourceLock class:
    // Small helper class to lock and unlock the source. 
    // It works like the AutoLock class in Common\critsec.h.
    class SourceLock{

				  public:
								
								SourceLock(CMFMpeg2Source* pSource);
								~SourceLock();

				  private:

								CMFMpeg2Source* m_pSource;				
    };

    CMFMpeg2Source*      m_pSource;
    IMFStreamDescriptor* m_pStreamDescriptor;
    IMFMediaEventQueue*  m_pEventQueue;

    StreamState m_State;
    BOOL m_bActive;
    BOOL m_bEOS;

    SampleList m_Samples;
    TokenList m_Requests;
				
    HRESULT DispatchSamples();
    HRESULT CheckShutdown() const{ return (m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK); }
};

#endif