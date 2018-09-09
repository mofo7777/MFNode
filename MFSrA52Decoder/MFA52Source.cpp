//----------------------------------------------------------------------------------------------
// MFA52Source.cpp
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
#include "StdAfx.h"

CMFA52Source::CMFA52Source(HRESULT& hr)
		: m_nRefCount(1),
		  m_pStream(NULL),
				m_pEventQueue(NULL),
				m_pPresentationDescriptor(NULL),
				m_pByteStream(NULL),
				m_pBeginOpenResult(NULL),
				m_A52State(NULL),
				m_State(SourceInvalid),
				m_bEOF(FALSE),
				m_rtCurrentPosition(0),
				m_uiAvgBytePerSec(0),
    m_OnByteStreamRead(this, &CMFA52Source::OnByteStreamRead){

		LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
		TRACE_SOURCE((L"Source::CTOR"));
}

HRESULT CMFA52Source::CreateInstance(CMFA52Source** ppSource){
		
		TRACE_SOURCE((L"Source::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));
		
		CMFA52Source* pSource = new (std::nothrow)CMFA52Source(hr);
		
		IF_FAILED_RETURN(pSource == NULL ? E_OUTOFMEMORY : S_OK);

		*ppSource = pSource;
		(*ppSource)->AddRef();

		SAFE_RELEASE(pSource);

		return hr;
}

HRESULT CMFA52Source::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_SOURCE((L"Source::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CMFA52Source, IMFMediaEventGenerator),
				QITABENT(CMFA52Source, IMFMediaSource),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CMFA52Source::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Source::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFA52Source::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Source::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

STDMETHODIMP CMFA52Source::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

		TRACE_SOURCE((L"Source::BeginGetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
  
		LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CMFA52Source::EndGetEvent(IMFAsyncResult* pCallback, IMFMediaEvent** punkState){

		TRACE_SOURCE((L"Source::EndGetEvent"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
  
		LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pCallback, punkState));

		return hr;
}

STDMETHODIMP CMFA52Source::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

		TRACE_SOURCE((L"Source::GetEvent"));

		HRESULT hr;

		IMFMediaEventQueue* pQueue = NULL;

		{
				AutoLock lock(m_CriticSection);

				LOG_HRESULT(hr = CheckShutdown());

				if(SUCCEEDED(hr)){

						pQueue = m_pEventQueue;
						pQueue->AddRef();
				}
		}

		if(SUCCEEDED(hr))
				LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));

		SAFE_RELEASE(pQueue);

		return hr;
}

STDMETHODIMP CMFA52Source::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

		TRACE_SOURCE((L"Source::QueueEvent : %s", MFEventString(met)));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}

STDMETHODIMP CMFA52Source::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){
		
		TRACE_SOURCE((L"Source::CreatePresentationDescriptor"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppPresentationDescriptor == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pPresentationDescriptor == NULL){
				IF_FAILED_RETURN(hr = E_FAIL);
				// Todo
				//IF_FAILED_RETURN(hr = CreateNewPresentationDescriptor());
		}

		LOG_HRESULT(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

		return hr;
}

STDMETHODIMP CMFA52Source::GetCharacteristics(DWORD* pdwCharacteristics){
		
		TRACE_SOURCE((L"Source::GetCharacteristics"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		// No Seek for now
		*pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE;

		return hr;
}

STDMETHODIMP CMFA52Source::Pause(){

		TRACE_SOURCE((L"Source::Pause"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_State != SourceStarted ? MF_E_INVALID_STATE_TRANSITION : S_OK));

		if(m_pStream){
				IF_FAILED_RETURN(hr = m_pStream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
		}

		IF_FAILED_RETURN(hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, NULL));

		m_State = SourcePaused;

		return hr;
}

STDMETHODIMP CMFA52Source::Shutdown(){

		TRACE_SOURCE((L"Source::Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pStream){
				LOG_HRESULT(m_pStream->Shutdown());
		}

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		m_cMFBuffer.Reset();
		m_rtCurrentPosition = 0;

		SAFE_RELEASE(m_pStream);
		SAFE_RELEASE(m_pEventQueue);
		SAFE_RELEASE(m_pPresentationDescriptor);

		#ifdef WRITE_WAVE_FILE
    UINT16 uiChannels = GetAC52Channels(m_A52State->acmod) + (UINT16)m_A52State->lfeon;

				UINT32 uiSamplePerSec;

				switch(m_A52State->fscod){

		    case 0x00: uiSamplePerSec = 48000; break;
				  case 0x01: uiSamplePerSec = 44100; break;
				  case 0x02: uiSamplePerSec = 32000; break;
				  default: uiSamplePerSec = 0;
		  }

				cMFWriteWaveFile.WriteWaveHeader(m_dwTotalSize, uiChannels, uiSamplePerSec);
				cMFWriteWaveFile.Close();
  #endif

  #ifdef WRITE_INPUT_FILE
    cMFWriteInputFile.Close();
  #endif

		if(m_A52State){
				a52_free(m_A52State);
				m_A52State = NULL;
		}

		m_State = SourceShutdown;

		return hr;
}

STDMETHODIMP CMFA52Source::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){

		TRACE_SOURCE((L"Source::Start"));

		// Start or Seek (we can't seek)
		HRESULT hr;
		
		IF_FAILED_RETURN(hr = (pvarStartPosition == NULL ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (pPresentationDescriptor == NULL ? E_INVALIDARG : S_OK));

		// Check the time format. We support only reference time, which is indicated by a NULL parameter or by time format = GUID_NULL.
		IF_FAILED_RETURN(hr = ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		// Check the data type of the start position.
		IF_FAILED_RETURN(hr = ((pvarStartPosition->vt != VT_I8) && (pvarStartPosition->vt != VT_EMPTY) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		// Check if this is a seek request. Currently, this sample does not support seeking.
		if(pvarStartPosition->vt == VT_I8){

				// If the current state is STOPPED, then position 0 is valid.
				// If the current state is anything else, then the start position must be VT_EMPTY (current position).
				IF_FAILED_RETURN(hr = ((m_State != SourceStopped) || (pvarStartPosition->hVal.QuadPart != 0) ? MF_E_INVALIDREQUEST : S_OK));
		}

		AutoLock lock(m_CriticSection);
		
		IMFMediaEvent* pEvent = NULL;

		PROPVARIANT var;
		PropVariantInit(&var);
		LONGLONG llStartOffset = 0;

		BOOL bQueuedStartEvent = FALSE;
		BOOL bIsRestartFromCurrentPosition = FALSE;

		try{

				IF_FAILED_THROW(hr = CheckShutdown());

				IF_FAILED_THROW(hr = (m_State == SourceInvalid ? E_FAIL: S_OK));
				
				if(pvarStartPosition->vt == VT_I8){
						
						// Start position is given in pvarStartPosition in 100-ns units.
						llStartOffset = pvarStartPosition->hVal.QuadPart;

						/*if(m_State != SourceStopped){
								// Source is running or paused, so this is a seek.
								bIsSeek = TRUE;
						}*/
				}
    else if(pvarStartPosition->vt == VT_EMPTY){
						
						// Start position is "current position". For stopped, that means 0. Otherwise, use the current position.
						if(m_State == SourceStopped){
								llStartOffset = 0;
						}
						else{
								
								llStartOffset = m_rtCurrentPosition;
								bIsRestartFromCurrentPosition = TRUE;
						}
				}

				//-----------------------------------------------------------------------------

				if(bIsRestartFromCurrentPosition == FALSE){
						
						#ifdef WRITE_WAVE_FILE
						  cMFWriteWaveFile.MFCreateFile(L"F:\\dvd\\ac3.wav");
						  byte bHeader[80];
		      cMFWriteWaveFile.MFWriteFile(bHeader, 80);
								m_dwTotalSize = 0;
      #endif

						#ifdef WRITE_INPUT_FILE
        cMFWriteInputFile.MFCreateFile(L"F:\\dvd\\input.ac3");
      #endif

						m_cMFBuffer.Reset();
						ULONG ulRead;
						//QWORD dwPos;
						//m_pByteStream->GetCurrentPosition(&dwPos);
						IF_FAILED_THROW(hr = m_pByteStream->SetCurrentPosition(llStartOffset));
						IF_FAILED_THROW(hr = m_cMFBuffer.Reserve(A52_BUFFER_SIZE));
						IF_FAILED_THROW(hr = m_pByteStream->Read(m_cMFBuffer.GetReadStartBuffer(), A52_BUFFER_SIZE, &ulRead));
						IF_FAILED_THROW(hr = m_cMFBuffer.SetEndPosition(ulRead));
				}

				m_bEOF = FALSE;
				//-----------------------------------------------------------------------------

				// Todo : check if same Descriptor
    //IF_FAILED_THROW(hr = ValidatePresentationDescriptor(pPresentationDescriptor));

				IF_FAILED_THROW(hr = QueueNewStreamEvent(pPresentationDescriptor));
				
				var.vt = VT_I8;
				var.hVal.QuadPart = llStartOffset;

				IF_FAILED_THROW(hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent));

				if(bIsRestartFromCurrentPosition){
						
						IF_FAILED_THROW(hr = pEvent->SetUINT64(MF_EVENT_SOURCE_ACTUAL_START, llStartOffset));
				}

				IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));

				bQueuedStartEvent = TRUE;

				IF_FAILED_THROW(hr = (m_pStream == NULL ? E_POINTER : S_OK));

				IF_FAILED_THROW(hr = m_pStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));

				m_State = SourceStarted;
		}
		catch(HRESULT){}

		if(FAILED(hr) && bQueuedStartEvent){
				LOG_HRESULT(hr = QueueEvent(MEError, GUID_NULL, hr, &var));
		}

		PropVariantClear(&var);
		SAFE_RELEASE(pEvent);

		return hr;
}

STDMETHODIMP CMFA52Source::Stop(){

		TRACE_SOURCE((L"Source::Stop"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_State = SourceStopped;

		m_cMFBuffer.Reset();
		m_rtCurrentPosition = 0;

		//if(m_pStream)
				//IF_FAILED_RETURN(hr = m_pStream->Flush());

		if(m_pStream){
				IF_FAILED_RETURN(hr = m_pStream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
		}

		LOG_HRESULT(hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, NULL));

		return hr;
}

HRESULT CMFA52Source::BeginOpen(IMFByteStream* pStream, IMFAsyncCallback* pCB, IUnknown* pState){

		TRACE_SOURCE((L"Source::BeginOpen"));

		AutoLock lock(m_CriticSection);

		if(pStream == NULL || pCB == NULL){
				return E_POINTER;
		}

		if(m_State != SourceInvalid){
				return MF_E_INVALIDREQUEST;
		}

		HRESULT hr = S_OK;
		DWORD dwCaps = 0;

		m_pByteStream = pStream;
		m_pByteStream->AddRef();

		IF_FAILED_RETURN(hr = pStream->GetCapabilities(&dwCaps));

		if((dwCaps & MFBYTESTREAM_IS_SEEKABLE) == 0){
				IF_FAILED_RETURN(hr = MF_E_BYTESTREAM_NOT_SEEKABLE);
		}

		if((dwCaps & MFBYTESTREAM_IS_READABLE) == 0){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		IF_FAILED_RETURN(hr = m_cMFBuffer.Initialize());

		IF_FAILED_RETURN(hr = MFCreateAsyncResult(NULL, pCB, pState, &m_pBeginOpenResult));

		IF_FAILED_RETURN(hr = m_cMFBuffer.Reserve(A52_BUFFER_SIZE));

		IF_FAILED_RETURN(hr = m_pByteStream->BeginRead(m_cMFBuffer.GetReadStartBuffer(), A52_BUFFER_SIZE, &m_OnByteStreamRead, NULL));

		m_State = SourceOpening;
		
		return hr;
}

HRESULT CMFA52Source::EndOpen(IMFAsyncResult* pResult){

		TRACE_SOURCE((L"Source::EndOpen"));

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		hr = pResult->GetStatus();

		if(FAILED(hr)){
				// The source is not designed to recover after failing to open. Switch to shut-down state.
				Shutdown();
		}

		return hr;
}

HRESULT CMFA52Source::OnByteStreamRead(IMFAsyncResult* pResult){
		
		TRACE_SOURCE((L"Source::OnByteStreamRead"));

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		if(m_State == SourceShutdown){
				return hr;
		}

		DWORD cbRead = 0;
		IUnknown* pState = NULL;
		IMFMediaType* pType = NULL;

		try{

				LOG_HRESULT(pResult->GetState(&pState));

				IF_FAILED_THROW(hr = m_pByteStream->EndRead(pResult, &cbRead));

    if(cbRead == 0){

						m_bEOF = TRUE;

						// Todo...
						/*if(m_pStream){

								IF_FAILED_THROW(hr = m_pStream->EndOfStream());
						}*/
				}
				else{

						IF_FAILED_THROW(hr = m_cMFBuffer.SetEndPosition(cbRead));

						assert(m_A52State == NULL);

						// 60 minimal header ??
						assert(cbRead > 60);

						m_A52State = a52_init();
						IF_FAILED_THROW(hr = (m_A52State == NULL ? E_POINTER : S_OK));

						int iSampleRate;
						int iBitRate;
      int iFlags;
						int iFrameLength = a52_syncinfo(m_cMFBuffer.GetStartBuffer(), &iFlags, &iSampleRate, &iBitRate);

						assert(iFrameLength);

						sample_t fLevel = 1;
						sample_t fBias = 0;
						iFlags |= A52_ADJUST_LEVEL;

						// level *= gain;

						if(a52_frame(m_A52State, m_cMFBuffer.GetStartBuffer(), &iFlags, &fLevel, fBias)){
								IF_FAILED_THROW(hr = E_FAIL);
						}

						IF_FAILED_THROW(hr = CreateAudioMediaType(&pType, iSampleRate, GetAC52Channels(m_A52State->acmod)));
						IF_FAILED_THROW(hr = CreateAudioStream(pType, iSampleRate, iBitRate));
				}
		}
		catch(HRESULT){}

		if(FAILED(hr)){
				StreamingError(hr);
		}
		
		SAFE_RELEASE(pType);
		SAFE_RELEASE(pState);
		
		return hr;
}

HRESULT CMFA52Source::CreateAudioMediaType(IMFMediaType** ppType, const int iSampleRate, const UINT16 uiChannels){
		
		TRACE_SOURCE((L"Source::CreateAudioMediaType"));

		HRESULT hr = S_OK;
		IMFMediaType* pType = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

				const UINT32 uiBitsPerSample = 32;
				const UINT32 uiBlockAlign = uiChannels * 4;
				m_uiAvgBytePerSec = uiBlockAlign * iSampleRate;

				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, uiChannels));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, iSampleRate));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, uiBlockAlign));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_uiAvgBytePerSec));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, uiBitsPerSample));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

				// Todo : check this
				//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_CHANNEL_MASK, 3));

				*ppType = pType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);
		return hr;
}

