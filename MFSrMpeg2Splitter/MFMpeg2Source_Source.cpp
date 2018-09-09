//----------------------------------------------------------------------------------------------
// MFMpeg2Source_Source.cpp
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

HRESULT CMFMpeg2Source::CreatePresentationDescriptor(IMFPresentationDescriptor** ppPresentationDescriptor){
		
		TRACE_SOURCE((L"Source::CreatePresentationDescriptor"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppPresentationDescriptor == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
		IF_FAILED_RETURN(hr = IsInitialized());

		IF_FAILED_RETURN(hr = (m_pPresentationDescriptor == NULL ? MF_E_NOT_INITIALIZED : S_OK));

		IF_FAILED_RETURN(hr = m_pPresentationDescriptor->Clone(ppPresentationDescriptor));

		return hr;
}

HRESULT CMFMpeg2Source::GetCharacteristics(DWORD* pdwCharacteristics){
		
		TRACE_SOURCE((L"Source::GetCharacteristics"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwCharacteristics == NULL ? E_POINTER : S_OK));
		
		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pdwCharacteristics =  MFMEDIASOURCE_CAN_PAUSE | MFMEDIASOURCE_CAN_SEEK;

		return hr;
}

HRESULT CMFMpeg2Source::Pause(){
		
		TRACE_SOURCE((L"Source::Pause"));

		HRESULT hr;
		
		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = QueueAsyncOperation(CMFStartOperation::OP_PAUSE));

		return hr;
}

HRESULT CMFMpeg2Source::Shutdown(){
		
		TRACE_SOURCE((L"Source::Shutdown"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		for(DWORD i = 0; i < m_streams.GetCount(); i++){

				LOG_HRESULT(m_streams[i]->Shutdown());
		}

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		for(DWORD i = 0; i < m_streams.GetCount(); i++){
				SAFE_RELEASE(m_streams[i]);
		}
		
		m_streams.SetSize(0);
		m_streamMap.Clear();

		SAFE_RELEASE(m_pEventQueue);
		SAFE_RELEASE(m_pPresentationDescriptor);
		SAFE_RELEASE(m_pBeginOpenResult);

		if(m_pByteStream){
				
				m_pByteStream->Close();
				SAFE_RELEASE(m_pByteStream);
		}

		SAFE_RELEASE(m_pCurrentOp);

		m_State = SourceShutdown;

		return hr;
}

HRESULT CMFMpeg2Source::Start(IMFPresentationDescriptor* pPresentationDescriptor, const GUID* pguidTimeFormat, const PROPVARIANT* pvarStartPosition){
		
		TRACE_SOURCE((L"Source::Start"));

		HRESULT hr;
		
		IF_FAILED_RETURN(hr = (pvarStartPosition == NULL ? E_INVALIDARG : S_OK));
		IF_FAILED_RETURN(hr = (pPresentationDescriptor == NULL ? E_INVALIDARG : S_OK));

		IF_FAILED_RETURN(hr = ((pguidTimeFormat != NULL) && (*pguidTimeFormat != GUID_NULL) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		IF_FAILED_RETURN(hr = ((pvarStartPosition->vt != VT_I8) && (pvarStartPosition->vt != VT_EMPTY) ? MF_E_UNSUPPORTED_TIME_FORMAT : S_OK));

		if(pvarStartPosition->vt == VT_I8){

				IF_FAILED_RETURN(hr = ((m_State != SourceStopped) || (pvarStartPosition->hVal.QuadPart != 0) ? MF_E_INVALIDREQUEST : S_OK));
		}

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
		IF_FAILED_RETURN(hr = IsInitialized());
		IF_FAILED_RETURN(hr = ValidatePresentationDescriptor(pPresentationDescriptor));

		CMFSourceOperation* pAsyncOp = NULL;

		try{

				if(m_pByteStream->IsInitialized() == FALSE){

						IF_FAILED_THROW(hr = m_pByteStream->Start());
				}

				if(m_State == SourceStopped){
						
						IF_FAILED_THROW(hr = m_pByteStream->Reset());
						m_bSkipInfo = TRUE;
						m_cReadBuffer.Reset();
						m_cParser.Reset();

						for(DWORD i = 0; i < m_streams.GetCount(); i++){

								m_streams[i]->ResetSample();
						}
				}
				
				IF_FAILED_THROW(hr = CMFSourceOperation::CreateStartOp(pPresentationDescriptor, &pAsyncOp));

    IF_FAILED_THROW(hr = pAsyncOp->SetData(*pvarStartPosition));

    IF_FAILED_THROW(hr = QueueOperation(pAsyncOp));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pAsyncOp);
		
		return hr;
}

HRESULT CMFMpeg2Source::Stop(){

		TRACE_SOURCE((L"Source::Stop"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());
		IF_FAILED_RETURN(hr = IsInitialized());

		LOG_HRESULT(hr = QueueAsyncOperation(CMFSourceOperation::OP_STOP));

		return hr;
}