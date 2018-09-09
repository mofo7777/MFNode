//----------------------------------------------------------------------------------------------
// SinkDxva2Renderer_Clock.cpp
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

HRESULT CSinkDxva2Renderer::OnClockStart(MFTIME /*hnsSystemTime*/, LONGLONG llClockStartOffset){

	TRACE_SINK((L"SinkRenderer::OnClockStart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	UINT32 uiWidth;
	UINT32 uiHeight;
	UINT32 uiStride;
	D3DFORMAT VideoFmt;

	IF_FAILED_RETURN(hr = GetVideoAttribute(&uiWidth, &uiHeight, &uiStride, &VideoFmt));

	IF_FAILED_RETURN(hr = m_cDxva2Manager.InitDxva2(uiWidth, uiHeight, uiStride, VideoFmt));

	IF_FAILED_RETURN(hr = StartRenderer());

	PostThreadMessage(m_dwThreadID, RENDERER_PLAY, 0, 0);

	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->Start(llClockStartOffset));

	return hr;
}

HRESULT CSinkDxva2Renderer::OnClockStop(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockStop"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->Stop());

	LOG_HRESULT(StopRenderer());

	m_cDxva2Manager.ReleaseDxva2();

	m_cMFSafeQueue.ReleaseQueue();

	return hr;
}

HRESULT CSinkDxva2Renderer::OnClockPause(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockPause"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	PostThreadMessage(m_dwThreadID, RENDERER_PAUSE, 0, 0);

	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->Pause());

	return hr;
}

HRESULT CSinkDxva2Renderer::OnClockRestart(MFTIME /*hnsSystemTime*/){

	TRACE_SINK((L"SinkRenderer::OnClockRestart"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->Restart());

	PostThreadMessage(m_dwThreadID, RENDERER_PLAY, 0, 0);

	return hr;
}

HRESULT CSinkDxva2Renderer::OnClockSetRate(MFTIME /*hnsSystemTime*/, float /*flRate*/){

	TRACE_SINK((L"SinkRenderer::OnClockSetRate"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	return hr;
}