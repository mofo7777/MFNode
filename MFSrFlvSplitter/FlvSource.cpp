//----------------------------------------------------------------------------------------------
// FlvSource.cpp
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

CFlvSource::CFlvSource(HRESULT& hr)
	: m_nRefCount(1),
	m_bEOS(FALSE),
	m_bParsing(TRUE),
	m_pVideoStream(NULL),
	m_pAudioStream(NULL),
	m_llStatrTime(0),
	m_pEventQueue(NULL),
	m_pPresentationDescriptor(NULL),
	m_pByteStream(NULL),
	m_pBeginOpenResult(NULL),
	m_State(SourceInvalid),
	m_OnByteStreamRead(this, &CFlvSource::OnByteStreamRead){

	LOG_HRESULT(hr = MFCreateEventQueue(&m_pEventQueue));
	TRACE_SOURCE((L"Source::CTOR"));
}

HRESULT CFlvSource::CreateInstance(CFlvSource** ppSource){

	TRACE_SOURCE((L"Source::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppSource == NULL ? E_POINTER : S_OK));

	CFlvSource* pSource = new (std::nothrow)CFlvSource(hr);

	IF_FAILED_RETURN(pSource == NULL ? E_OUTOFMEMORY : S_OK);

	*ppSource = pSource;
	(*ppSource)->AddRef();

	SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CFlvSource::QueryInterface(REFIID riid, void** ppv){

	TRACE_SOURCE((L"Source::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CFlvSource, IMFMediaEventGenerator),
			QITABENT(CFlvSource, IMFMediaSource),
			{ 0 }
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CFlvSource::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CFlvSource::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"Source::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

STDMETHODIMP CFlvSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

	TRACE_EVENTSOURCE((L"Source::BeginGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

	return hr;
}

STDMETHODIMP CFlvSource::EndGetEvent(IMFAsyncResult* pCallback, IMFMediaEvent** punkState){

	TRACE_EVENTSOURCE((L"Source::EndGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pCallback, punkState));

	return hr;
}

STDMETHODIMP CFlvSource::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

	TRACE_EVENTSOURCE((L"Source::GetEvent"));

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

STDMETHODIMP CFlvSource::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

	TRACE_EVENTSOURCE((L"Source::QueueEvent : %s", MFEventString(met)));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

	return hr;
}

STDMETHODIMP CFlvSource::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){

	TRACE_SOURCE((L"Source::CreatePresentationDescriptor"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppPresentationDescriptor == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pPresentationDescriptor == NULL){

		IF_FAILED_RETURN(hr = E_FAIL);
	}

	LOG_HRESULT(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

	return hr;
}

STDMETHODIMP CFlvSource::GetCharacteristics(DWORD* pdwCharacteristics){

	TRACE_SOURCE((L"Source::GetCharacteristics"));

	HRESULT hr;

	IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwCharacteristics = MFMEDIASOURCE_CAN_PAUSE;

	return hr;
}

STDMETHODIMP CFlvSource::Pause(){

	TRACE_SOURCE((L"Source::Pause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_State != SourceStarted ? MF_E_INVALID_STATE_TRANSITION : S_OK));

	if(m_pVideoStream && m_pVideoStream->IsActive()){
		IF_FAILED_RETURN(hr = m_pVideoStream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
	}
	if(m_pAudioStream && m_pAudioStream->IsActive()){
		IF_FAILED_RETURN(hr = m_pAudioStream->QueueEvent(MEStreamPaused, GUID_NULL, S_OK, NULL));
	}

	IF_FAILED_RETURN(hr = QueueEvent(MESourcePaused, GUID_NULL, S_OK, NULL));

	m_State = SourcePaused;

	return hr;
}

STDMETHODIMP CFlvSource::Shutdown(){

	TRACE_SOURCE((L"Source::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pVideoStream){
		LOG_HRESULT(m_pVideoStream->Shutdown());
	}
	if(m_pAudioStream){
		LOG_HRESULT(m_pAudioStream->Shutdown());
	}

	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pVideoStream);
	SAFE_RELEASE(m_pAudioStream);

	SAFE_RELEASE(m_pEventQueue);
	SAFE_RELEASE(m_pPresentationDescriptor);

	SAFE_RELEASE(m_pByteStream);

	ReleaseSampleList();

	m_State = SourceShutdown;

	return hr;
}

STDMETHODIMP CFlvSource::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){

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

	BOOL bQueuedStartEvent = FALSE;

	try{

		IF_FAILED_THROW(hr = CheckShutdown());

		IF_FAILED_THROW(hr = (m_State == SourceInvalid ? E_FAIL : S_OK));

		IF_FAILED_THROW(hr = ((m_pAudioStream == NULL && m_pVideoStream == NULL) ? E_POINTER : S_OK));

		//IF_FAILED_THROW(hr = m_pByteStream->SetCurrentPosition(0));
		IF_FAILED_THROW(hr = m_pByteStream->Reset());
		m_cReadBuffer.Reset();
		m_cFlvParser.Clear();
		m_llStatrTime = 0;

		// Todo check one stream selected
//IF_FAILED_THROW(hr = ValidatePresentationDescriptor(pPresentationDescriptor));

		IF_FAILED_THROW(hr = RequestData(FLV_READ_SIZE));

		IF_FAILED_THROW(hr = QueueNewStreamEvent(pPresentationDescriptor));

		var.vt = VT_I8;
		var.hVal.QuadPart = 0;

		IF_FAILED_THROW(hr = MFCreateMediaEvent(MESourceStarted, GUID_NULL, hr, &var, &pEvent));

		IF_FAILED_THROW(hr = m_pEventQueue->QueueEvent(pEvent));

		if(m_pVideoStream && m_pVideoStream->IsActive()){
			IF_FAILED_THROW(hr = m_pVideoStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));
		}

		if(m_pAudioStream && m_pAudioStream->IsActive()){
			IF_FAILED_THROW(hr = m_pAudioStream->QueueEvent(MEStreamStarted, GUID_NULL, hr, &var));
		}

		bQueuedStartEvent = TRUE;

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

STDMETHODIMP CFlvSource::Stop(){

	TRACE_SOURCE((L"Source::Stop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = CheckShutdown());

	m_State = SourceStopped;

	ReleaseSampleList();

	if(m_pVideoStream && m_pVideoStream->IsActive()){
		IF_FAILED_RETURN(hr = m_pVideoStream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
	}
	if(m_pAudioStream && m_pAudioStream->IsActive()){
		IF_FAILED_RETURN(hr = m_pAudioStream->QueueEvent(MEStreamStopped, GUID_NULL, S_OK, NULL));
	}

	LOG_HRESULT(hr = QueueEvent(MESourceStopped, GUID_NULL, S_OK, NULL));

	return hr;
}

HRESULT CFlvSource::BeginOpen(LPCWSTR pwszFile, IMFAsyncCallback* pCB, IUnknown* pState){

	TRACE_SOURCE((L"Source::BeginOpen"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pwszFile == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pCB == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (m_State != SourceInvalid ? MF_E_INVALIDREQUEST : S_OK));

	assert(m_pByteStream == NULL);

	DWORD dwCaps = 0;

	IF_FAILED_RETURN(hr = CMFByteStream::CreateInstance(&m_pByteStream));
	IF_FAILED_RETURN(hr = m_pByteStream->Initialize(pwszFile, &dwCaps));

	if((dwCaps & MFBYTESTREAM_IS_SEEKABLE) == 0){
		IF_FAILED_RETURN(hr = MF_E_BYTESTREAM_NOT_SEEKABLE);
	}

	if((dwCaps & MFBYTESTREAM_IS_READABLE) == 0){
		IF_FAILED_RETURN(hr = E_FAIL);
	}

	IF_FAILED_RETURN(hr = m_cReadBuffer.Initialize(FLV_READ_SIZE));

	IF_FAILED_RETURN(hr = MFCreateAsyncResult(NULL, pCB, pState, &m_pBeginOpenResult));

	IF_FAILED_RETURN(hr = RequestData(FLV_READ_SIZE));

	m_State = SourceOpening;

	return hr;
}

HRESULT CFlvSource::EndOpen(IMFAsyncResult* pResult){

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

HRESULT CFlvSource::RequestData(DWORD cbRequest){

	TRACE_SOURCE((L"Source::RequestData"));

	HRESULT hr;

	IF_FAILED_RETURN(hr = m_cReadBuffer.Reserve(cbRequest));

	//IF_FAILED_RETURN(hr = m_pByteStream->BeginRead(m_cReadBuffer.GetReadStartBuffer(), cbRequest, &m_OnByteStreamRead, NULL));
	LOG_HRESULT(hr = m_pByteStream->BeginRead(m_cReadBuffer.GetReadStartBuffer(), cbRequest, &m_OnByteStreamRead));

	return hr;
}

HRESULT CFlvSource::OnByteStreamRead(IMFAsyncResult* pResult){

	TRACE_SOURCE((L"Source::OnByteStreamRead"));

	AutoLock lock(m_CriticSection);

	HRESULT hr = S_OK;
	DWORD cbRead = 0;

	IUnknown* pState = NULL;

	if(m_State == SourceShutdown){
		return hr;
	}

	try{

		//LOG_HRESULT(pResult->GetState(&pState));

		IF_FAILED_THROW(hr = m_pByteStream->EndRead(pResult, &cbRead));

		if(cbRead == 0){

			m_bEOS = TRUE;

			/*if(m_pStream){

					IF_FAILED_THROW(hr = m_pStream->EndOfStream());
			}*/
		}
		else{

			// Update the end-position of the read buffer.
			IF_FAILED_THROW(hr = m_cReadBuffer.SetEndPosition(cbRead));

			// Parse the new data.
			if(m_bParsing)
				IF_FAILED_THROW(hr = ParseData());
		}
	}
	catch(HRESULT){}

	if(FAILED(hr)){
		StreamingError(hr);
	}

	SAFE_RELEASE(pState);

	return hr;
}

void CFlvSource::StreamingError(const HRESULT hr){

	TRACE_SOURCE((L"Source::StreamingError"));

	if(m_State == SourceOpening){

		CompleteOpen(hr);
	}
	else if(m_State != SourceShutdown){

		LOG_HRESULT(QueueEvent(MEError, GUID_NULL, hr, NULL));
	}
}

HRESULT CFlvSource::CompleteOpen(const HRESULT hrStatus){

	TRACE_SOURCE((L"Source::CompleteOpen"));

	HRESULT hr = S_OK;

	if(m_pBeginOpenResult){

		try{

			IF_FAILED_THROW(hr = m_pBeginOpenResult->SetStatus(hrStatus));
			IF_FAILED_THROW(hr = MFInvokeCallback(m_pBeginOpenResult));

			//m_cFlvParser.ClearInfo();
			m_bParsing = FALSE;
		}
		catch(HRESULT){}
	}

	SAFE_RELEASE(m_pBeginOpenResult);

	return hr;
}

HRESULT CFlvSource::QueueNewStreamEvent(IMFPresentationDescriptor* pPD){

	TRACE_SOURCE((L"Source::QueueNewStreamEvent"));

	assert(pPD != NULL);

	HRESULT hr = S_OK;

	IMFStreamDescriptor* pSD = NULL;
	DWORD dwStreams = 0;
	BOOL fSelected;
	BOOL fVWasSelected;

	try{

		IF_FAILED_THROW(hr = pPD->GetStreamDescriptorCount(&dwStreams));

		IF_FAILED_THROW(hr = (dwStreams == 0 ? E_UNEXPECTED : S_OK));

		for(DWORD i = 0; i < dwStreams; i++){

			IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));

			GUID majorType = GUID_NULL;
			IF_FAILED_THROW(hr = GetStreamMajorType(pSD, &majorType));

			// Duplicate code, refactor...
			if(majorType == MFMediaType_Video){

				fVWasSelected = m_pVideoStream->IsActive();
				m_pVideoStream->Activate(fSelected);

				if(fSelected){

					if(fVWasSelected){
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pVideoStream));
					}
					else{
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pVideoStream));
					}
				}
			}
			else if(majorType == MFMediaType_Audio){

				fVWasSelected = m_pAudioStream->IsActive();
				m_pAudioStream->Activate(fSelected);

				if(fSelected){

					if(fVWasSelected){
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, m_pAudioStream));
					}
					else{
						IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, m_pAudioStream));
					}
				}
			}

			SAFE_RELEASE(pSD);
		}
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSD);

	return hr;
}