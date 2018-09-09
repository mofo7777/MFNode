//----------------------------------------------------------------------------------------------
// SinkVideoRenderer_Thread.cpp
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

DWORD WINAPI CSinkVideoRenderer::StaticThreadProc(LPVOID lpParameter){

	CSinkVideoRenderer* pRenderer = reinterpret_cast<CSinkVideoRenderer*>(lpParameter);

	if(pRenderer == NULL){
		return (DWORD)-1;
	}

	return pRenderer->RendererThreadProc();
}

HRESULT CSinkVideoRenderer::StartRenderer(){

	TRACE_SINK((L"SinkRenderer::StartRenderer"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_hRendererThread != NULL ? E_UNEXPECTED : S_OK));

	timeBeginPeriod(1);

	try{

		m_hThreadReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		IF_FAILED_THROW_LAST_ERROR(m_hThreadReadyEvent, E_POINTER);

		DWORD dwID = 0;
		m_hRendererThread = CreateThread(NULL, 0, StaticThreadProc, (LPVOID)this, 0, &dwID);
		IF_FAILED_THROW_LAST_ERROR(m_hRendererThread, E_POINTER);

		HANDLE hObjects[] = {m_hThreadReadyEvent, m_hRendererThread};
		DWORD dwWait = 0;

		dwWait = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);

		if(WAIT_OBJECT_0 != dwWait){

			CLOSE_EVENT_IF(m_hRendererThread);
			IF_FAILED_THROW(E_UNEXPECTED);
		}

		m_dwThreadID = dwID;
	}
	catch(HRESULT hError){ hr = hError; }

	CLOSE_EVENT_IF(m_hThreadReadyEvent);

	if(FAILED(hr)){
		timeEndPeriod(1);
	}

	return hr;
}

HRESULT CSinkVideoRenderer::StopRenderer(){

	TRACE_SINK((L"SinkRenderer::StopRenderer"));

	HRESULT hr = S_OK;

	if(m_hRendererThread == NULL){
		return hr;
	}

	PostThreadMessage(m_dwThreadID, RENDERER_EXIT, 0, 0);

	WaitForSingleObject(m_hRendererThread, INFINITE);

	CLOSE_EVENT_IF(m_hRendererThread);

	timeEndPeriod(1);
	m_dwThreadID = 0;

	return hr;
}

DWORD CSinkVideoRenderer::RendererThreadProc(){

	TRACE_SINK((L"SinkRenderer::RendererThreadProc"));

	HRESULT hr = S_OK;
	MSG     msg;
	LONG    lWait = INFINITE;
	BOOL    bExitThread = FALSE;
	DWORD   dwResult;
	BOOL    bPlaying = TRUE;

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	SetEvent(m_hThreadReadyEvent);

	while(!bExitThread){

		dwResult = MsgWaitForMultipleObjects(0, NULL, FALSE, lWait, QS_POSTMESSAGE);

		if(dwResult == WAIT_TIMEOUT){

			if(bPlaying){

				IMFSample* pSampleToRender = NULL;

				m_cMFSafeQueue.GetSampleNoAddRef(&pSampleToRender);

				if(IsTimeToPresentSample(pSampleToRender, lWait)){

					hr = m_cDirectX9Manager.OnFrameMove(pSampleToRender);
					m_cMFSafeQueue.PopSample();

					if(FAILED(hr)){
						break;
					}

					hr = m_pStreamVideoRenderer->RequestSample();

					if(FAILED(hr)){
						break;
					}
				}
			}

			hr = m_cDirectX9Manager.OnFrameRender();

			if(FAILED(hr)){
				break;
			}
		}

		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){

			switch(msg.message){

			case RENDERER_EXIT:
				bExitThread = TRUE;
				break;

			case RENDERER_PLAY:
				bPlaying = TRUE;
				lWait = 0;
				break;

			case RENDERER_PAUSE:
				bPlaying = FALSE;
				lWait = INFINITE;
				break;
			}
		}
	}

	return (SUCCEEDED(hr) ? 0 : 1);
}

BOOL CSinkVideoRenderer::IsTimeToPresentSample(IMFSample* pSample, LONG& lWait){

	BOOL bPresent = FALSE;
	lWait = 0;

	if(pSample == NULL)
		return bPresent;

	HRESULT hr = S_OK;

	LONGLONG hnsPresentationTime = 0;
	LONGLONG hnsTimeNow = 0;
	MFTIME   hnsSystemTime = 0;

	if(m_pClock){

		hr = pSample->GetSampleTime(&hnsPresentationTime);

		if(SUCCEEDED(hr)){
			hr = m_pClock->GetCorrelatedTime(0, &hnsTimeNow, &hnsSystemTime);
		}

		LONGLONG hnsDelta = hnsPresentationTime - hnsTimeNow;

		if(hnsDelta < -m_PerFrame_1_4th){

			//TRACE((L"lWait : %d", MFTimeToMilliSec(hnsDelta)));
			bPresent = TRUE;
		}
		else if(hnsDelta > (3 * m_PerFrame_1_4th)){

			lWait = MFTimeToMilliSec(hnsDelta - (3 * m_PerFrame_1_4th));

			//TRACE((L"lWait : %d", lWait));
			//bPresent = FALSE;
		}
	}

	return bPresent;
}