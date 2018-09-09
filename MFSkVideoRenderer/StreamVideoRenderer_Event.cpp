//----------------------------------------------------------------------------------------------
// StreamVideoRenderer_Event.cpp
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

HRESULT CStreamVideoRenderer::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){
		
		TRACE_STREAM((L"StreamRenderer::GetEvent"));

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

HRESULT CStreamVideoRenderer::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

		TRACE_STREAM((L"StreamRenderer::BeginGetEvent"));

		HRESULT hr;

  AutoLock lock(m_CriticSection);
		
		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

HRESULT CStreamVideoRenderer::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){

		TRACE_STREAM((L"StreamRenderer::EndGetEvent"));

		HRESULT hr;

  AutoLock lock(m_CriticSection);
		
		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

		return hr;
}

HRESULT CStreamVideoRenderer::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

		TRACE_STREAM((L"StreamRenderer::QueueEvent : %s", MFEventString(met)));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}