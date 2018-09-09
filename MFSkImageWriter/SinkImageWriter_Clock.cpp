//----------------------------------------------------------------------------------------------
// SinkImageWriter_Clock.cpp
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

HRESULT CSkinkImageWriter::OnClockStart(MFTIME /*hnsSystemTime*/, LONGLONG llClockStartOffset){

		TRACE_SINK((L"ImageSink::OnClockStart"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pStreamImage->Start(llClockStartOffset));

		m_ClockState = MFCLOCK_STATE_RUNNING;

		return hr;
}

HRESULT CSkinkImageWriter::OnClockStop(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"ImageSink::OnClockStop"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pStreamImage->Stop());

		m_ClockState = MFCLOCK_STATE_STOPPED;

		return hr;
}

HRESULT CSkinkImageWriter::OnClockPause(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"ImageSink::OnClockPause"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pStreamImage->Pause());

		m_ClockState = MFCLOCK_STATE_PAUSED;

		return hr;
}

HRESULT CSkinkImageWriter::OnClockRestart(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"ImageSink::OnClockRestart"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pStreamImage->Restart());

		m_ClockState = MFCLOCK_STATE_RUNNING;

		return hr;
}

HRESULT CSkinkImageWriter::OnClockSetRate(MFTIME /*hnsSystemTime*/, float /*flRate*/){

		TRACE_SINK((L"ImageSink::OnClockSetRate"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		return hr;
}