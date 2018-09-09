//----------------------------------------------------------------------------------------------
// FlvSource.h
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
#ifndef FLVSOURCE_H
#define FLVSOURCE_H

class CFlvSource : BaseObject, public IMFMediaSource{

  public:

				static HRESULT CreateInstance(CFlvSource**);

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

				// FlvSource.cpp
				HRESULT BeginOpen(LPCWSTR, IMFAsyncCallback*, IUnknown*);
				HRESULT EndOpen(IMFAsyncResult*);

				// FlvSource_Sample.cpp
				HRESULT GetSample(IMFSample**, const DWORD);

				// Inline
				SourceState GetState() const { /*AutoLock lock(m_CriticSection);*/ return m_State; }

  private:

    CFlvSource(HRESULT&);
				virtual ~CFlvSource(){ TRACE_SOURCE((L"Source::DTOR")); Shutdown(); }

				CriticSection m_CriticSection;
				volatile long m_nRefCount;
				SourceState m_State;
				BOOL m_bParsing;

    CFlvStream* m_pVideoStream;
				CFlvStream* m_pAudioStream;

    IMFMediaEventQueue* m_pEventQueue;
    IMFPresentationDescriptor* m_pPresentationDescriptor;
				//IMFByteStream* m_pByteStream;
				CMFByteStream* m_pByteStream;
				IMFAsyncResult* m_pBeginOpenResult;
								
				CMFBuffer m_cReadBuffer;
				CFlvParser m_cFlvParser;

				CAsyncCallback<CFlvSource> m_OnByteStreamRead;
				
				LONGLONG m_llStatrTime;
				BOOL m_bEOS;

				queue<IMFSample*> m_qVideoSample;
				queue<IMFSample*> m_qAudioSample;

				// FlvSource.cpp
				HRESULT RequestData(DWORD);
				HRESULT OnByteStreamRead(IMFAsyncResult*);
				void StreamingError(const HRESULT);
				HRESULT CompleteOpen(const HRESULT);
				HRESULT QueueNewStreamEvent(IMFPresentationDescriptor*);
				HRESULT GetStreamMajorType(IMFStreamDescriptor*, GUID*);

				// FlvSource_Parse.cpp
				HRESULT ParseData();
				HRESULT CheckInfo();
				BOOL NeedData() const;
				HRESULT ReadPayload(DWORD*, DWORD*);
				BOOL IsStreamActive(const BYTE);

				// FlvSource_stream.cpp
				HRESULT CreateAudioStream();
				HRESULT CreateVideoStream();
				HRESULT CreateAudioAACMediaType(IMFMediaType**);
				HRESULT CreateAudioMpegMediaType(IMFMediaType**);
				HRESULT CreateVideoH264MediaType(IMFMediaType**);
				HRESULT CreateVideoVP6MediaType(IMFMediaType**);
				HRESULT CreateVideoH263MediaType(IMFMediaType**);
				HRESULT InitPresentationDescriptor();
				HRESULT InitVideoPresentationDescriptor();
				HRESULT InitAudioPresentationDescriptor();

				// FlvSource_Sample.cpp
				BOOL GetSampleFromList(IMFSample**, const DWORD);
				HRESULT PrepareSample(IMFSample**, const DWORD);
				HRESULT CreateVideoSample(IMFSample**);
				HRESULT CreateAudioSample(IMFSample**);

				// Inline
				HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
				void ReleaseSampleList();
};

inline void CFlvSource::ReleaseSampleList(){

		IMFSample* pSample = NULL;

  while(!m_qVideoSample.empty()){
    
				pSample = m_qVideoSample.front();
				SAFE_RELEASE(pSample);
    m_qVideoSample.pop();
  }

  while(!m_qAudioSample.empty()){
    
				pSample = m_qAudioSample.front();
				SAFE_RELEASE(pSample);
    m_qAudioSample.pop();
  }
}

inline BOOL CFlvSource::IsStreamActive(const BYTE btFlvTag){

		if(btFlvTag == FLV_TAG_VIDEO || btFlvTag == FLV_TAG_AUDIO)
    return TRUE;

  return FALSE;
}

inline HRESULT CFlvSource::GetStreamMajorType(IMFStreamDescriptor* pSD, GUID* pguidMajorType){
		
		HRESULT hr;

		IF_FAILED_RETURN(hr = (pSD == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pguidMajorType == NULL ? E_POINTER : S_OK));
		
		IMFMediaTypeHandler* pHandler = NULL;

		hr = pSD->GetMediaTypeHandler(&pHandler);
		
		if(SUCCEEDED(hr)){
				hr = pHandler->GetMajorType(pguidMajorType);
		}

		/*IMFMediaType* pType = NULL;

		hr = pHandler->GetCurrentMediaType(&pType);
		
		if(SUCCEEDED(hr)){

				LogMediaType(pType);
		}

		SAFE_RELEASE(pType);*/
		
		SAFE_RELEASE(pHandler);
		
		return hr;
}

#endif