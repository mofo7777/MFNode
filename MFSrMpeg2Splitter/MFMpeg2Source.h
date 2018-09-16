//----------------------------------------------------------------------------------------------
// MFMpeg2Source.h
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
#ifndef MFMPEG2SOURCE_H
#define MFMPEG2SOURCE_H

class CMFMpeg2Source : public IMFMediaSource, public CMFOperationQueue<CMFSourceOperation>{

public:

	// MFMpeg2Source.cpp
	static HRESULT CreateInstance(CMFMpeg2Source**);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaEventGenerator - MFMpeg2Source_Event.cpp
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaSource - MFMpeg2Source_Source.cpp
	STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor**);
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP Pause();
	STDMETHODIMP Shutdown();
	STDMETHODIMP Start(IMFPresentationDescriptor*, const GUID*, const PROPVARIANT*);
	STDMETHODIMP Stop();

	// MFMpeg2Source.cpp
	HRESULT BeginOpen(LPCWSTR, IMFAsyncCallback*, IUnknown*);
	HRESULT EndOpen(IMFAsyncResult*);
	HRESULT OnByteStreamRead(IMFAsyncResult*);
	HRESULT QueueAsyncOperation(CMFSourceOperation::Operation);

	// Called by the CMFMpeg2Stream
	void Lock(){ m_CriticSection.Lock(); }
	void Unlock(){ m_CriticSection.Unlock(); }

private:

	CMFMpeg2Source(HRESULT&);
	virtual ~CMFMpeg2Source();

	volatile long m_nRefCount;

	CriticSection m_CriticSection;
	SourceState m_State;

	CMFBuffer m_cReadBuffer;
	CMFParser m_cParser;
	BOOL m_bSkipInfo;

	IMFMediaEventQueue*        m_pEventQueue;
	IMFPresentationDescriptor* m_pPresentationDescriptor;
	IMFAsyncResult*            m_pBeginOpenResult;
	CMFByteStream*             m_pByteStream;

	StreamList m_streams;
	StreamMap m_streamMap;

	// Parser handle it, is it needed, perhaps if async
	MPEG2SystemHeader m_SysHeader;

	DWORD m_cPendingEOS;
	ULONG m_cRestartCounter;

	CMFSourceOperation* m_pCurrentOp;
	CMFSourceOperation* m_pSampleRequest;

	CAsyncCallback<CMFMpeg2Source> m_OnByteStreamRead;

	LONGLONG m_hnsStart;

	// MFMpeg2Source.cpp
	HRESULT CompleteOpen(HRESULT);

	// MFMpeg2Source_Operation.cpp
	HRESULT DoStart(CMFStartOperation*);
	HRESULT DoStop(CMFSourceOperation*);
	HRESULT DoPause(CMFSourceOperation*);
	HRESULT OnStreamRequestSample(CMFSourceOperation*);
	HRESULT OnEndOfStream(CMFSourceOperation*);
	HRESULT BeginAsyncOp(CMFSourceOperation*);
	HRESULT CompleteAsyncOp(CMFSourceOperation*);
	HRESULT DispatchOperation(CMFSourceOperation*);
	HRESULT ValidateOperation(CMFSourceOperation*);

	// MFMpeg2Source_Stream.cpp
	HRESULT ValidatePresentationDescriptor(IMFPresentationDescriptor*);
	HRESULT SelectStreams(IMFPresentationDescriptor*, const PROPVARIANT);
	HRESULT RequestData(DWORD);
	HRESULT EndOfMPEGStream();
	HRESULT ParseData();
	BOOL StreamsNeedData() const;
	HRESULT ReadPayload(DWORD*, DWORD*);
	HRESULT DeliverPayload();
	HRESULT SkipInfo(DWORD*);
	HRESULT CreateStream(const MPEG2PacketHeader&);
	HRESULT InitPresentationDescriptor();
	HRESULT GetVideoDuration(MFTIME&);

	// MFMpeg2Source_MediaType.cpp
	HRESULT CreateVideoMediaType(const MPEG2VideoSeqHeader&, IMFMediaType**, const BOOL);
	HRESULT CreateAudioMediaType(const MPEG2AudioFrameHeader&, IMFMediaType**);
	HRESULT CreateAC3AudioMediaType(const WAVEFORMATEX&, IMFMediaType**);

	// inline
	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
	BOOL IsStreamTypeSupported(StreamType type) const{ return (type == StreamType_Video || type == StreamType_Audio || type == StreamType_Private1); }
	HRESULT IsInitialized() const;
	void StreamingError(HRESULT);
	CMFMpeg2Stream* GetStream(BYTE);
	BOOL IsStreamActive(const MPEG2PacketHeader&);
	HRESULT GetStreamMajorType(IMFStreamDescriptor*, GUID*);
};

inline HRESULT CMFMpeg2Source::IsInitialized() const{

	if(m_State == SourceOpening || m_State == SourceInvalid){
		return MF_E_NOT_INITIALIZED;
	}
	else{
		return S_OK;
	}
}

inline void CMFMpeg2Source::StreamingError(HRESULT hr){

	TRACE((L"Source::StreamingError"));

	if(m_State == SourceOpening){

		CompleteOpen(hr);
	}
	else if(m_State != SourceShutdown){

		QueueEvent(MEError, GUID_NULL, hr, NULL);
	}
}

inline CMFMpeg2Stream* CMFMpeg2Source::GetStream(BYTE stream_id){

	CMFMpeg2Stream* pStream = NULL;

	DWORD index = 0;

	HRESULT hr = m_streamMap.Find(stream_id, &index);

	if(SUCCEEDED(hr)){

		assert(m_streams.GetCount() > index);
		pStream = m_streams[index];
	}

	return pStream;
}

inline BOOL CMFMpeg2Source::IsStreamActive(const MPEG2PacketHeader& packetHdr){

	if(m_State == SourceOpening){

		return IsStreamTypeSupported(packetHdr.type);
	}
	else{

		CMFMpeg2Stream* pStream = GetStream(packetHdr.stream_id);

		if(pStream == NULL){
			return FALSE;
		}
		else{
			return pStream->IsActive();
		}
	}
}

inline HRESULT CMFMpeg2Source::GetStreamMajorType(IMFStreamDescriptor* pSD, GUID* pguidMajorType){

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
