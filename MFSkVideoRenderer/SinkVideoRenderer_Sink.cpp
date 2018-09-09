//----------------------------------------------------------------------------------------------
// SinkVideoRenderer_Sink.cpp
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

HRESULT CSinkVideoRenderer::GetCharacteristics(DWORD* pdwCharacteristics){

		TRACE_SINK((L"SinkRenderer::GetCharacteristics"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		// MEDIASINK_REQUIRE_REFERENCE_MEDIATYPE pour plus tard, plusieurs streams
		*pdwCharacteristics = MEDIASINK_FIXED_STREAMS | MEDIASINK_CAN_PREROLL;

		return hr;
}

HRESULT CSinkVideoRenderer::AddStreamSink(DWORD /*dwStreamSinkIdentifier*/, IMFMediaType* /*pMediaType*/, IMFStreamSink** /*ppStreamSink*/){

		TRACE_SINK((L"SinkRenderer::AddStreamSink"));
		return MF_E_STREAMSINKS_FIXED;
}

HRESULT CSinkVideoRenderer::RemoveStreamSink(DWORD /*dwStreamSinkIdentifier*/){

		TRACE_SINK((L"SinkRenderer::RemoveStreamSink"));
		return MF_E_STREAMSINKS_FIXED;
}

HRESULT CSinkVideoRenderer::GetStreamSinkCount(DWORD* pcStreamSinkCount){

		TRACE_SINK((L"SinkRenderer::GetStreamSinkCount"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pcStreamSinkCount == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pcStreamSinkCount = 1;

		return hr;
}

HRESULT CSinkVideoRenderer::GetStreamSinkByIndex(DWORD dwIndex, IMFStreamSink** ppStreamSink){

		TRACE_SINK((L"SinkRenderer::GetStreamSinkByIndex"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (dwIndex != 0 ? MF_E_INVALIDINDEX : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*ppStreamSink = m_pStreamVideoRenderer;
		(*ppStreamSink)->AddRef();

		return hr;
}

HRESULT CSinkVideoRenderer::GetStreamSinkById(DWORD dwStreamSinkIdentifier, IMFStreamSink** ppStreamSink){

		TRACE_SINK((L"SinkRenderer::GetStreamSinkById"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppStreamSink == NULL ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (dwStreamSinkIdentifier != 0 ? MF_E_INVALIDSTREAMNUMBER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*ppStreamSink = m_pStreamVideoRenderer;
		(*ppStreamSink)->AddRef();

		return hr;
}

HRESULT CSinkVideoRenderer::SetPresentationClock(IMFPresentationClock* pPresentationClock){

		TRACE_SINK((L"SinkRenderer::SetPresentationClock"));
		
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

HRESULT CSinkVideoRenderer::GetPresentationClock(IMFPresentationClock** ppPresentationClock){

		TRACE_SINK((L"SinkRenderer::GetPresentationClock"));
		
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

HRESULT CSinkVideoRenderer::Shutdown(){

		TRACE_SINK((L"SinkRenderer::Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		LOG_HRESULT(StopRenderer());

		m_cDirectX9Manager.ReleaseDirectX9();

		if(m_pStreamVideoRenderer){
				IF_FAILED_RETURN(hr = m_pStreamVideoRenderer->Shutdown());
				SAFE_RELEASE(m_pStreamVideoRenderer);
		}

		m_cMFSafeQueue.ReleaseQueue();

		SAFE_RELEASE(m_pClock);

		m_bShutdown = TRUE;

		return hr;
}