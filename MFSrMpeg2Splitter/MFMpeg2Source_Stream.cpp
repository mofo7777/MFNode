//----------------------------------------------------------------------------------------------
// MFMpeg2Source_Stream.cpp
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

HRESULT CMFMpeg2Source::ValidatePresentationDescriptor(IMFPresentationDescriptor* pPD){

		TRACE_STREAMSOURCE((L"Source::ValidatePDS"));
		
		HRESULT hr;

		/*if(m_pHeader == NULL){
				return E_UNEXPECTED;
		}*/

		DWORD cStreams = 0;

		// The caller's PD must have the same number of streams as ours.
		IF_FAILED_RETURN(hr = pPD->GetStreamDescriptorCount(&cStreams));

		/*if(cStreams != m_pHeader->cStreams){
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}*/

		BOOL fSelected = FALSE;
		IMFStreamDescriptor* pSD = NULL;

		try{
				
				// The caller must select at least one stream.
				for(DWORD i = 0; i < cStreams; i++){
						
						IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));
						
						if(fSelected){
								break;
						}
						
						SAFE_RELEASE(pSD);
				}

				IF_FAILED_THROW(hr = (!fSelected ? E_INVALIDARG: S_OK));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSD);
		
		return hr;
}

HRESULT CMFMpeg2Source::SelectStreams(IMFPresentationDescriptor* pPD, const PROPVARIANT varStart){

		TRACE_STREAMSOURCE((L"Source::SelectStreams"));

		HRESULT hr = S_OK;
		BOOL fSelected = FALSE;
		BOOL fWasSelected = FALSE;
		DWORD stream_id = 0;

		IMFStreamDescriptor* pSD = NULL;
		CMFMpeg2Stream* pStream = NULL;

		m_cPendingEOS = 0;

		try{

				for(DWORD i = 0; i < m_streams.GetCount(); i++){

						IF_FAILED_THROW(hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD));

						IF_FAILED_THROW(hr = pSD->GetStreamIdentifier(&stream_id));

						pStream = GetStream((BYTE)stream_id);

						if(pStream == NULL){
								IF_FAILED_THROW(hr = E_INVALIDARG);
						}

						fWasSelected = pStream->IsActive();

						IF_FAILED_THROW(hr = pStream->Activate(fSelected));

						if(fSelected){

								m_cPendingEOS++;

								if(fWasSelected){

										IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEUpdatedStream, GUID_NULL, hr, pStream));
								}
								else{

										IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, pStream));
								}

								IF_FAILED_THROW(hr = pStream->Start(varStart));
						}

						SAFE_RELEASE(pSD);
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSD);

		return hr;
}

