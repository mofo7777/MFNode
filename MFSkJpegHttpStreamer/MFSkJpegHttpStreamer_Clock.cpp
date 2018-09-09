//----------------------------------------------------------------------------------------------
// MFSkJpegHttpStreamer_Clock.cpp
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

HRESULT CMFSkJpegHttpStreamer::OnClockStart(MFTIME /*hnsSystemTime*/, LONGLONG llClockStartOffset){

		TRACE_SINK((L"JpegSink::OnClockStart"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pJpegStream->Start(llClockStartOffset));

		return hr;
}

HRESULT CMFSkJpegHttpStreamer::OnClockStop(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"JpegSink::OnClockStop"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pJpegStream->Stop());

		return hr;
}

HRESULT CMFSkJpegHttpStreamer::OnClockPause(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"JpegSink::OnClockPause"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pJpegStream->Pause());

		return hr;
}

HRESULT CMFSkJpegHttpStreamer::OnClockRestart(MFTIME /*hnsSystemTime*/){

		TRACE_SINK((L"JpegSink::OnClockRestart"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pJpegStream->Restart());

		return hr;
}

HRESULT CMFSkJpegHttpStreamer::OnClockSetRate(MFTIME /*hnsSystemTime*/, float /*flRate*/){

		TRACE_SINK((L"JpegSink::OnClockSetRate"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		return hr;
}