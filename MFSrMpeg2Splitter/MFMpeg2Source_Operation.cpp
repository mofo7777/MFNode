//----------------------------------------------------------------------------------------------
// MFMpeg2Source_Operation.cpp
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

HRESULT CMFMpeg2Source::DoStart(CMFStartOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::DoStart"));

		assert(pOp->Op() == CMFStartOperation::OP_START);

		IMFPresentationDescriptor* pPD = NULL;
		IMFMediaEvent* pEvent = NULL;

		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = BeginAsyncOp(pOp));
    IF_FAILED_THROW(hr = pOp->GetPresentationDescriptor(&pPD));

				IF_FAILED_THROW(hr = SelectStreams(pPD, pOp->Data()));

    m_State = SourceStarted;

    IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, S_OK, &pOp->Data()));
		}
		catch(HRESULT){}

		if(FAILED(hr)){

				LOG_HRESULT(m_pEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, hr, NULL));
		}

		LOG_HRESULT(CompleteAsyncOp(pOp));

		SAFE_RELEASE(pEvent);
		SAFE_RELEASE(pPD);
		
		return hr;
}

HRESULT CMFMpeg2Source::DoStop(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::DoStop"));

		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = BeginAsyncOp(pOp));

				for(DWORD i = 0; i < m_streams.GetCount(); i++){
						
						if(m_streams[i]->IsActive()){

								IF_FAILED_THROW(hr = m_streams[i]->Stop());
						}
				}

				IF_FAILED_THROW(hr = m_pByteStream->Seek(0));

    ++m_cRestartCounter;

				SAFE_RELEASE(m_pSampleRequest);

				m_State = SourceStopped;
		}
		catch(HRESULT){}

		LOG_HRESULT(m_pEventQueue->QueueEventParamVar(MESourceStopped, GUID_NULL, hr, NULL));

		LOG_HRESULT(CompleteAsyncOp(pOp));

		return hr;
}

HRESULT CMFMpeg2Source::DoPause(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::DoPause"));

		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = BeginAsyncOp(pOp));

				if(m_State != SourceStarted){
						IF_FAILED_THROW(hr = MF_E_INVALID_STATE_TRANSITION);
				}

    for(DWORD i = 0; i < m_streams.GetCount(); i++){

						if(m_streams[i]->IsActive()){

								IF_FAILED_THROW(hr = m_streams[i]->Pause());
						}
				}

				m_State = SourcePaused;
		}
		catch(HRESULT){}

		LOG_HRESULT(m_pEventQueue->QueueEventParamVar(MESourcePaused, GUID_NULL, hr, NULL));

		LOG_HRESULT(CompleteAsyncOp(pOp));

		return hr;
}

HRESULT CMFMpeg2Source::OnStreamRequestSample(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::OnStreamRequestSample"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = BeginAsyncOp(pOp));

		if(m_pSampleRequest == NULL){

				PROPVARIANT var;
				var.vt = VT_UI4;
				var.ulVal = m_cRestartCounter;

				IF_FAILED_RETURN(hr = pOp->SetData(var));

				m_pSampleRequest = pOp;
				m_pSampleRequest->AddRef();

				ParseData();
		}

		LOG_HRESULT(CompleteAsyncOp(pOp));

		return hr;
}

HRESULT CMFMpeg2Source::OnEndOfStream(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::OnEndOfStream"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = BeginAsyncOp(pOp));

		--m_cPendingEOS;
		
		if(m_cPendingEOS == 0){

				LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(MEEndOfPresentation, GUID_NULL, S_OK, NULL));
		}

		LOG_HRESULT(CompleteAsyncOp(pOp));

		return hr;
}

HRESULT CMFMpeg2Source::BeginAsyncOp(CMFSourceOperation* pOp){

		TRACE_OPSOURCE((L"Source::BeginAsyncOp"));
		
		HRESULT hr = S_OK;

		if(pOp == NULL || m_pCurrentOp != NULL){
				
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		m_pCurrentOp = pOp;
		m_pCurrentOp->AddRef();

		return hr;
}

HRESULT CMFMpeg2Source::CompleteAsyncOp(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::CompleteAsyncOp"));
		
		HRESULT hr = S_OK;

		if(pOp == NULL || m_pCurrentOp == NULL){
				
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(m_pCurrentOp != pOp){
				
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		SAFE_RELEASE(m_pCurrentOp);

		LOG_HRESULT(hr = ProcessQueue());

		return hr;
}

HRESULT CMFMpeg2Source::DispatchOperation(CMFSourceOperation* pOp){
		
		TRACE_OPSOURCE((L"Source::DispatchOperation"));

		AutoLock lock(m_CriticSection);

		HRESULT hr = S_OK;

		if(m_State == SourceShutdown){				
				return hr;
		}

		switch(pOp->Op()){

				case CMFSourceOperation::OP_START:
						hr = DoStart((CMFStartOperation*)pOp);
						break;

				case CMFSourceOperation::OP_STOP:
						hr = DoStop(pOp);
						break;

				case CMFSourceOperation::OP_PAUSE:
						hr = DoPause(pOp);
						break;

				case CMFSourceOperation::OP_REQUEST_DATA:
						hr = OnStreamRequestSample(pOp);
						break;

				case CMFSourceOperation::OP_END_OF_STREAM:
						hr = OnEndOfStream(pOp);
						break;

				default:
						hr = E_UNEXPECTED;
		}

		if(FAILED(hr)){
				StreamingError(hr);
		}
		
		return hr;
}

HRESULT CMFMpeg2Source::ValidateOperation(CMFSourceOperation*){
		
		TRACE_OPSOURCE((L"Source::ValidateOperation"));
		
		HRESULT hr = S_OK;

		if(m_pCurrentOp != NULL){

				IF_FAILED_RETURN(hr = MF_E_NOTACCEPTING);
		}
		
		return hr;
}