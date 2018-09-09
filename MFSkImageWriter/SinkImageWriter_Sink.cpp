//----------------------------------------------------------------------------------------------
// SinkImageWriter_Sink.cpp
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

HRESULT CSkinkImageWriter::GetCharacteristics(DWORD* pdwCharacteristics){

	TRACE_SINK((L"ImageSink::GetCharacteristics"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;

	return hr;
}

HRESULT CSkinkImageWriter::AddStreamSink(DWORD /*dwStreamSinkIdentifier*/, IMFMediaType* /*pMediaType*/, IMFStreamSink** /*ppStreamSink*/){

	TRACE_SINK((L"ImageSink::AddStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CSkinkImageWriter::RemoveStreamSink(DWORD /*dwStreamSinkIdentifier*/){

	TRACE_SINK((L"ImageSink::RemoveStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CSkinkImageWriter::GetStreamSinkCount(DWORD* pcStreamSinkCount){

	TRACE_SINK((L"ImageSink::GetStreamSinkCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pcStreamSinkCount == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pcStreamSinkCount = 1;

	return hr;
}

HRESULT CSkinkImageWriter::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"ImageSink::GetStreamSinkByIndex"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwIndex != 0 ? MF_E_INVALIDINDEX : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pStreamImage;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CSkinkImageWriter::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"ImageSink::GetStreamSinkById"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwStreamSinkIdentifier != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pStreamImage;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CSkinkImageWriter::SetPresentationClock(IMFPresentationClock* pPresentationClock){

	TRACE_SINK((L"ImageSink::SetPresentationClock"));

	HRESULT hr = S_OK;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pPresentationClock){
		IF_FAILED_RETURN(hr = m_pPresentationClock->RemoveClockStateSink(this));
	}

	if(pPresentationClock){
		IF_FAILED_RETURN(hr = pPresentationClock->AddClockStateSink(this));
	}

	SAFE_RELEASE(m_pPresentationClock);

	if(pPresentationClock){

		m_pPresentationClock = pPresentationClock;
		m_pPresentationClock->AddRef();
	}

	return hr;
}

HRESULT CSkinkImageWriter::GetPresentationClock(IMFPresentationClock** ppPresentationClock){

	TRACE_SINK((L"ImageSink::GetPresentationClock"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppPresentationClock == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pPresentationClock == NULL){

		hr = MF_E_NO_CLOCK;
	}
	else{

		*ppPresentationClock = m_pPresentationClock;
		(*ppPresentationClock)->AddRef();
	}

	return hr;
}

HRESULT CSkinkImageWriter::Shutdown(){

	TRACE_SINK((L"ImageSink::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	//if(m_pStreamImage){
	IF_FAILED_RETURN(hr = m_pStreamImage->Shutdown());
	//}

	SAFE_RELEASE(m_pStreamImage);
	SAFE_RELEASE(m_pPresentationClock);

	m_ClockState = MFCLOCK_STATE_INVALID;
	m_IsShutdown = TRUE;

	return hr;
}