HRESULT CMFMpeg2Source::RequestData(DWORD cbRequest){
		
		TRACE_STREAMSOURCE((L"Source::RequestData"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = m_cReadBuffer.Reserve(cbRequest));

		LOG_HRESULT(hr = m_pByteStream->BeginRead(m_cReadBuffer.GetReadStartBuffer(), cbRequest, &m_OnByteStreamRead));

		return hr;
}

HRESULT CMFMpeg2Source::EndOfMPEGStream(){

		TRACE_STREAMSOURCE((L"Source::EndOfMPEGStream"));

		HRESULT hr = S_OK;
		
		for(DWORD i = 0; i < m_streams.GetCount(); i++){

				if(m_streams[i]->IsActive()){

						IF_FAILED_RETURN(hr = m_streams[i]->EndOfStream());
				}
		}

		return hr;
}

HRESULT CMFMpeg2Source::ParseData(){

		TRACE_STREAMSOURCE((L"Source::ParseData"));

		HRESULT hr = S_OK;

		DWORD cbNextRequest = 0;
		BOOL bNeedMoreData = FALSE;

		while(StreamsNeedData()){

				DWORD cbAte = 0;

				if(m_cParser.HasSystemHeader()){
						
						int iNumStream = m_cParser.GetSystemHeader(&m_SysHeader);

						m_cParser.ClearSysHeader();

						IF_FAILED_RETURN(hr = m_streams.Allocate(iNumStream));
				}

				// Perhaps not here the best, see below...
				if(m_cParser.IsEndOfStream()){
						
						IF_FAILED_RETURN(hr = EndOfMPEGStream());
				}
				else if(m_cParser.HasPacket()){   
						
						IF_FAILED_RETURN(hr = ReadPayload(&cbAte, &cbNextRequest));

						if(hr == S_FALSE){
								bNeedMoreData = TRUE;
						}
				}
				else{
						
						IF_FAILED_RETURN(hr = m_cParser.ParseBytes(m_cReadBuffer.GetStartBuffer(), m_cReadBuffer.GetBufferSize(), &cbAte));

						if(hr == S_FALSE){
								bNeedMoreData = TRUE;
						}
				}

				if(m_cParser.IsEndOfStream()){
						
						IF_FAILED_RETURN(hr = EndOfMPEGStream());
						break;
				}

				IF_FAILED_RETURN(hr = m_cReadBuffer.SetStartPosition(cbAte));

				if(bNeedMoreData){
						
						IF_FAILED_RETURN(hr = RequestData(max(READ_SIZE, cbNextRequest)));
						break; 
				}
		}

		if(!bNeedMoreData){				
				SAFE_RELEASE(m_pSampleRequest);
		}

		return hr;
}

BOOL CMFMpeg2Source::StreamsNeedData() const{

		TRACE_STREAMSOURCE((L"Source::StreamsNeedData"));

		BOOL bNeedData = FALSE;

		switch(m_State){
		
		  case SourceOpening:
						return TRUE;

				case SourceShutdown:
						return FALSE;

				default:
						for(DWORD i = 0; i < m_streams.GetCount(); i++){
								
								if(m_streams[i]->NeedsData()){
										
										bNeedData = TRUE;
										break;
								}
						}
						return bNeedData;
		}
}

HRESULT CMFMpeg2Source::ReadPayload(DWORD* pcbAte, DWORD* pcbNextRequest){
		
		TRACE_STREAMSOURCE((L"Source::ReadPayload"));

		HRESULT hr = S_OK;

		DWORD cbPayloadRead = 0;
		DWORD cbPayloadUnread = 0;

		if(m_cParser.PayloadSize() > m_cReadBuffer.GetBufferSize()){
				
				cbPayloadUnread = m_cParser.PayloadSize() - m_cReadBuffer.GetBufferSize();
		}

		cbPayloadRead = m_cParser.PayloadSize() - cbPayloadUnread;

		if(!IsStreamActive(m_cParser.PacketHeader())){

				IF_FAILED_RETURN(hr = m_pByteStream->Seek(cbPayloadUnread));

				*pcbAte = cbPayloadRead;

				m_cParser.ClearPacket();
		}
		else if(cbPayloadUnread > 0){

				*pcbNextRequest = cbPayloadUnread;

				*pcbAte = 0;

				hr = S_FALSE;
		}
		else{

				StreamType st = m_cParser.GetPacketType();

				if(st == StreamType_Video || st == StreamType_Audio || st == StreamType_Private1){

						//TRACE((L"PacketHeader size : %d - number : %d", m_cParser.PacketHeader().cbPayload, m_cParser.PacketHeader().number));
						IF_FAILED_RETURN(hr = DeliverPayload());
				}

				*pcbAte = cbPayloadRead;

				m_cParser.ClearPacket();
		}

		return hr;
}

HRESULT CMFMpeg2Source::DeliverPayload(){
		
		TRACE_STREAMSOURCE((L"Source::ReadPayload"));

		HRESULT hr;
		MPEG2PacketHeader packetHdr;

		packetHdr = m_cParser.PacketHeader();

		IF_FAILED_RETURN(hr = (packetHdr.cbPayload > m_cReadBuffer.GetBufferSize() ? E_UNEXPECTED : S_OK));

		if(m_State == SourceOpening){
				
				IF_FAILED_RETURN(hr = CreateStream(packetHdr));
		}

		CMFMpeg2Stream* pStream = NULL;

		pStream = GetStream(packetHdr.stream_id);
		assert(pStream != NULL);

		IMFMediaBuffer* pBuffer = NULL;
		IMFSample* pSample = NULL;
		BYTE* pData = NULL;
		DWORD dwRealSize = packetHdr.cbPayload;
		DWORD dwAte = 0;

		if(m_cParser.GetPacketType() == StreamType_Audio && m_bSkipInfo){

				if(SkipInfo(&dwAte) == S_FALSE)
						return hr;

				dwRealSize -= dwAte;
		}

		try{

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwRealSize, &pBuffer));

    IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

    CopyMemory(pData, m_cReadBuffer.GetStartBuffer() + dwAte, dwRealSize);

    IF_FAILED_THROW(hr = pBuffer->Unlock());

    IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwRealSize));

    IF_FAILED_THROW(hr = MFCreateSample(&pSample));
    IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

				if(packetHdr.bHasPTS){

						// Should be done by splitter
						LONGLONG hnsStart = packetHdr.PTS * 10000 / 90;

						IF_FAILED_THROW(hr = pSample->SetSampleTime(hnsStart));
				}

				if(packetHdr.bHasDTS){

						// Should be done by parser
						LONGLONG hnsStartDecode = packetHdr.DTS * 10000 / 90;

						IF_FAILED_THROW(hr = pSample->SetUINT64(MFSampleExtension_DecodedTimeStamp, hnsStartDecode));
				}

				IF_FAILED_THROW(hr = pStream->DeliverPayload(pSample));

				if(m_State == SourceOpening){
						
						IF_FAILED_THROW(hr = InitPresentationDescriptor());
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pBuffer);
		SAFE_RELEASE(pSample);
		
		return hr;
}

