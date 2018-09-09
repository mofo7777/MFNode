//----------------------------------------------------------------------------------------------
// KinectStream.cpp
// Copyright (C) 2012 Dumonteil David
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

CKinectStream::CKinectStream(CKinectSource* pSource, IMFStreamDescriptor* pSD, const DWORD dwSize, const int iID, HRESULT& hr) :
	m_pEventQueue(NULL),
	m_State(SourceStopped),
	//m_bActive(FALSE),
	m_pMediaBuffer(NULL),
	m_pVideoBuffer(NULL),
	m_dwVideoSize(dwSize),
	m_iID(iID),
	m_rtCurrentPosition(0){

	assert(pSource != NULL);
	assert(pSD != NULL);

	m_pKinectSource = pSource;
	m_pKinectSource->AddRef();

	m_pStreamDescriptor = pSD;
	m_pStreamDescriptor->AddRef();

	try{

		// Create the media event queue.
		IF_FAILED_THROW(hr = MFCreateEventQueue(&m_pEventQueue));

		// Create the buffer.
		// hr = MFCreateMemoryBuffer(dwVideoSize, &pMediaBuffer);
		IF_FAILED_THROW(hr = MFCreateAlignedMemoryBuffer(m_dwVideoSize, MF_16_BYTE_ALIGNMENT, &m_pMediaBuffer));

		IF_FAILED_THROW(hr = m_pMediaBuffer->SetCurrentLength(m_dwVideoSize));
	}
	catch(HRESULT){}

	//TRACE((L"CKinectStream CTOR"));
}

HRESULT CKinectStream::CreateInstance(CKinectStream** ppStream, CKinectSource* pSource, IMFStreamDescriptor* pSD, const DWORD dwSize, const int iID, HRESULT& hr){

	//TRACE((L"CKinectStream CreateInstance"));

	IF_FAILED_RETURN(hr = (ppStream == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pSource == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pSD == NULL ? E_POINTER : S_OK));

	CKinectStream* pStream = new (std::nothrow)CKinectStream(pSource, pSD, dwSize, iID, hr);

	IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

	*ppStream = pStream;
	(*ppStream)->AddRef();

	SAFE_RELEASE(pStream);

	return hr;
}

HRESULT CKinectStream::QueryInterface(REFIID iid, void** ppv){

	static const QITAB qit[] = {
			QITABENT(CKinectStream, IMFMediaEventGenerator),
			QITABENT(CKinectStream, IMFMediaStream),
			{ 0 }
	};

	return QISearch(this, qit, iid, ppv);
}

HRESULT CKinectStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

	//TRACE((L"CKinectStream BeginGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

	return hr;
}

HRESULT CKinectStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){

	//TRACE((L"CKinectStream EndGetEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

	return hr;
}

HRESULT CKinectStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){

	//TRACE((L"CKinectStream GetEvent"));

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

	if(SUCCEEDED(hr)){
		LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));
	}

	SAFE_RELEASE(pQueue);

	return hr;
}

HRESULT CKinectStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

	//TRACE((L"CKinectStream QueueEvent"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

	return hr;
}

HRESULT CKinectStream::GetMediaSource(IMFMediaSource** ppMediaSource){

	//TRACE((L"CKinectStream GetMediaSource"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppMediaSource == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	// If called after shutdown, then m_pSource is NULL. Otherwise, m_pSource should not be NULL.
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = (m_pKinectSource == NULL ? E_UNEXPECTED : S_OK));

	LOG_HRESULT(hr = m_pKinectSource->QueryInterface(IID_PPV_ARGS(ppMediaSource)));

	return hr;
}

HRESULT CKinectStream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor){

	//TRACE((L"CKinectStream GetStreamDescriptor"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamDescriptor == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pStreamDescriptor == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamDescriptor = m_pStreamDescriptor;
	(*ppStreamDescriptor)->AddRef();

	return hr;
}

HRESULT CKinectStream::RequestSample(IUnknown* pToken){

	//TRACE((L"CKinectStream RequestSample"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = (m_pKinectSource == NULL ? E_UNEXPECTED : S_OK));

	// Check if we are shut down.
	IF_FAILED_RETURN(hr = CheckShutdown());

	// Check the source is stopped. GetState does not hold the source's critical section. Safe to call.
	IF_FAILED_RETURN(hr = (m_pKinectSource->GetState() == SourceStopped ? MF_E_INVALIDREQUEST : S_OK));

	//IMFMediaSource* pSource = NULL;
	IMFSample* pSample = NULL;  // Sample to deliver.

	try{

		// Create a new audio sample.
		if(m_iID == 0){
			IF_FAILED_THROW(hr = CreateVideoKinectSample(&pSample));
		}
		else if(m_iID == 1){
			IF_FAILED_THROW(hr = CreateAudioKinectSample(&pSample));
		}
		else{
			IF_FAILED_THROW(hr = E_UNEXPECTED);
		}

		// If the caller provided a token, attach it to the sample as an attribute.
		// NOTE: If we processed sample requests asynchronously, we would
		// need to call AddRef on the token and put the token onto a FIFO
		// queue. See documenation for IMFMediaStream::RequestSample.
		if(pToken){
			IF_FAILED_THROW(hr = pSample->SetUnknown(MFSampleExtension_Token, pToken));
		}

		// If paused, queue the sample for later delivery. Otherwise, deliver the sample now.
		if(m_pKinectSource->GetState() == SourcePaused){

			// Todo
			//hr = m_sampleQueue.Queue(pSample);
		}
		else{

			IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, hr, pSample));
		}

		// Cache a pointer to the source, prior to leaving the critical section.
		//pSource = m_pKinectSource;
		//pSource->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pSample);
	//SAFE_RELEASE(pSource);

	return hr;
}