HRESULT CMFA52Source::CreateAudioStream(IMFMediaType* pType, const int iSampleRate, const int iBitRate){

		TRACE_SOURCE((L"Source::CreateAudioStream"));

		HRESULT hr = S_OK;
		IMFStreamDescriptor* pSD = NULL;
		CMFA52Stream* pStream = NULL;
		IMFMediaTypeHandler* pHandler = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateStreamDescriptor(0, 1, &pType, &pSD));
				IF_FAILED_THROW(hr = pSD->GetMediaTypeHandler(&pHandler));
				IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));
				
				IF_FAILED_THROW(hr = CMFA52Stream::CreateInstance(&pStream, this, pSD, hr));

				IF_FAILED_THROW(hr = (pStream == NULL ? E_OUTOFMEMORY : S_OK));

				m_pStream = pStream;
				m_pStream->AddRef();

				// Perhaps not needed here
				IF_FAILED_THROW(hr = InitPresentationDescriptor(iSampleRate, iBitRate));
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pStream);
		SAFE_RELEASE(pSD);

		return hr;
}

HRESULT CMFA52Source::InitPresentationDescriptor(const int iSampleRate, const int iBitRate){

		TRACE_SOURCE((L"Source::InitPresentationDescriptor"));

		HRESULT hr = S_OK;

		assert(m_pPresentationDescriptor == NULL);
		assert(m_pStream != NULL);
		assert(m_State == SourceOpening);

		IMFStreamDescriptor** ppSD = new (std::nothrow)IMFStreamDescriptor*[1];

		IF_FAILED_RETURN(ppSD == NULL ? E_OUTOFMEMORY : S_OK);
		
		ZeroMemory(ppSD, sizeof(IMFStreamDescriptor*));

		try{

				IF_FAILED_THROW(hr = m_pStream->GetStreamDescriptor(&ppSD[0]));

				IF_FAILED_THROW(hr = MFCreatePresentationDescriptor(1, ppSD, &m_pPresentationDescriptor));
				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(0));
				
				QWORD qwFileSize;

				IF_FAILED_THROW(hr = m_pByteStream->GetLength(&qwFileSize));
				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_TOTAL_FILE_SIZE, qwFileSize));

				// Todo : check zero divide...
				MFTIME duration = (qwFileSize * 8 ) / iBitRate;
				duration *= 10000000;
				//MFTimeString(duration);

				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_DURATION, duration));
				IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_AUDIO_ENCODING_BITRATE, iSampleRate * 8000));

				m_State = SourceStopped;

				IF_FAILED_THROW(hr = CompleteOpen(S_OK));
		}
		catch(HRESULT){}
		
		if(ppSD){

				SAFE_RELEASE(ppSD[0]);
				delete[] ppSD;
		}
		
		return hr;
}

