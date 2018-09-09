//----------------------------------------------------------------------------------------------
// MFSkJpegHttpStreamer_Sink.cpp
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

HRESULT CMFSkJpegHttpStreamer::GetCharacteristics(DWORD* pdwCharacteristics){

	TRACE_SINK((L"JpegSink::GetCharacteristics"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_RATELESS;

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::AddStreamSink(DWORD /*dwStreamSinkIdentifier*/, IMFMediaType* /*pMediaType*/, IMFStreamSink** /*ppStreamSink*/){

	TRACE_SINK((L"JpegSink::AddStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CMFSkJpegHttpStreamer::RemoveStreamSink(DWORD /*dwStreamSinkIdentifier*/){

	TRACE_SINK((L"JpegSink::RemoveStreamSink"));
	return MF_E_STREAMSINKS_FIXED;
}

HRESULT CMFSkJpegHttpStreamer::GetStreamSinkCount(DWORD* pcStreamSinkCount){

	TRACE_SINK((L"JpegSink::GetStreamSinkCount"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pcStreamSinkCount == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*pcStreamSinkCount = 1;

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"JpegSink::GetStreamSinkByIndex"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwIndex != 0 ? MF_E_INVALIDINDEX : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pJpegStream;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink){

	TRACE_SINK((L"JpegSink::GetStreamSinkById"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
	IF_FAILED_RETURN(hr = (dwStreamSinkIdentifier != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamSink = m_pJpegStream;
	(*ppStreamSink)->AddRef();

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::SetPresentationClock(IMFPresentationClock* pPresentationClock){

	TRACE_SINK((L"JpegSink::SetPresentationClock"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pClock){
		IF_FAILED_RETURN(hr = m_pClock->RemoveClockStateSink(this));
	}

	if(pPresentationClock){
		IF_FAILED_RETURN(hr = pPresentationClock->AddClockStateSink(this));
	}

	SAFE_RELEASE(m_pClock);

	if(pPresentationClock){

		m_pClock = pPresentationClock;
		m_pClock->AddRef();
	}

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::GetPresentationClock(IMFPresentationClock** ppPresentationClock){

	TRACE_SINK((L"JpegSink::GetPresentationClock"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppPresentationClock == NULL ? E_INVALIDARG : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(m_pClock == NULL){

		hr = MF_E_NO_CLOCK;
	}
	else{

		*ppPresentationClock = m_pClock;
		(*ppPresentationClock)->AddRef();
	}

	return hr;
}

HRESULT CMFSkJpegHttpStreamer::Shutdown(){

	TRACE_SINK((L"JpegSink::Shutdown"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	//if(m_pJpegStream){
	IF_FAILED_RETURN(hr = m_pJpegStream->Shutdown());
	//}

	SAFE_RELEASE(m_pJpegStream);
	SAFE_RELEASE(m_pClock);

	m_IsShutdown = TRUE;

	return hr;
}