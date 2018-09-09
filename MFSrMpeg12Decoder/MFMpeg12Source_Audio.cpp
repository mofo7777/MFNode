//----------------------------------------------------------------------------------------------
// MFMpeg12Source_Audio.cpp
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

HRESULT CMFMpeg12Source::ReadAudioHeader(Mpeg12AudioFrameHeader& pMpeg12Header){

		TRACE_SOURCE((L"Source::ReadAudioHeader"));
		
		HRESULT hr = S_OK;

		const BYTE* pTmp = m_cMFBuffer.GetStartBuffer();
		
		// Todo check end of payload
		do{

				while(*pTmp != 0xff)
						pTmp++;

				pTmp++;

				if((*pTmp & 0xe0) == 0xe0)
						break;
		}
		while(true);

		pMpeg12Header.btVersion = (*pTmp & 0x18) >> 3;
		pMpeg12Header.btLayer = (*pTmp & 0x06) >> 1;
		pMpeg12Header.bProtection = *pTmp++ & 0x01;
		pMpeg12Header.btBitrateIndex = (*pTmp & 0xf0) >> 4;
		pMpeg12Header.btSamplingIndex = (*pTmp & 0x0C) >> 2;
		pMpeg12Header.bPadding = *pTmp & 0x02;
		pMpeg12Header.bPrivate = *pTmp++ & 0x01;
		pMpeg12Header.btChannelMode = (*pTmp & 0xC0) >> 6;
		pMpeg12Header.btModeExtension = (*pTmp & 0x30) >> 4;
		pMpeg12Header.bCopyright = *pTmp & 0x08;
		pMpeg12Header.bOriginal = *pTmp & 0x04;
		pMpeg12Header.btEmphasis = *pTmp & 0x03;

		// Log header to debug

		if(pMpeg12Header.btVersion == 0x01){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(pMpeg12Header.btLayer == 0x00){
				IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(pMpeg12Header.btLayer == 0x01){
				//pMpeg12Header.Layer = Mpeg12_Audio_Layer3;
				IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(pMpeg12Header.btLayer == 0x02){
				pMpeg12Header.Layer = Mpeg12_Audio_Layer2;
		}
		else{
				pMpeg12Header.Layer = Mpeg12_Audio_Layer1;
		}

		if(pMpeg12Header.btSamplingIndex == 0x03){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(pMpeg12Header.btVersion == 0x03){
				IF_FAILED_RETURN(hr = GetAudioBitRateMpeg1(pMpeg12Header.Layer, pMpeg12Header.btBitrateIndex, &pMpeg12Header.uiBitrate));
		}
		else{
				IF_FAILED_RETURN(hr = GetAudioBitRateMpeg2(pMpeg12Header.Layer, pMpeg12Header.btBitrateIndex, &pMpeg12Header.uiBitrate));
		}

		IF_FAILED_RETURN(hr = GetSamplingFrequency(pMpeg12Header.btVersion, pMpeg12Header.btSamplingIndex, &pMpeg12Header.uiSamplePerSec));
		
		if(pMpeg12Header.btChannelMode == 3){
				pMpeg12Header.uiChannel = 1;
		}
		else{
				pMpeg12Header.uiChannel = 2;
		}

		return S_OK;
}

HRESULT CMFMpeg12Source::GetAudioBitRateMpeg1(Mpeg12AudioLayer layer, BYTE index, UINT* puiBitRate){
		
		HRESULT hr = S_OK;
		const DWORD MAX_BITRATE_INDEX = 14;

		assert(layer >= Mpeg12_Audio_Layer1 && layer <= Mpeg12_Audio_Layer3);

		// Table of bit rates. 
		const DWORD bitrate[3][(MAX_BITRATE_INDEX + 1)] = {
				{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },    // Layer I
				{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },       // Layer II
				{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 }         // Layer III
		};

		// Not needed here
		/*if(layer < MPEG2_Audio_Layer1 || layer > MPEG2_Audio_Layer3){
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}*/
		
		if(index > MAX_BITRATE_INDEX){

				//*pdwBitRate = 0;
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}
		else{

				*puiBitRate = bitrate[layer][index];
		}

		return hr;
}

HRESULT CMFMpeg12Source::GetAudioBitRateMpeg2(Mpeg12AudioLayer layer, BYTE index, UINT* puiBitRate){
		
		HRESULT hr = S_OK;
		const DWORD MAX_BITRATE_INDEX = 14;

		assert(layer >= Mpeg12_Audio_Layer1 && layer <= Mpeg12_Audio_Layer3);

		// Table of bit rates. 
		const DWORD bitrate[3][(MAX_BITRATE_INDEX + 1)] = {
				{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},    // Layer I
				{0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160},       // Layer II
				{0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160}         // Layer III
		};

		// Not needed here
		/*if(layer < MPEG2_Audio_Layer1 || layer > MPEG2_Audio_Layer3){
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}*/
		
		if(index > MAX_BITRATE_INDEX){

				//*pdwBitRate = 0;
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}
		else{

				*puiBitRate = bitrate[layer][index];
		}

		return hr;
}

HRESULT CMFMpeg12Source::GetSamplingFrequency(const BYTE btVersion, const BYTE btIndex, UINT* pdwSamplesPerSec){
		
		HRESULT hr = S_OK;

		assert(btIndex < 0x03);

		const DWORD SamplingRateMpeg1[3]  = { 44100, 48000, 32000};
		const DWORD SamplingRateMpeg2[3]  = { 22050, 24000, 16000};
		const DWORD SamplingRateMpeg25[3] = { 11025, 12000,  8000};

		if(btVersion == 0x03){
				*pdwSamplesPerSec = SamplingRateMpeg1[btIndex];
		}
		else if(btVersion == 0x02){
				*pdwSamplesPerSec = SamplingRateMpeg2[btIndex];
		}
		else{
				*pdwSamplesPerSec = SamplingRateMpeg25[btIndex];
		}
		
		return hr;
}

HRESULT CMFMpeg12Source::CreateAudioMediaType(IMFMediaType** ppmt, Mpeg12AudioFrameHeader& pMpeg12Header){

		TRACE_SOURCE((L"Source::CreateAudioMediaType"));

		HRESULT hr = S_OK;
		IMFMediaType* pType = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

				const UINT32 uiBitsPerSample = 16;
				const UINT32 uiBlockAlign = pMpeg12Header.uiChannel * (uiBitsPerSample / 8);
				m_uiAvgBytePerSec = uiBlockAlign * pMpeg12Header.uiSamplePerSec;

				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, TRUE));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pMpeg12Header.uiChannel));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pMpeg12Header.uiSamplePerSec));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, uiBlockAlign));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_uiAvgBytePerSec));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, uiBitsPerSample));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

				*ppmt = pType;
				(*ppmt)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);
		return hr;
}

