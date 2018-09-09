//----------------------------------------------------------------------------------------------
// FlvSource_Sample.cpp
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

HRESULT CFlvSource::GetSample(IMFSample** ppSample, const DWORD dwID){

	TRACE_SOURCE((L"Source::GetSample"));

	HRESULT hr;

	// Should never occur
	IF_FAILED_RETURN(hr = (ppSample == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	if(GetSampleFromList(ppSample, dwID))
		return hr;

	if(!m_cFlvParser.HasSystemHeader()){

		IF_FAILED_RETURN(hr = m_cFlvParser.ParseHeader(m_cReadBuffer.GetStartBuffer()));
		IF_FAILED_RETURN(hr = m_cReadBuffer.SetStartPosition(9));
	}

	DWORD cbAte;
	DWORD cbNextRequest = 0;
	BOOL  bNeedMoreData = FALSE;
	ULONG ulRead;

	while(NeedData()){

		cbAte = 0;
		cbNextRequest = 0;

		if(m_cFlvParser.HasPacket()){

			if(FAILED(hr = PrepareSample(ppSample, dwID))){
				TRACE((L"FlvSample Error PrepareSample"));
				break;
			}

			cbAte = m_cFlvParser.PayloadSize();
		}
		else if(FAILED(hr = m_cFlvParser.ParseBytesStreaming(m_cReadBuffer.GetStartBuffer(), m_cReadBuffer.GetBufferSize(), &cbAte, &cbNextRequest))){

			TRACE((L"FlvSample Error ParseBytes"));
			break;
		}

		if(hr == S_FALSE){
			bNeedMoreData = TRUE;
		}

		if(FAILED(hr = m_cReadBuffer.SetStartPosition(cbAte))){

			TRACE((L"FlvSample Error SetEndPosition"));
			break;
		}

		if(*ppSample != NULL)
			break;

		if(bNeedMoreData){

			bNeedMoreData = FALSE;
			cbNextRequest = max(FLV_READ_SIZE, cbNextRequest);

			if(FAILED(hr = m_cReadBuffer.Reserve(cbNextRequest))){

				TRACE((L"FlvSample Error Reserve"));
				break;
			}

			if(FAILED(hr = m_pByteStream->Read(m_cReadBuffer.GetReadStartBuffer(), cbNextRequest, &ulRead))){

				LOG_HRESULT(hr);
				TRACE((L"FlvSample Error Read : cbNextRequest = %d, ulRead = %d, DataSize = %d", cbNextRequest, ulRead, m_cReadBuffer.GetBufferSize()));
				break;
			}

			if(ulRead == 0){

				m_bEOS = TRUE;
				break;
			}

			if(FAILED(hr = m_cReadBuffer.SetEndPosition(ulRead))){

				TRACE((L"FlvSample Error SetEndPosition"));
				break;
			}
		}
	}

	return hr;
}

BOOL CFlvSource::GetSampleFromList(IMFSample** ppSample, const DWORD dwID){

	if(dwID == FLV_VIDEO_ID){

		if(!m_qVideoSample.empty()){

			*ppSample = m_qVideoSample.front();
			//(*ppSample)->AddRef();
			m_qVideoSample.pop();
			return TRUE;
		}
	}
	else if(dwID == FLV_AUDIO_ID){

		if(!m_qAudioSample.empty()){

			*ppSample = m_qAudioSample.front();
			//(*ppSample)->AddRef();
			m_qAudioSample.pop();
			return TRUE;
		}
	}

	return FALSE;
}

HRESULT CFlvSource::PrepareSample(IMFSample** ppSample, const DWORD dwID){

	TRACE_SOURCE((L"Source::PrepareSample"));

	HRESULT hr;

	//TRACE((L"PrepareSample : PayloadSize = %d, DataSize = %d", m_cFlvParser.PayloadSize(), m_cReadBuffer.GetBufferSize()));

	IF_FAILED_RETURN(hr = (m_cFlvParser.PayloadSize() <= m_cReadBuffer.GetBufferSize() ? S_OK : E_FAIL));

	BYTE btPacket = m_cFlvParser.GetPacketType();

	if(IsStreamActive(btPacket)){

		IMFSample* pSample = NULL;

		try{

			if(btPacket == FLV_TAG_VIDEO){

				IF_FAILED_THROW(hr = CreateVideoSample(&pSample));

				if(dwID == FLV_VIDEO_ID){

					*ppSample = pSample;
					//(*ppSample)->AddRef();
				}
				else{

					//pSample->AddRef();
					m_qVideoSample.push(pSample);
				}
			}
			else if(btPacket == FLV_TAG_AUDIO){

				IF_FAILED_THROW(hr = CreateAudioSample(&pSample));

				if(dwID == FLV_AUDIO_ID){

					*ppSample = pSample;
					//(*ppSample)->AddRef();
				}
				else{

					//pSample->AddRef();
					m_qAudioSample.push(pSample);
				}
			}
			/*else{
					IF_FAILED_THROW(hr = E_FAIL);
			}*/
		}
		catch(HRESULT){}

		//SAFE_RELEASE(pSample);
	}

	m_cFlvParser.ClearPacket();

	return hr;
}

HRESULT CFlvSource::CreateVideoSample(IMFSample** ppSample){

	TRACE_SOURCE((L"Source::CreateVideoSample"));

	HRESULT hr = S_OK;
	IMFSample* pSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;
	BYTE* pData = NULL;

	DWORD dwSampleSize = m_cFlvParser.PayloadSize();
	DWORD dwNaluSize = 0;

	if(m_cFlvParser.IsKeyFrame() && m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_AVC){

		FLV_VIDEO_HEADER* pFlvVideoHeader = m_cFlvParser.GetVideoTag();
		dwNaluSize = pFlvVideoHeader->wSequenceParameterSetLength;
		dwNaluSize += pFlvVideoHeader->wPictureParameterSetLength;
		dwNaluSize += 6;
		dwSampleSize += dwNaluSize;
	}
	else if(m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_H263){
		dwSampleSize++;
	}

	try{

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwSampleSize, &pBuffer));

		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

		if(m_cFlvParser.IsKeyFrame() && m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_AVC){
			CopyMemory(pData, m_cFlvParser.GetVideoRecord(), dwNaluSize);
			pData += dwNaluSize;
		}

		// With H263, should be dwSampleSize. But H263 doesn't work for me (no codec).
		CopyMemory(pData, m_cReadBuffer.GetStartBuffer(), m_cFlvParser.PayloadSize());

		IF_FAILED_THROW(hr = pBuffer->Unlock());
		IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwSampleSize));

		IF_FAILED_THROW(hr = MFCreateSample(&pSample));
		IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

		if(m_cFlvParser.IsKeyFrame())
			IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

		// Or if first frame...
		if(m_llStatrTime == 0)
			IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE));

		LONGLONG llLast = m_llStatrTime;
		m_llStatrTime = 10000i64 * (m_cFlvParser.GetTimeStamp() + m_cFlvParser.GetComposition());

		IF_FAILED_THROW(hr = pSample->SetSampleTime(m_llStatrTime));
		IF_FAILED_THROW(hr = pSample->SetSampleDuration(m_llStatrTime - llLast));

		//TRACE((L"rtStart %d\r", m_llStatrTime));

		*ppSample = pSample;
		(*ppSample)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pBuffer);
	SAFE_RELEASE(pSample);

	return hr;
}