HRESULT CMFMpeg2Source::SkipInfo(DWORD* dwAte){

		BYTE* pData = m_cReadBuffer.GetStartBuffer();

		do{

				while(*pData != 0xff){

						if(*dwAte == m_cReadBuffer.GetBufferSize()){
								break;
						}
						pData++;								
						(*dwAte)++;
				}

				if(*dwAte == m_cReadBuffer.GetBufferSize()){
						break;
				}

				pData++;
				(*dwAte)++;

				if((*pData & 0xe0) == 0xe0)
						break;
		}
		while(true);

		if(*dwAte == m_cReadBuffer.GetBufferSize()){
				return S_FALSE;
		}

		(*dwAte)--;

		m_bSkipInfo = FALSE;

		return S_OK;
}

HRESULT CMFMpeg2Source::CreateStream(const MPEG2PacketHeader& packetHdr){
		
		TRACE_STREAMSOURCE((L"Source::CreateStream"));

		assert(IsStreamTypeSupported(packetHdr.type));

		if(GetStream(packetHdr.stream_id) != NULL){

				return S_OK; 
		}

		HRESULT hr = S_OK;
		DWORD dwAte = 0;
		DWORD cbHeader = 0;
		BYTE* pPayload = NULL;

		IMFMediaType* pType = NULL;

		cbHeader = packetHdr.cbPacketSize - packetHdr.cbPayload;
		pPayload = m_cReadBuffer.GetStartBuffer();

		switch(packetHdr.type){
		
		  case StreamType_Video:
						MPEG2VideoSeqHeader videoSeqHdr;
						IF_FAILED_RETURN(hr = m_cParser.ReadVideoSequenceHeader(pPayload, packetHdr.cbPayload, videoSeqHdr));
						IF_FAILED_RETURN(hr = CreateVideoMediaType(videoSeqHdr, &pType, m_cParser.IsMpeg1()));
						break;

				case StreamType_Audio:
						MPEG2AudioFrameHeader audioFrameHeader;
						IF_FAILED_RETURN(hr = m_cParser.ReadAudioFrameHeader(pPayload, packetHdr.cbPayload, audioFrameHeader, &dwAte));   
						IF_FAILED_RETURN(hr = CreateAudioMediaType(audioFrameHeader, &pType));
						break;

				case StreamType_Private1:
						WAVEFORMATEX audioAC3FrameHeader;
						IF_FAILED_RETURN(hr = m_cParser.ReadAC3AudioFrameHeader(pPayload, packetHdr.cbPayload, audioAC3FrameHeader, &dwAte));   
						IF_FAILED_RETURN(hr = CreateAC3AudioMediaType(audioAC3FrameHeader, &pType));
						break;

				default:
						IF_FAILED_RETURN(hr = E_UNEXPECTED);
		}

		assert(pType != NULL);

		IMFStreamDescriptor* pSD = NULL;
		CMFMpeg2Stream* pStream = NULL;
		IMFMediaTypeHandler* pHandler = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateStreamDescriptor(packetHdr.stream_id, 1, &pType, &pSD));

    IF_FAILED_THROW(hr = pSD->GetMediaTypeHandler(&pHandler));
    IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));

				pStream = new (std::nothrow)CMFMpeg2Stream(this, pSD, hr);
				
				IF_FAILED_THROW(hr = (pStream == NULL ? E_OUTOFMEMORY : S_OK));
				
				DWORD cStreams = m_streams.GetCount();

				IF_FAILED_THROW(hr = m_streams.SetSize(cStreams + 1));

				m_streams[cStreams] = pStream;
				m_streams[cStreams]->AddRef();

				IF_FAILED_THROW(hr = m_streamMap.Insert(packetHdr.stream_id, cStreams));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSD);
		SAFE_RELEASE(pStream);
		
		return hr;
}

