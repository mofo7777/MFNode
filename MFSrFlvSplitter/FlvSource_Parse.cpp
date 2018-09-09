//----------------------------------------------------------------------------------------------
// FlvSource_Parse.cpp
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

HRESULT CFlvSource::ParseData(){

		TRACE_SOURCE((L"Source::ParseData"));

		HRESULT hr = S_OK;

		if(!m_cFlvParser.HasSystemHeader()){

				IF_FAILED_RETURN(hr = m_cFlvParser.ParseHeader(m_cReadBuffer.GetStartBuffer()));
				IF_FAILED_RETURN(hr = m_cReadBuffer.SetStartPosition(9));
		}

		DWORD cbAte;		
		DWORD cbNextRequest = 0;
		BOOL  bNeedMoreData = FALSE;

		while(NeedData()){
						
						cbAte = 0;

						// If parsing
						if(FAILED(hr = CheckInfo())){
								TRACE((L"FlvSource Error CheckInfo"));
								break;
						}
						
						if(m_cFlvParser.HasPacket()){

								if(FAILED(hr = ReadPayload(&cbAte, &cbNextRequest))){
										TRACE((L"FlvSource Error ReadPayload"));
										break;
								}

								/*if(hr == S_FALSE){
										bNeedMoreData = TRUE;
								}*/
						}
						else if(FAILED(hr = m_cFlvParser.ParseBytes(m_cReadBuffer.GetStartBuffer(), m_cReadBuffer.GetBufferSize(), &cbAte, &cbNextRequest))){
								
								TRACE((L"FlvSource Error ParseBytes"));
								break;
						}

						if(hr == S_FALSE){
								bNeedMoreData = TRUE;
						}

						if(FAILED(hr = m_cReadBuffer.SetStartPosition(cbAte))){

								TRACE((L"FlvSource Error SetStartPosition"));
								break;
						}

						if(bNeedMoreData){

								cbNextRequest = max(FLV_READ_SIZE, cbNextRequest);
								//TRACE((L"cbNextRequest = %d\r"), cbNextRequest);

								if(FAILED(hr = RequestData(cbNextRequest))){

										TRACE((L"FlvSource Error RequestData"));
										break;
								}

								break; 
						}
		}

		return hr;
}

HRESULT CFlvSource::CheckInfo(){
		
		HRESULT hr = S_OK;

		if(m_pVideoStream == NULL && m_cFlvParser.HasVideoInfo()){
    IF_FAILED_RETURN(hr = CreateVideoStream());
  }
  else if(m_pAudioStream == NULL && m_cFlvParser.HasAudioInfo()){
  		IF_FAILED_RETURN(hr = CreateAudioStream());
  }

		return hr;
}

BOOL CFlvSource::NeedData() const{

		TRACE_SOURCE((L"Source::NeedData"));
		
		BOOL bNeedData = FALSE;

		switch(m_State){
		
		  case SourceOpening:
						bNeedData = TRUE;

    case SourceShutdown:
				case SourceStopped:
						break;

				default:
						if(((m_pVideoStream && m_pVideoStream->IsActive()) || (m_pAudioStream && m_pAudioStream->IsActive())) && !m_bEOS)
        bNeedData = TRUE;
		}

		return bNeedData;
}

HRESULT CFlvSource::ReadPayload(DWORD* pcbAte, DWORD* pcbNextRequest){

		TRACE_SOURCE((L"Source::ReadPayload"));

		HRESULT hr = S_OK;

		DWORD cbPayloadRead = 0;
		DWORD cbPayloadUnread = 0;

		if(m_cFlvParser.PayloadSize() > m_cReadBuffer.GetBufferSize()){
				
				cbPayloadUnread = m_cFlvParser.PayloadSize() - m_cReadBuffer.GetBufferSize();
		}

		cbPayloadRead = m_cFlvParser.PayloadSize() - cbPayloadUnread;

		if(m_State == SourceOpening/* && !IsStreamActive(m_cFlvParser.GetPacketType())*/){
				
				//QWORD qwCurrentPosition = 0;

				//IF_FAILED_RETURN(hr = m_pByteStream->Seek(msoCurrent, cbPayloadUnread, MFBYTESTREAM_SEEK_FLAG_CANCEL_PENDING_IO, &qwCurrentPosition));
				IF_FAILED_RETURN(hr = m_pByteStream->Seek(cbPayloadUnread));

				*pcbAte = cbPayloadRead;

				m_cFlvParser.ClearPacket();
		}
		else if(cbPayloadUnread > 0){
				
				*pcbNextRequest = cbPayloadUnread;
				*pcbAte = 0;
				hr = S_FALSE;
		}
		else{
				
				*pcbAte = cbPayloadRead;
				m_cFlvParser.ClearPacket();
		}

		return hr;
}