HRESULT CMFMpeg12Source::GetSample(IMFSample** ppSample){

		TRACE_SOURCE((L"Source::GetSample"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;

		IF_FAILED_RETURN(hr = CheckShutdown());

		BOOL bNeedMoreData;
		UINT uiFrameSize = 0;
		UINT uiDecodedSize = 0;
		ULONG ulRead;

		while(!m_bEOF){

				bNeedMoreData = FALSE;

				// Todo : out of while
				if(m_bSkipInfo){
						IF_FAILED_RETURN(hr = SkipInfo(bNeedMoreData));
				}

				if(bNeedMoreData == FALSE){

						IF_FAILED_RETURN(hr = m_cMpeg12Decoder.DecodeFrame(m_cMFBuffer.GetStartBuffer(), m_cMFBuffer.GetBufferSize(), &uiFrameSize, m_sAudioBuffer, &uiDecodedSize));

						if(uiFrameSize == 0){
								bNeedMoreData = TRUE;
						}

						IF_FAILED_RETURN(hr = m_cMFBuffer.SetStartPosition(uiFrameSize));
				}

				if(bNeedMoreData){

						bNeedMoreData = FALSE;

						IF_FAILED_RETURN(hr = m_cMFBuffer.Reserve(MPEG12_READ_SIZE));
						IF_FAILED_RETURN(hr = m_pByteStream->Read(m_cMFBuffer.GetReadStartBuffer(), MPEG12_READ_SIZE, &ulRead));

						if(ulRead == 0){

								m_bEOF = TRUE;
								m_pStream->EndOfStream();
								break;
						}

						IF_FAILED_RETURN(hr = m_cMFBuffer.SetEndPosition(ulRead));
				}
				else{

						assert(uiDecodedSize != 0);
						IF_FAILED_RETURN(hr = CreateSample(ppSample, uiDecodedSize));
						break;
				}
		}

		return hr;
}

HRESULT CMFMpeg12Source::CreateSample(IMFSample** ppSample, const UINT uiDecodedSize){

		TRACE_SOURCE((L"Source::CreateSample"));

		HRESULT hr = S_OK;
		IMFSample* pSample = NULL;
		IMFMediaBuffer* pBuffer = NULL;
		BYTE* pData = NULL;

		LONGLONG duration = 0;

		try{

				IF_FAILED_THROW(hr = MFCreateMemoryBuffer(uiDecodedSize, &pBuffer));
				
				IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, NULL));

				CopyMemory(pData, m_sAudioBuffer, uiDecodedSize);

				IF_FAILED_THROW(hr = pBuffer->Unlock());

				IF_FAILED_THROW(hr = pBuffer->SetCurrentLength(uiDecodedSize));

				IF_FAILED_THROW(hr = MFCreateSample(&pSample));
    IF_FAILED_THROW(hr = pSample->AddBuffer(pBuffer));

				IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtCurrentPosition));

				duration = (LONGLONG)uiDecodedSize * 10000000 / m_uiAvgBytePerSec;
				IF_FAILED_THROW(hr = pSample->SetSampleDuration(duration));

				//TRACE((L"Time %I64d - Duration %I64d\r", m_rtCurrentPosition, duration));

				//MFTimeString(m_rtCurrentPosition);

				m_rtCurrentPosition += duration;

				*ppSample = pSample;
				(*ppSample)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pBuffer);
		SAFE_RELEASE(pSample);

		return hr;
}

HRESULT CMFMpeg12Source::SkipInfo(BOOL& bNeedMoreData){

		HRESULT hr = S_OK;
		BYTE* pData = m_cMFBuffer.GetStartBuffer();
		DWORD dwAte = 0;

		do{

				while(*pData != 0xff){

						if(dwAte == m_cMFBuffer.GetBufferSize()){
								break;
						}
						pData++;								
						dwAte++;
				}

				if(dwAte == m_cMFBuffer.GetBufferSize()){
						break;
				}

				pData++;
				dwAte++;

				if((*pData & 0xe0) == 0xe0)
						break;
		}
		while(true);

		if(dwAte == m_cMFBuffer.GetBufferSize()){

				bNeedMoreData = TRUE;
				return hr;
		}

		IF_FAILED_RETURN(hr = m_cMFBuffer.SetStartPosition(dwAte - 1));

		m_bSkipInfo = FALSE;

		return hr;
}