HRESULT CMFMpeg2Source::InitPresentationDescriptor(){
		
		TRACE_STREAMSOURCE((L"Source::InitDescriptor"));

		HRESULT hr = S_OK;

		//assert(m_pPresentationDescriptor == NULL);
		assert(m_State == SourceOpening);
		assert(m_SysHeader.streams[0].type != StreamType_Unknown || m_SysHeader.streams[1].type != StreamType_Unknown);

		IMFStreamDescriptor** ppSD = NULL;
		DWORD dwNewStream = 0;
		
		if(m_SysHeader.streams[0].type != StreamType_Unknown)
				dwNewStream++;

		if(m_SysHeader.streams[1].type != StreamType_Unknown)
				dwNewStream++;

		if(dwNewStream > m_streams.GetCount()){

				return S_OK;
		}

		try{

				SAFE_RELEASE(m_pPresentationDescriptor);
				
				ppSD = new (std::nothrow)IMFStreamDescriptor*[dwNewStream];

				IF_FAILED_THROW(hr = (ppSD == NULL ? E_OUTOFMEMORY : S_OK));
				
				ZeroMemory(ppSD, dwNewStream * sizeof(IMFStreamDescriptor*));

				for(DWORD i = 0; i < dwNewStream; i++){

						IF_FAILED_THROW(hr =  m_streams[i]->GetStreamDescriptor(&ppSD[i]));
				}

				IF_FAILED_THROW(hr = MFCreatePresentationDescriptor(dwNewStream, ppSD, &m_pPresentationDescriptor));

    for(DWORD i = 0; i < dwNewStream; i++){
						
						GUID majorType = GUID_NULL;
						IF_FAILED_THROW(hr = GetStreamMajorType(ppSD[i], &majorType));

						if(majorType == MFMediaType_Video || majorType == MFMediaType_Audio){
								IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(i));
						}
				}

				if(dwNewStream == 2){

						MFTIME llDuration;
						LOG_HRESULT(hr = GetVideoDuration(llDuration));

						if(SUCCEEDED(hr)){
								LOG_HRESULT(m_pPresentationDescriptor->SetUINT64(MF_PD_DURATION, llDuration));
						}

						m_State = SourceStopped;

						IF_FAILED_THROW(hr = CompleteOpen(S_OK));
				}
		}
		catch(HRESULT){}

		if(ppSD){
				
				for(DWORD i = 0; i < dwNewStream; i++){
						SAFE_RELEASE(ppSD[i]);
				}
				
				delete[] ppSD;
		}
		
		return hr;
}