HRESULT CKinectStream::Shutdown(){

	//TRACE((L"CKinectStream Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;

	IF_FAILED_RETURN(hr = CheckShutdown());

	// Flush queued samples.
	//Flush();

	// Shut down the event queue.
	if(m_pEventQueue){
		LOG_HRESULT(m_pEventQueue->Shutdown());
	}

	SAFE_RELEASE(m_pEventQueue);
	SAFE_RELEASE(m_pKinectSource);
	SAFE_RELEASE(m_pStreamDescriptor);

	if(m_pMediaBuffer && m_pVideoBuffer){
		LOG_HRESULT(hr = m_pMediaBuffer->Unlock());
		m_pVideoBuffer = NULL;
	}

	SAFE_RELEASE(m_pMediaBuffer);
	m_dwVideoSize = 0;

	m_State = SourceShutdown;

	return S_OK;
}

HRESULT CKinectStream::CreateVideoKinectSample(IMFSample** ppSample){

	//TRACE((L"CKinectStream video"));

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pMediaBuffer == NULL ? E_POINTER : S_OK));

	IMFSample* pSample = NULL;

	LONGLONG duration = 0;

	try{

		// Get a pointer to the buffer memory.
		IF_FAILED_THROW(hr = m_pMediaBuffer->Lock(&m_pVideoBuffer, NULL, NULL));

		// Fill the buffer				
		IF_FAILED_THROW(hr = m_pKinectSource->KinectGetNextVideoFrame(m_pVideoBuffer, m_dwVideoSize, duration));

		// Unlock the buffer.
		IF_FAILED_THROW(hr = m_pMediaBuffer->Unlock());
		m_pVideoBuffer = NULL;

		// Create a new sample and add the buffer to it.
		IF_FAILED_THROW(hr = MFCreateSample(&pSample));

		IF_FAILED_THROW(hr = pSample->AddBuffer(m_pMediaBuffer));

		IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

		// Set the time stamps, duration, and sample flags.
		// m_rtCurrentPosition
		IF_FAILED_THROW(hr = pSample->SetSampleTime(0));

		IF_FAILED_THROW(hr = pSample->SetSampleDuration(duration));

		// This attribute applies to media samples. If this attribute is TRUE, it means there was
		// a discontinuity in the stream and this sample is the first to appear after the gap.
		// Todo : set the discontinuity flag.
		/*if(m_discontinuity){
				IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE));
		}*/

		// Update our current position.
		m_rtCurrentPosition += duration;

		// Give the pointer to the caller.
		*ppSample = pSample;
		(*ppSample)->AddRef();
	}
	catch(HRESULT){}

	if(m_pMediaBuffer && m_pVideoBuffer){
		LOG_HRESULT(hr = m_pMediaBuffer->Unlock());
		m_pVideoBuffer = NULL;
	}

	SAFE_RELEASE(pSample);

	return hr;
}

HRESULT CKinectStream::CreateAudioKinectSample(IMFSample** ppSample){

	//TRACE((L"CKinectStream Audio"));

	HRESULT hr = S_OK;

	IMFMediaBuffer* pBuffer = NULL;
	IMFSample* pSample = NULL;

	DWORD cbBuffer = 32000;
	BYTE* pData = NULL;

	LONGLONG duration = 0;

	try{

		// Create the buffer.
		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(cbBuffer, &pBuffer));

		// Get a pointer to the buffer memory.
		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

		// Fill the buffer
		IF_FAILED_THROW(hr = m_pKinectSource->KinectGetNextAudioFrame(pData, cbBuffer));

		// Unlock the buffer.
		IF_FAILED_THROW(hr = pBuffer->Unlock());
		pData = NULL;

		// Set the size of the valid data in the buffer.
		// For Testing
		IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(cbBuffer));

		// Create a new sample and add the buffer to it.
		IF_FAILED_THROW(hr = MFCreateSample(&pSample));

		IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

		// Set the time stamps, duration, and sample flags.
		// m_rtCurrentPosition
		IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtCurrentPosition));

		duration = cbBuffer * 10000000 / 32000;
		IF_FAILED_THROW(hr = pSample->SetSampleDuration(duration));

		// This attribute applies to media samples. If this attribute is TRUE, it means there was
		// a discontinuity in the stream and this sample is the first to appear after the gap.
		// Todo : set the discontinuity flag.
		/*if(m_discontinuity){
				IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE));
		}*/

		// Update our current position.
		m_rtCurrentPosition += duration;

		// Give the pointer to the caller.
		*ppSample = pSample;
		(*ppSample)->AddRef();
	}
	catch(HRESULT){}

	if(pData && pBuffer){
		LOG_HRESULT(hr = pBuffer->Unlock());
	}

	SAFE_RELEASE(pBuffer);
	SAFE_RELEASE(pSample);

	return hr;
}