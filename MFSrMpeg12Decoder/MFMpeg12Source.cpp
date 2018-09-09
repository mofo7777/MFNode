//----------------------------------------------------------------------------------------------
// MFMpeg12Source.cpp
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
#include "StdAfx.h"

CMFMpeg12Source::CMFMpeg12Source(HRESULT& hr)
	: m_nRefCount(1),
	m_pStream(NULL),
	m_pEventQueue(NULL),
	m_pPresentationDescriptor(NULL),
	m_pByteStream(NULL),
	m_pBeginOpenResult(NULL),
	m_State(SourceInvalid),
	m_bEOF(FALSE),
	m_bSkipInfo(TRUE),
	m_rtCurrentPosition(0),
	m_uiAvgBytePerSec(0),
	m_OnByteStreamRead(this, &CMFMpeg12Source::OnByteStreamRead){

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
	TRACE_SOURCE((L"Source::CTOR"));
}

HRESULT CMFMpeg12Source::CreateInstance(CMFMpeg12Source** ppSource){

	TRACE_SOURCE((L"Source::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));

	CMFMpeg12Source* pSource = new (std::nothrow)CMFMpeg12Source(hr);

	IF_FAILED_RETURN(pSource == NULL ? E_OUTOFMEMORY : S_OK);

	*ppSource = pSource;
	(*ppSource)->AddRef();

	SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CMFMpeg12Source::QueryInterface(REFIID riid, void** ppv){

	TRACE_SOURCE((L"Source::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CMFMpeg12Source, IMFMediaEventGenerator),
			QITABENT(CMFMpeg12Source, IMFMediaSource),
			{ 0 }
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CMFMpeg12Source::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CMFMpeg12Source::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

STDMETHODIMP CMFMpeg12Source::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

	TRACE_SOURCE((L"Source::BeginGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

	return hr;
}

STDMETHODIMP CMFMpeg12Source::EndGetEvent(IMFAsyncResult* pCallback, IMFMediaEvent** punkState){

	TRACE_SOURCE((L"Source::EndGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pCallback, punkState));

	return hr;
}

STDMETHODIMP CMFMpeg12Source::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

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

STDMETHODIMP CMFMpeg12Source::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

	TRACE_SOURCE((L"Source::QueueEvent : %s", MFEventString(met)));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

	return hr;
}

STDMETHODIMP CMFMpeg12Source::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){

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

STDMETHODIMP CMFMpeg12Source::GetCharacteristics(DWORD* pdwCharacteristics){

	TRACE_SOURCE((L"Source::GetCharacteristics"));

	HRESULT hr;

	IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	// No Seek for now
	*pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE;

	return hr;
}

STDMETHODIMP CMFMpeg12Source::Pause(){

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

STDMETHODIMP CMFMpeg12Source::Shutdown(){

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

	m_State = SourceShutdown;

	return hr;
}

STDMETHODIMP CMFMpeg12Source::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){

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

		IF_FAILED_THROW(hr = (m_State == SourceInvalid ? E_FAIL : S_OK));

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
				m_bSkipInfo = TRUE;
				llStartOffset = 0;
			}
			else{

				llStartOffset = m_rtCurrentPosition;
				bIsRestartFromCurrentPosition = TRUE;
			}
		}

		//-----------------------------------------------------------------------------

		if(bIsRestartFromCurrentPosition == FALSE){

			m_cMFBuffer.Reset();
			ULONG ulRead;
			//QWORD dwPos;
			//m_pByteStream->GetCurrentPosition(&dwPos);
			IF_FAILED_THROW(hr = m_pByteStream->SetCurrentPosition(llStartOffset));
			IF_FAILED_THROW(hr = m_cMFBuffer.Reserve(MPEG12_READ_SIZE));
			IF_FAILED_THROW(hr = m_pByteStream->Read(m_cMFBuffer.GetReadStartBuffer(), MPEG12_READ_SIZE, &ulRead));
			IF_FAILED_THROW(hr = m_cMFBuffer.SetEndPosition(ulRead));
			m_cMpeg12Decoder.Initialize();
			//m_pMp2Decoder->Initialize();
			/*if(m_AVCodecContext == NULL)
					MP2_decode_init(&m_AVCodecContext);*/
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

STDMETHODIMP CMFMpeg12Source::Stop(){

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

HRESULT CMFMpeg12Source::BeginOpen(IMFByteStream* pStream, IMFAsyncCallback* pCB, IUnknown* pState){

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

	IF_FAILED_RETURN(hr = m_cMFBuffer.Initialize(MPEG12_READ_SIZE));

	// Create an async result object. We'll use it later to invoke the callback.
	IF_FAILED_RETURN(hr = MFCreateAsyncResult(NULL, pCB, pState, &m_pBeginOpenResult));

	// Start reading data from the stream.
	IF_FAILED_RETURN(hr = m_cMFBuffer.Reserve(MPEG12_READ_SIZE));

	IF_FAILED_RETURN(hr = m_pByteStream->BeginRead(m_cMFBuffer.GetReadStartBuffer(), MPEG12_READ_SIZE, &m_OnByteStreamRead, NULL));

	m_State = SourceOpening;

	return hr;
}

HRESULT CMFMpeg12Source::EndOpen(IMFAsyncResult* pResult){

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

HRESULT CMFMpeg12Source::OnByteStreamRead(IMFAsyncResult* pResult){

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

			Mpeg12AudioFrameHeader pMpeg12Header;

			IF_FAILED_THROW(hr = ReadAudioHeader(pMpeg12Header));
			IF_FAILED_THROW(hr = CreateAudioMediaType(&pType, pMpeg12Header));
			IF_FAILED_THROW(hr = CreateAudioStream(pType, pMpeg12Header.uiBitrate));
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

HRESULT CMFMpeg12Source::CreateAudioStream(IMFMediaType* pType, const UINT uiBitrate){

	TRACE_SOURCE((L"Source::CreateAudioStream"));

	HRESULT hr = S_OK;
	IMFStreamDescriptor* pSD = NULL;
	CMFMpeg12Stream* pStream = NULL;
	IMFMediaTypeHandler* pHandler = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateStreamDescriptor(0, 1, &pType, &pSD));
		IF_FAILED_THROW(hr = pSD->GetMediaTypeHandler(&pHandler));
		IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));

		IF_FAILED_THROW(hr = CMFMpeg12Stream::CreateInstance(&pStream, this, pSD, hr));

		IF_FAILED_THROW(hr = (pStream == NULL ? E_OUTOFMEMORY : S_OK));

		m_pStream = pStream;
		m_pStream->AddRef();

		// Perhaps not needed here
		IF_FAILED_THROW(hr = InitPresentationDescriptor(uiBitrate));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pHandler);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pSD);

	return hr;
}

HRESULT CMFMpeg12Source::InitPresentationDescriptor(const UINT uiBitrate){

	TRACE_SOURCE((L"Source::InitPresentationDescriptor"));

	HRESULT hr = S_OK;
	QWORD qwFileSize;
	MFTIME duration;

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

		IF_FAILED_THROW(hr = m_pByteStream->GetLength(&qwFileSize));
		IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_TOTAL_FILE_SIZE, qwFileSize));

		// Todo : check zero divide...
		duration = (qwFileSize / uiBitrate) * 8;
		duration *= 10000;
		MFTimeString(duration);

		IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_DURATION, duration));
		IF_FAILED_THROW(hr = m_pPresentationDescriptor->SetUINT64(MF_PD_AUDIO_ENCODING_BITRATE, uiBitrate * 8000));

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

HRESULT CMFMpeg12Source::CompleteOpen(const HRESULT hrStatus){

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

HRESULT CMFMpeg12Source::QueueNewStreamEvent(IMFPresentationDescriptor* pPD){

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

			IF_FAILED_THROW(hr = CMFMpeg12Stream::CreateInstance(&m_pStream, this, pSD, hr));

			IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pStream));
		}
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSD);

	return hr;
}