HRESULT CMFMpeg2Source::GetVideoDuration(MFTIME& pllDuration){
		
		TRACE_STREAMSOURCE((L"Source::GetVideoDuration"));

		// Try calculate duration by getting the first and the last PTS
		
		HRESULT hr;
		pllDuration = 0;

		IF_FAILED_RETURN(hr = m_pByteStream->Reset());
  m_cReadBuffer.Reset();
  m_cParser.Reset();

		CMFReadParam ReadParam;
		ReadParam.SetByteToRead(READ_SIZE);

		BOOL bFound = FALSE;
		REFERENCE_TIME rtBegin = 0;
		REFERENCE_TIME rtEnd = 0;

		DWORD dwAte = 0;
		BOOL bNeedMoreData = TRUE;

		while(bFound == FALSE){

				if(bNeedMoreData){

						IF_FAILED_RETURN(hr = m_cReadBuffer.Reserve(READ_SIZE));
				  ReadParam.SetDataPtr(m_cReadBuffer.GetReadStartBuffer());

				  IF_FAILED_RETURN(hr = m_pByteStream->Read(&ReadParam));
				  IF_FAILED_RETURN(hr = m_cReadBuffer.SetEndPosition(ReadParam.GetByteRead()));
						bNeedMoreData = FALSE;
				}
				else{
						ReadParam.SetDataPtr(m_cReadBuffer.GetStartBuffer());
				}

				IF_FAILED_RETURN(hr = m_cParser.ParseBytes(ReadParam.GetDataPtr(), ReadParam.GetByteRead(), &dwAte));

				if(m_cParser.IsEndOfStream()){
						
						IF_FAILED_RETURN(hr = E_FAIL);
				}
				else if(m_cParser.HasPacket()){

						MPEG2PacketHeader packetHdr = m_cParser.PacketHeader();

						if(packetHdr.type == StreamType_Video || packetHdr.type == StreamType_Audio){

								if(packetHdr.bHasPTS){
										rtBegin = packetHdr.PTS;
								  bFound = TRUE;
						    break;
								}
						}

						dwAte = packetHdr.cbPacketSize;
						m_cParser.ClearPacket();
				}

				if(dwAte > m_cReadBuffer.GetBufferSize()){
						dwAte = m_cReadBuffer.GetBufferSize();
						hr = S_FALSE;
				}

				if(hr == S_FALSE){
						bNeedMoreData = TRUE;
				}

				IF_FAILED_RETURN(hr = m_cReadBuffer.SetStartPosition(dwAte));
		}

		IF_FAILED_RETURN(hr = m_pByteStream->Reset());
  m_cReadBuffer.Reset();

		bFound = FALSE;
		dwAte = 0;
		bNeedMoreData = TRUE;
		DWORD dwSeek = READ_SIZE;
		BOOL bContinueSearch = FALSE;

		while(bFound == FALSE){

				if(bNeedMoreData){

						IF_FAILED_RETURN(hr = m_cReadBuffer.Reserve(READ_SIZE));
				  ReadParam.SetDataPtr(m_cReadBuffer.GetReadStartBuffer());

						IF_FAILED_RETURN(hr = m_pByteStream->SeekEnd(dwSeek));
				  IF_FAILED_RETURN(hr = m_pByteStream->Read(&ReadParam));
				  IF_FAILED_RETURN(hr = m_cReadBuffer.SetEndPosition(ReadParam.GetByteRead()));
						bNeedMoreData = FALSE;
				}
				else{
						ReadParam.SetDataPtr(m_cReadBuffer.GetStartBuffer());
				}

				IF_FAILED_RETURN(hr = m_cParser.ParseBytes(ReadParam.GetDataPtr(), ReadParam.GetByteRead(), &dwAte));

				if(m_cParser.IsEndOfStream()){

						if(bContinueSearch){
								bFound = TRUE;
								break;
						}

						m_cParser.ResetEOS();
						hr = S_FALSE;
				}
				else if(m_cParser.HasPacket()){

						MPEG2PacketHeader packetHdr = m_cParser.PacketHeader();

						if(packetHdr.type == StreamType_Video || packetHdr.type == StreamType_Audio){

								if(packetHdr.bHasPTS){
										rtEnd = packetHdr.PTS;
										bContinueSearch = TRUE;
								}
						}

						dwAte = packetHdr.cbPacketSize;
						m_cParser.ClearPacket();
				}

				if(dwAte > m_cReadBuffer.GetBufferSize()){
						dwAte = m_cReadBuffer.GetBufferSize();
						hr = S_FALSE;
				}

				if(hr == S_FALSE){

						if(bContinueSearch){
								bFound = TRUE;
								break;
						}

						dwSeek += (READ_SIZE / 2);
						bNeedMoreData = TRUE;
				}

				IF_FAILED_RETURN(hr = m_cReadBuffer.SetStartPosition(dwAte));
		}

		if(bFound){

				pllDuration = (rtEnd - rtBegin) * 10000 / 90;
				MFTimeString(pllDuration);
		}
		else{
				hr = E_FAIL;
		}

		return hr;
}