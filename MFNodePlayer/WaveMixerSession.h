//----------------------------------------------------------------------------------------------
// WaveMixerSession.h
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
//----------------------------------------------------------------------------------------------
// Thanks to Anton Polinger and his book : "Developing Microsoft Media Foundation Application".
// It helps me a lot to implement IMFMediaSession.
//----------------------------------------------------------------------------------------------
#ifndef WAVEMIXERSESSION_H
#define WAVEMIXERSESSION_H

class CWaveMixerSession : public IMFMediaSession, public IMFAsyncCallback{

  public:
				
				// WaveMixerSession.cpp
				CWaveMixerSession(PCWSTR, PCWSTR);
				virtual ~CWaveMixerSession();

				// IUnknown - WaveMixerSession.cpp
				STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

				// IMFAsyncCallback - WaveMixerSession.cpp
				STDMETHODIMP GetParameters(DWORD*, DWORD*){ TRACE_SESSION((L"Session::GetParameters")); return E_NOTIMPL; }
				STDMETHODIMP Invoke(IMFAsyncResult*);

				// IMFMediaSession - WaveMixerSession.cpp
				STDMETHODIMP SetTopology(DWORD, IMFTopology*){ TRACE_SESSION((L"Session::SetTopology")); return E_NOTIMPL; }
				STDMETHODIMP ClearTopologies(){ TRACE_SESSION((L"Session::ClearTopologies")); return E_NOTIMPL; }
				STDMETHODIMP Start(const GUID*, const PROPVARIANT*);
				STDMETHODIMP Pause();
				STDMETHODIMP Stop();
				STDMETHODIMP Close();
				STDMETHODIMP Shutdown();
				STDMETHODIMP GetClock(IMFClock**);
				STDMETHODIMP GetSessionCapabilities(DWORD*);
				STDMETHODIMP GetFullTopology(DWORD, TOPOID, IMFTopology**){ TRACE_SESSION((L"Session::GetFullTopology")); return E_NOTIMPL; }

				// IMFMediaEventGenerator - WaveMixerSession.cpp
				STDMETHODIMP BeginGetEvent(IMFAsyncCallback*,IUnknown*);
				STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
				STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
				STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

				// WaveMixerSession.cpp
				// We should use SetTopology from IMFMediaSession, but this is a quick way to init Topology.
				HRESULT InitTopology();

  private:
				
				CriticSection m_CriticSection;
				IMFMediaEventQueue* m_pEventQueue;
				DWORD m_syncMftWorkerQueue;
				bool m_sessionStarted;
				volatile long m_nRefCount;
				volatile BOOL bTest;

				HANDLE m_hEvents[5];

				// Todo : event for waiting bytestream end init... or try to use CancelObjectCreation
				//HANDLE m_hByteStreamEvents[2];

				wstring m_szFileUrl1;
				wstring m_szFileUrl2;

				IMFSample* m_pReadySourceSample1;
				IMFSample* m_pReadySourceSample2;
				HANDLE m_sourceSampleReadyEvent1;
				HANDLE m_sourceSampleReadyEvent2;

				IMFPresentationClock* m_pClock; 

				IMFByteStreamHandler* m_pByteStreamHandler1;
				IMFByteStreamHandler* m_pByteStreamHandler2;
    IMFPresentationDescriptor* m_pPresentation1;
    IMFPresentationDescriptor* m_pPresentation2;

    IMFMediaSource* m_pSource1;
				IMFMediaSource* m_pSource2;
    IMFMediaStream* m_pSourceStream1;
				IMFMediaStream* m_pSourceStream2;

    IMFTransform* m_pWaveMixer;
    IMFTransform* m_pResampler;
    IMFMediaSink* m_pSink;
    IMFStreamSink* m_pStreamSink; 
				
				// WaveMixerSession.cpp
				HRESULT StopStreaming();
				HRESULT CloseStreaming();
				HRESULT ListenEvents();
    HRESULT HandleByteStreamHandlerEvent(IMFAsyncResult*, IMFByteStreamHandler*, IMFMediaSource**);
				HRESULT HandleSourceEvent(IMFAsyncResult*, IMFMediaSource*, IMFMediaStream**, const MFNodeMediaEvent, const MFNodeMediaEvent, const int);
				HRESULT HandleSourceStreamEvent(IMFAsyncResult*, IMFMediaStream*, IMFSample**, HANDLE, const MFNodeMediaEvent, const int);
				HRESULT HandleStreamSinkEvent(IMFAsyncResult*);
				HRESULT HandleSynchronousMftRequest();
				HRESULT PullDataFromMFT(IMFTransform*, IMFSample**);
				HRESULT InitOutputDataBuffer(IMFTransform*, MFT_OUTPUT_DATA_BUFFER*);
				HRESULT PullDataFromSource(IMFSample**, IMFSample**);

				// WaveMixerSession_Topology.cpp
    HRESULT LoadCustomTopology();
    HRESULT BeginCreateMediaSource(IMFByteStreamHandler**, const WCHAR*, const MFNodeMediaEvent);
    HRESULT NegotiateMediaTypes();
    HRESULT ConnectSourceToMft(IMFPresentationDescriptor**, IMFMediaSource*, IMFTransform*, const DWORD);
    HRESULT ConnectMftToMft(IMFTransform*, IMFTransform*);
    HRESULT ConnectMftToSink(IMFTransform*);
    HRESULT FireTopologyReadyEvent();
};

#endif