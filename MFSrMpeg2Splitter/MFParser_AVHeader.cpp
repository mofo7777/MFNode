//----------------------------------------------------------------------------------------------
// MFParser_AVHeader.cpp
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

HRESULT CMFParser::ReadVideoSequenceHeader(const BYTE *pData, DWORD cbData, MPEG2VideoSeqHeader& seqHeader){
		
		TRACE_PARSER((L"Parser::ReadVideoHeader"));

		while(((DWORD*)pData)[0] == 0){
				
				pData += 4;
				cbData -= 4;

				if(cbData < 4){						
						return S_FALSE;
				}
		}

		// See File Problem
		while(MAKE_DWORD(pData) != MPEG2_SEQUENCE_HEADER_CODE){
				
				pData += 1;
				cbData -= 1;

				if(cbData < 4){
						return S_FALSE;
				}
		}

		HRESULT hr = S_OK;

		//DWORD dwTest = MAKE_DWORD(pData);

		if(MAKE_DWORD(pData) != MPEG2_SEQUENCE_HEADER_CODE){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(cbData < MIN_BYTE_TO_READ_MPEG_HEADER){
				TRACE_PARSER((L"Parser MIN_BYTE_TO_READ_MPEG_HEADER"));
				return S_FALSE;
		}

		DWORD cbRequired = MIN_BYTE_TO_READ_MPEG_HEADER;

		BOOL bNonIntra = FALSE;
		
		if(HAS_FLAG(pData[11], 0x02)){

				cbRequired += 64;

				if(cbData >= cbRequired){
						bNonIntra = HAS_FLAG( pData[11 + 64], 0x01 );
				}
		}

		else if(HAS_FLAG(pData[11], 0x01)){
				cbRequired += 64;
		}

		if(cbData < cbRequired){
				TRACE_PARSER((L"Parser cbData < cbRequired"));
				return S_FALSE;
		}

		ZeroMemory(&seqHeader, sizeof(seqHeader));

		if(!HAS_FLAG(pData[10], 0x20)){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		BYTE parCode = pData[7] >> 4;
		BYTE frameRateCode = pData[7] & 0x0F;

		if(m_bMpeg1){

				if(FAILED(GetPixelAspectRatioMpeg1(parCode, &seqHeader.pixelAspectRatio))){
						IF_FAILED_RETURN(hr = E_FAIL);
				}

				if(FAILED(GetFrameRateMpeg1(frameRateCode, &seqHeader.frameRate))){
						IF_FAILED_RETURN(hr = E_FAIL);
				}
		}
		else{

				if(FAILED(GetPixelAspectRatioMpeg2(parCode, &seqHeader.pixelAspectRatio))){
						IF_FAILED_RETURN(hr = E_FAIL);
				}

				if(FAILED(GetFrameRateMpeg2(frameRateCode, &seqHeader.frameRate))){
						IF_FAILED_RETURN(hr = E_FAIL);
				}
		}

		seqHeader.width = (pData[4] << 4) | (pData[5] >> 4) ;
		seqHeader.height = ((pData[5] & 0x0F) << 8) | (pData[6]);
		seqHeader.bitRate = (pData[8] << 10) | (pData[9] << 2) | (pData[10] >> 6);

		if(seqHeader.bitRate == 0){
				IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(seqHeader.bitRate == 0x3FFFF){
				seqHeader.bitRate = 0;
		}
		else{
				seqHeader.bitRate = seqHeader.bitRate * 400;
		}

		seqHeader.cbVBV_Buffer = ( ((pData[10] & 0x1F) << 5) | (pData[11] >> 3) ) * 2048;
		seqHeader.bConstrained = HAS_FLAG(pData[11], 0x04);

		seqHeader.cbHeader = cbRequired;
		CopyMemory(seqHeader.header, pData, cbRequired);

		//*pAte = cbRequired + cbPadding;

		return hr;
}

HRESULT CMFParser::ReadAudioFrameHeader(const BYTE* pData, DWORD cbData, MPEG2AudioFrameHeader& audioHeader, DWORD* dwAte){
		
		TRACE_PARSER((L"Parser::ReadAudioHeader"));

		if(cbData < MPEG2_AUDIO_FRAME_HEADER_SIZE){
				TRACE_PARSER((L"Parser MPEG2_AUDIO_FRAME_HEADER_SIZE"));
				return S_FALSE;
		}

		HRESULT hr = S_OK;
		ZeroMemory(&audioHeader, sizeof(MPEG2AudioFrameHeader));

		const BYTE* pTmp = pData;
		*dwAte = 0;

		// Todo check end of payload (critical)
		do{

				while(*pTmp != 0xff){

						if(*dwAte == cbData)
								return S_FALSE;

						(*dwAte)++;
						pTmp++;
				}

				if(*dwAte == cbData)
						return S_FALSE;

				(*dwAte)++;
				pTmp++;

				if((*pTmp & 0xe0) == 0xe0)
						break;
		}
		while(true);

		(*dwAte)--;

		BYTE btVersion = (*pTmp & 0x18) >> 3;
		BYTE btLayer = (*pTmp & 0x06) >> 1;
		BOOL bProtection = *pTmp++ & 0x01;
		BYTE btBitrateIndex = (*pTmp & 0xf0) >> 4;
		BYTE btSamplingIndex = (*pTmp & 0x0C) >> 2;
		BOOL bPadding = *pTmp & 0x02;
		BOOL bPrivate = *pTmp++ & 0x01;
		audioHeader.mode = (MPEG2AudioMode)((*pTmp & 0xC0) >> 6);
		audioHeader.modeExtension = (*pTmp & 0x30) >> 4;
		BOOL bCopyright = *pTmp & 0x08;
		BOOL bOriginal = *pTmp & 0x04;
		audioHeader.emphasis = *pTmp & 0x03;

		/*if(btVersion == 0x01){
				TRACE((L"\rAudio version ERROR"));
		}
		else if(btVersion == 0x03){
				TRACE((L"\rAudio version 1"));
		}
		else{
				TRACE((L"\rAudio version : %d", (btVersion == 0x02 ? 2 : 25)));
		}*/

		/*TRACE((L"layer = %d\rCrc = %s\rBitRate = %d\rSamplesPerSec = %d\rPadding = %s\rPrivate = %s\rmode = %d\rmodeExtension = %d\rCopyright = %s\rOriginal = %s\remphasis = %d\r",
				btLayer, (bProtection ? L"True" : L"False"), btBitrateIndex, btSamplingIndex, (bPadding ? L"True" : L"False"), (bPrivate ? L"True" : L"False"),
				audioHeader.mode, audioHeader.modeExtension, (bCopyright ? L"True" : L"False"), (bOriginal ? L"True" : L"False"), audioHeader.emphasis));*/

		if(btLayer == 0x00){
				IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(btLayer == 0x01){
				audioHeader.layer = MPEG2_Audio_Layer3;
		}
		else if(btLayer == 0x02){
				audioHeader.layer = MPEG2_Audio_Layer2;
		}
		else{
				audioHeader.layer = MPEG2_Audio_Layer1;
		}

		/*if(bProtection == FALSE){
				CalculateCRC();
		}*/

		if(btVersion == 0x01){
				IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(btVersion == 0x03){
				IF_FAILED_RETURN(hr = GetAudioBitRateMpeg1(audioHeader.layer, btBitrateIndex, &audioHeader.dwBitRate));
		}
		else{
				IF_FAILED_RETURN(hr = GetAudioBitRateMpeg2(audioHeader.layer, btBitrateIndex, &audioHeader.dwBitRate));
		}

		if(btSamplingIndex == 0x03){
				IF_FAILED_RETURN(hr = E_FAIL);
		}

		IF_FAILED_RETURN(hr = GetSamplingFrequency(btVersion, btSamplingIndex, &audioHeader.dwSamplesPerSec));

		if(audioHeader.mode == MPEG2_Audio_SingleChannel){
				audioHeader.nChannels = 1;
		}
		else{
				audioHeader.nChannels = 2;
		}

		if(bProtection){
				audioHeader.wFlags |= MPEG2_AUDIO_PROTECTION_BIT;
		}
		if(bPrivate){
				audioHeader.wFlags |= MPEG2_AUDIO_PRIVATE_BIT;
		}
		if(bCopyright){
				audioHeader.wFlags |= MPEG2_AUDIO_COPYRIGHT_BIT;
		}
		if(bOriginal){
				audioHeader.wFlags |= MPEG2_AUDIO_ORIGINAL_BIT;
		}

		audioHeader.nBlockAlign = 1;

		//(*dwAte) += ??;

		return  hr;
}

HRESULT CMFParser::ReadAC3AudioFrameHeader(const BYTE* pData, DWORD cbData, WAVEFORMATEX& audioHeader, DWORD* dwAte){
		
		TRACE_PARSER((L"Parser::ReadAC3AudioHeader"));

		if(cbData < AC3_AUDIO_FRAME_HEADER_SIZE){
				TRACE_PARSER((L"Parser AC3_AUDIO_FRAME_HEADER_SIZE"));
				return S_FALSE;
		}

		HRESULT hr = S_OK;
		ZeroMemory(&audioHeader, sizeof(WAVEFORMATEX));

		const BYTE* pTmp = pData;
		*dwAte = 0;

		do{

				while(*pTmp != 0x0b){

						if(*dwAte == cbData)
								return S_FALSE;

						(*dwAte)++;
						pTmp++;
				}

				if(*dwAte == cbData)
						return S_FALSE;

				(*dwAte)++;
				pTmp++;

				if(*pTmp == 0x77)
						break;
		}
		while(true);

		(*dwAte) += (AC3_AUDIO_FRAME_HEADER_SIZE - 2);
		pTmp--;

		// Check AC3 header specification and improve this.
		WORD sync_ = 0;
  WORD crc1_ = 0;
  BYTE fscod = 0;
  BYTE frmsizecod = 0;
  BYTE bsid = 0;
  BYTE bsmod_ = 0;
  BYTE acmod = 0;
  BYTE cmixlev_ = 0;
  BYTE surmixlev_ = 0;
  BYTE dsurmod_ = 0;
  BYTE lfeon = 0;

		BYTE btTmp = *pTmp++;
	 sync_ = (*pTmp++) | (btTmp << 8 );

		btTmp = *pTmp++;
	 crc1_ = (*pTmp++) | (btTmp << 8 );
		
	 fscod = (*pTmp & 0xc0) >> 6;
	 frmsizecod = *pTmp++ & 0x3f;
	 bsid = (*pTmp & 0xf8) >> 3;
	 bsmod_ = *pTmp++ & 0x07;
	 acmod = (*pTmp & 0xe0) >> 5;

	 if((acmod & 1) && acmod != 1)
				cmixlev_ = (*pTmp & 0x18) >> 3;

	 if(acmod & 4)
				surmixlev_ = (*pTmp & 0x18) >> 3;

	 if(acmod == 2)
				dsurmod_ = (*pTmp & 0x18) >> 3;

	 lfeon = (*pTmp & 0x10) >> 4;
	 
	 if(bsid >= 12 || fscod == 3 || frmsizecod >= 38)
	 	return E_FAIL;
	 
	 audioHeader.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	 
		// Check in function for tab 0-7
	 int channels[] = {2, 1, 2, 3, 3, 4, 4, 5};
	 audioHeader.nChannels = channels[acmod] + lfeon;
	 
		// Check in function for tab 0-3
	 int freq[] = {48000, 44100, 32000, 0};
	 audioHeader.nSamplesPerSec = freq[fscod];
	 
	 switch(bsid){

				case 9: audioHeader.nSamplesPerSec >>= 1; break;
	 		case 10: audioHeader.nSamplesPerSec >>= 2; break;
	   case 11: audioHeader.nSamplesPerSec >>= 3; break;
	   default: break;
	 }

		// Check in function for tab 0-18
	 int rate[] = {32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 448, 512, 576, 640};	 
	 audioHeader.nAvgBytesPerSec = rate[frmsizecod >> 1] * 1000 / 8;

	 audioHeader.nBlockAlign = (WORD)(1536 * audioHeader.nAvgBytesPerSec / audioHeader.nSamplesPerSec);

		return  hr;
}

HRESULT CMFParser::GetPixelAspectRatioMpeg1(BYTE pixelAspectCode, MFRatio* pRatio){
		
		DWORD height[] = {0, 10000, 6735, 7031, 7615, 8055, 8437, 8935, 9157, 9815, 10255, 10695, 10950, 11575, 12015};

		const DWORD width = 10000;
		HRESULT hr = S_OK;

		if(pixelAspectCode < 1 || pixelAspectCode >= ARRAYSIZE(height)){
				IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}

		pRatio->Numerator = height[pixelAspectCode];
		pRatio->Denominator = width;

		return hr;
}

HRESULT CMFParser::GetFrameRateMpeg1(BYTE frameRateCode, MFRatio* pRatio){
		
		// OK for mpeg2, check for mpeg1...
		HRESULT hr = S_OK;

		MFRatio frameRates[] = {
				{ 0, 0 },           // invalid
				{ 24000, 1001 },    // 23.976 fps
				{ 24, 1 },
				{ 25, 1 },
				{ 30000, 1001 },    // 29.97 fps
				{ 50, 1 },
				{ 60000, 1001 },    // 59.94 fps
				{ 60, 1 }
		};

		if(frameRateCode < 1 || frameRateCode >= ARRAYSIZE(frameRates)){
				IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}

		pRatio->Numerator = frameRates[frameRateCode].Numerator;
		pRatio->Denominator = frameRates[frameRateCode].Denominator;

		return hr;
}

HRESULT CMFParser::GetPixelAspectRatioMpeg2(BYTE pixelAspectCode, MFRatio* pRatio){
		
		HRESULT hr = S_OK;

		if(pixelAspectCode < 1 || pixelAspectCode > 4){
				IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}

		int iNumerator;
		int iDenominator;

		if(pixelAspectCode == 0x01){
				iNumerator = iDenominator = 1;
		}
		else if(pixelAspectCode == 0x02){
				iNumerator = 4;
				iDenominator = 3;
		}
		else if(pixelAspectCode == 0x03){
				iNumerator = 16;
				iDenominator = 9;
		}
		else{
				iNumerator = 221;
				iDenominator = 100;
		}

		pRatio->Numerator = iNumerator;
		pRatio->Denominator = iDenominator;

		return hr;
}

HRESULT CMFParser::GetFrameRateMpeg2(BYTE frameRateCode, MFRatio* pRatio){
		
		// OK for mpeg2, check for mpeg1... i don't have the specification...
		HRESULT hr = S_OK;

		MFRatio frameRates[] = {
				{ 0, 0 },           // invalid
				{ 24000, 1001 },    // 23.976 fps
				{ 24, 1 },
				{ 25, 1 },
				{ 30000, 1001 },    // 29.97 fps
				{ 30, 1 },
				{ 50, 1 },
				{ 60000, 1001 },    // 59.94 fps
				{ 60, 1 }
		};

		if(frameRateCode < 1 || frameRateCode >= ARRAYSIZE(frameRates)){
				IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}

		pRatio->Numerator = frameRates[frameRateCode].Numerator;
		pRatio->Denominator = frameRates[frameRateCode].Denominator;

		return hr;
}

HRESULT CMFParser::GetAudioBitRateMpeg1(MPEG2AudioLayer layer, BYTE index, DWORD* pdwBitRate){
		
		HRESULT hr = S_OK;
		const DWORD MAX_BITRATE_INDEX = 14;

		assert(layer >= MPEG2_Audio_Layer1 && layer <= MPEG2_Audio_Layer3);

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

				*pdwBitRate = 0;
				//IF_FAILED_RETURN(hr = E_INVALIDARG);
		}
		else{

				*pdwBitRate = bitrate[layer][index];
		}

		return hr;
}

HRESULT CMFParser::GetAudioBitRateMpeg2(MPEG2AudioLayer layer, BYTE index, DWORD* pdwBitRate){
		
		HRESULT hr = S_OK;
		const DWORD MAX_BITRATE_INDEX = 14;

		assert(layer >= MPEG2_Audio_Layer1 && layer <= MPEG2_Audio_Layer3);

		// Table of bit rates. 
		const DWORD bitrate[3][(MAX_BITRATE_INDEX + 1)] = {
				{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},    // Layer I
				{0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160},    // Layer II
				{0,  8, 16, 24, 32, 40, 48,  56,  64,  80,  96, 112, 128, 144, 160}     // Layer III
		};

		// Not needed here
		/*if(layer < MPEG2_Audio_Layer1 || layer > MPEG2_Audio_Layer3){
				IF_FAILED_RETURN(hr = E_INVALIDARG);
		}*/
		
		if(index > MAX_BITRATE_INDEX){

				//*pdwBitRate = 0;
				//IF_FAILED_RETURN(hr = E_INVALIDARG);
		}
		else{

				*pdwBitRate = bitrate[layer][index];
		}

		return hr;
}

HRESULT CMFParser::GetSamplingFrequency(const BYTE btVersion, const BYTE btIndex, DWORD* pdwSamplesPerSec){
		
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