HRESULT CFlvSource::CreateAudioSample(IMFSample** ppSample){

	TRACE_SOURCE((L"Source::CreateAudioSample"));

	HRESULT hr = S_OK;
	IMFSample* pSample = NULL;
	IMFMediaBuffer* pBuffer = NULL;
	BYTE* pData = NULL;

	DWORD dwSampleSize = m_cFlvParser.PayloadSize();

	try{

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(dwSampleSize, &pBuffer));

		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

		CopyMemory(pData, m_cReadBuffer.GetStartBuffer(), dwSampleSize);

		IF_FAILED_THROW(hr = pBuffer->Unlock());

		IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(dwSampleSize));

		IF_FAILED_THROW(hr = MFCreateSample(&pSample));
		IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

		LONGLONG llLast = m_llStatrTime;
		m_llStatrTime = 10000i64 * (m_cFlvParser.GetTimeStamp() + m_cFlvParser.GetComposition());

		IF_FAILED_THROW(hr = pSample->SetSampleTime(m_llStatrTime));
		IF_FAILED_THROW(hr = pSample->SetSampleDuration(m_llStatrTime - llLast));

		//TRACE((L"rtStart %d\r", m_llStatrTime));

		*ppSample = pSample;
		(*ppSample)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pBuffer);
	SAFE_RELEASE(pSample);

	return hr;
}