HRESULT CMFA52Source::CompleteOpen(const HRESULT hrStatus){

		TRACE_SOURCE((L"Source::CompleteOpen"));

		HRESULT hr = S_OK;

		if(m_pBeginOpenResult){

				try{

						IF_FAILED_THROW(hr = m_pBeginOpenResult->SetStatus(hrStatus));
						IF_FAILED_THROW(hr = MFInvokeCallback(m_pBeginOpenResult));
				}
				catch(HRESULT){}
		}

		SAFE_RELEASE(m_pBeginOpenResult);
		
		return hr;
}

HRESULT CMFA52Source::QueueNewStreamEvent(IMFPresentationDescriptor* pPD){
		
		TRACE_SOURCE((L"Source::QueueNewStreamEvent"));

		assert(pPD != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSD = NULL;
		BOOL fSelected = FALSE;
		DWORD cStreams = 0;

		try{

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&cStreams));

				IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD));
				
				assert(fSelected);

				if(m_pStream){

						m_pStream->StartStream();
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pStream));
				}
				else{

						IF_FAILED_THROW(hr = CMFA52Stream::CreateInstance(&m_pStream, this, pSD, hr));

						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pStream));
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSD);

		return hr;
}

HRESULT CMFA52Source::GetSample(IMFSample** ppSample){

		TRACE_SOURCE((L"Source::GetSample"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		DWORD dwAte = 0;

		if(!m_bEOF){

				hr = DecodeA52(ppSample, m_cMFBuffer.GetStartBuffer(), m_cMFBuffer.GetBufferSize(), &dwAte);

				if(FAILED(hr)){

						// try/catch to handle this...
						if(*ppSample){
								SAFE_RELEASE(*ppSample);
								IF_FAILED_RETURN(hr);
						}
				}
				else{

						hr = ConvertOutput(*ppSample);
						
						if(FAILED(hr)){

								// try/catch to handle this...
						  SAFE_RELEASE(*ppSample);
								IF_FAILED_RETURN(hr);
						}
				}

				IF_FAILED_RETURN(hr = m_cMFBuffer.SetStartPosition(dwAte));

				IF_FAILED_RETURN(hr = m_cMFBuffer.Reserve(A52_BUFFER_SIZE));

				ULONG ulRead;

				IF_FAILED_RETURN(hr = m_pByteStream->Read(m_cMFBuffer.GetReadStartBuffer(), A52_BUFFER_SIZE, &ulRead));

				if(ulRead != 0){

						IF_FAILED_RETURN(hr = m_cMFBuffer.SetEndPosition(ulRead));
				}
				else{
						m_bEOF = TRUE;
						m_pStream->EndOfStream();
				}
		}

		return hr;
}

/*HRESULT CMFA52Source::DecodeA52(IMFSample** ppSample, BYTE* pData, const DWORD dwSize){

		TRACE_SOURCE((L"Source::DecodeA52"));
		
		HRESULT hr = S_OK;
		
		static uint8_t buf[3840];
  static uint8_t* bufptr = buf;
  static uint8_t* bufpos = buf + 7;

  // sample_rate and flags are static because this routine could
  // exit between the a52_syncinfo() and the ao_setup(), and we want
  // to have the same values when we get back !

  static int sample_rate;
  static int flags;

		BYTE* start1 = pData;
  BYTE* end1 = pData + dwSize;

  //int bit_rate;
  int len;

		//int disable_adjust = 0;
		//int disable_dynrng = 0;

		while(1){

    len = end1 - start1;

				if(!len)
	     break;

				if(len > bufpos - bufptr)
	     len = bufpos - bufptr;

				memcpy(bufptr, start1, len);

				bufptr += len;
	   start1 += len;

				if(bufptr == bufpos){

						if(bufpos == buf + 7){

								int length;

								length = a52_syncinfo(buf, &flags, &sample_rate);

								if(!length){

										//fprintf (stderr, "skip\n");

										for(bufptr = buf; bufptr < buf + 6; bufptr++)
												bufptr[0] = bufptr[1];

										continue;
								}

								m_iFrameLength += length;
								bufpos = buf + length;
						}
						else{

								sample_t level = 1;
								sample_t bias = 0;

								//if(ao_setup (output, sample_rate, &flags, &level, &bias))
		        //goto error;

								//if(!disable_adjust)
		        flags |= A52_ADJUST_LEVEL;

								// level *= gain;

								if(a52_frame(m_A52State, buf, &flags, &level, bias))
										goto error;

								//if(disable_dynrng)
										//a52_dynrng(state, NULL, NULL);

								IF_FAILED_RETURN(hr = CreateSample(ppSample, level));

								bufptr = buf;
		      bufpos = buf + 7;
		      //print_fps (0);
								continue;
	    
        error:
		        //fprintf (stderr, "error\n");
		        bufptr = buf;
		        bufpos = buf + 7;
						}
				}
		}

  return hr;
}*/

HRESULT CMFA52Source::DecodeA52(IMFSample** ppSample, BYTE* pData, const DWORD dwSize, DWORD* dwAte){

		TRACE_SOURCE((L"Source::DecodeA52"));

		#ifdef WRITE_INPUT_FILE
    cMFWriteInputFile.MFWriteFile(pData, dwSize);
  #endif
		
		HRESULT hr = S_OK;
		
		//unsigned char* p = pData;
  //unsigned char* base = p;
  //unsigned char* end = p + dwSize;

		BYTE* p = pData;
		DWORD dwCurSize = dwSize;
		int iBiteRate;

  while(dwCurSize > 7){

				int size = 0;
				int flags;
				int sample_rate;

				if((size = a52_syncinfo(p, &flags, &sample_rate, &iBiteRate)) > 0){

						//bool enoughData = p + size <= end;

						if(size <= dwCurSize){

								//if(!disable_adjust)
		        flags |= A52_ADJUST_LEVEL;

								// level *= gain;

								sample_t level = 1;
								sample_t bias = 0;

								if(a52_frame(m_A52State, p, &flags, &level, bias) == 0){

										//if(disable_dynrng)
										  //a52_dynrng(state, NULL, NULL);

										IF_FAILED_RETURN(hr = CreateSample(ppSample, level));
								}

								p += size;
								dwCurSize -= size;
      }
						else{
								break;
						}

						//memmove(base, p, end - p);
      //end = base + (end - p);
      //p = base;
						//if(!enoughData)
						//		break;
    }
				else{

						p++;
						dwCurSize--;
				}
		 }

		*dwAte = dwSize - dwCurSize;

  return hr;
}

HRESULT CMFA52Source::CreateSample(IMFSample** ppSample, const FLOAT level){

		TRACE_SOURCE((L"Source::CreateSample"));

		HRESULT hr = S_OK;
		IMFMediaBuffer* pBuffer = NULL;
		BYTE* pData = NULL;

		DWORD dwSizeBuffer = 6 * 256 * (GetAC52Channels(m_A52State->acmod) + m_A52State->lfeon) * sizeof(float);

		try{

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwSizeBuffer, &pBuffer));
				
				IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));
				
				FLOAT* pfData = (FLOAT*)pData;

				int i = 0;

				for(; i < 6 && a52_block(m_A52State) == 0; i++){

						sample_t* samples = a52_samples(m_A52State);

						for(int j = 0; j < 256; j++, samples++)
								for(int ch = 0; ch < 2; ch++)
										*pfData++ = float(*(samples + 256 * ch) / level);
						    // Don't know, but this work for a dvd i tested...
						    //*pfData++ = float(*(samples + 256 * ch) / level) / 53003768.55f;
						    // Need to much understand liba52...
				}

				assert(i == 6);

				#ifdef WRITE_WAVE_FILE
				  cMFWriteWaveFile.MFWriteFile(pData, dwSizeBuffer);
				  m_dwTotalSize += dwSizeBuffer;
				#endif

				IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwSizeBuffer));

				if(*ppSample == NULL){
						IF_FAILED_THROW(hr = MFCreateSample(ppSample));
				}

    IF_FAILED_THROW(hr = (*ppSample)->AddBuffer(pBuffer));
		}
		catch(HRESULT){}

		if(pBuffer && pData){

				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CMFA52Source::ConvertOutput(IMFSample* pSample){

		TRACE_SOURCE((L"Source::CreateSample"));

		HRESULT hr = S_OK;
		DWORD dwBufferCount = 0;
		IMFMediaBuffer* pBuffer = NULL;
		DWORD dwSize = 0;
		BYTE* btData = NULL;

		try{

				IF_FAILED_THROW(hr = pSample->GetBufferCount(&dwBufferCount));

				assert(dwBufferCount != 0);

				IF_FAILED_THROW(hr = pSample->ConvertToContiguousBuffer(&pBuffer));

				IF_FAILED_THROW(hr = pBuffer->Lock(&btData, NULL, &dwSize));
				//TRACE((L"Count = %d - Size = %d", dwBufferCount, dwSize));

				IF_FAILED_THROW(hr = pSample->RemoveAllBuffers());

				IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

				IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtCurrentPosition));

				LONGLONG duration = (LONGLONG)dwSize * 10000000 / m_uiAvgBytePerSec;
				IF_FAILED_THROW(hr = pSample->SetSampleDuration(duration));

				//TRACE((L"Time %I64d - Duration %I64d\r", m_rtCurrentPosition, duration));

				//MFTimeString(m_rtCurrentPosition);

				m_rtCurrentPosition += duration;
		}
		catch(HRESULT){}

		if(pBuffer && btData){
				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}