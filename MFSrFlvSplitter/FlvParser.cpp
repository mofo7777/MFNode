//----------------------------------------------------------------------------------------------
// FlvParser.cpp
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

CFlvParser::CFlvParser() : m_dwDebugLastSize(0){

	TRACE_PARSER((L"Parser::CTOR"));

	ZeroMemory(&m_FlvHeader, sizeof(FLV_HEADER));
	ZeroMemory(&m_FlvTag, sizeof(FLV_TAG));
	ZeroMemory(&m_FlvVideoHeader, sizeof(FLV_VIDEO_HEADER));
	ZeroMemory(&m_FlvAudioHeader, sizeof(FLV_AUDIO_HEADER));
}

HRESULT CFlvParser::ParseHeader(BYTE* pBuffer){

	TRACE_PARSER((L"Parser::ParseHeader"));

	HRESULT hr = E_FAIL;

	BYTE* pByte = pBuffer;

	if(*pByte++ != 0x46)
		return hr;

	if(*pByte++ != 0x4C)
		return hr;

	if(*pByte++ != 0x56)
		return hr;

	m_FlvHeader.btVersion = *pByte++;

	if(m_FlvHeader.btVersion != 1){
		TRACE((L"Parser::ParseHeader FLV version %c error", m_FlvHeader.btVersion));
		return hr;
	}

	// Some files do not respect this, but can be play...
	/*if((*pByte | 0x05) != 0x05){
			TRACE((L"Parser::ParseHeader FLV reserved bytes error"));
			return hr;
	}*/

	m_FlvHeader.bVideoPresent = ((*pByte & 0x04) == 0x04 ? TRUE : FALSE);
	m_FlvHeader.bAudioPresent = ((*pByte & 0x01) == 0x01 ? TRUE : FALSE);

	pByte++;

	DWORD dwSize = MAKE_DWORD(pByte);

	hr = (dwSize == 9 ? S_OK : E_FAIL);

	return hr;
}

HRESULT CFlvParser::ParseBytes(BYTE* pBuffer, const DWORD cbLen, DWORD* pdwAte, DWORD* dwNextRequest){

	TRACE_PARSER((L"Parser::ParseBytes"));

	*pdwAte = 0;

	if(cbLen < FLV_TAG_MINSIZE){
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	BYTE* pByte = pBuffer;

	m_FlvTag.dwPreviousTagSize = MAKE_DWORD(pByte);
	pByte += 4;

	// It seems not to be always respected...
	/*if(m_dwDebugLastSize != m_FlvTag.dwPreviousTagSize){
			TRACE((L"Parser::ParseBytes error last size : %d : %d", m_dwDebugLastSize, m_FlvTag.dwPreviousTagSize));
			return hr;
	}*/

	if((*pByte & 0xC0) != 0x00){
		TRACE((L"Parser::ParseHeader reserved bytes error"));
		return hr;
	}

	// Pre-processing required. We don't handle.
	if((*pByte & 0x20) != 0x00){
		TRACE((L"Parser::ParseBytes error Pre-processing required"));
		return hr;
	}

	// We do not handle decryption
	m_FlvTag.bFilter = FALSE;

	m_FlvTag.btFlvTag = *pByte++ & 0x1F;

	m_FlvTag.dwDataSize = MAKE_24DWORD(pByte);

	if(m_FlvTag.dwDataSize == 0){
		TRACE((L"Parser::ParseHeader size error"));
		return hr;
	}

	if((cbLen - FLV_TAG_MINSIZE) < m_FlvTag.dwDataSize){

		*dwNextRequest = m_FlvTag.dwDataSize;
		return hr = S_FALSE;
	}

	pByte += 3;

	m_FlvTag.dwTimeStamp = MAKE_24DWORD(pByte);
	pByte += 3;

	m_FlvTag.dwTimeStamp |= ((*pByte++) << 24);

	// Should be zero
	m_FlvTag.dwStreamID = MAKE_24DWORD(pByte);
	pByte += 3;

	switch(m_FlvTag.btFlvTag){

	case FLV_TAG_AUDIO:
		hr = ReadAudioTag(pByte, cbLen, pdwAte);
		break;

	case FLV_TAG_VIDEO:
		hr = ReadVideoTag(pByte, cbLen, pdwAte);
		break;

	case FLV_TAG_SCRIPT:
		m_FlvTag.bHasPacket = TRUE;
		m_FlvTag.dwPayloadSize = m_FlvTag.dwDataSize;
		*pdwAte = FLV_TAG_MINSIZE;
		hr = S_OK;
		break;

	default:
		assert(FALSE);
		break;
	}

	m_dwDebugLastSize = m_FlvTag.dwDataSize + 11;

	return hr;
}

HRESULT CFlvParser::ParseBytesStreaming(BYTE* pBuffer, const DWORD cbLen, DWORD* pdwAte, DWORD* dwNextRequest){

	TRACE_PARSER((L"Parser::ParseBytesStreaming"));

	*pdwAte = 0;

	if(cbLen < FLV_TAG_MINSIZE){
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	BYTE* pByte = pBuffer;

	m_FlvTag.dwPreviousTagSize = MAKE_DWORD(pByte);
	pByte += 4;

	// It seems not to be always respected...
	/*if(m_dwDebugLastSize != m_FlvTag.dwPreviousTagSize){
			TRACE((L"Parser::ParseBytes error last size : %d : %d", m_dwDebugLastSize, m_FlvTag.dwPreviousTagSize));
			return hr;
	}*/

	if((*pByte & 0xC0) != 0x00){
		TRACE((L"Parser::ParseHeader reserved bytes error"));
		return hr;
	}

	// Pre-processing required. We don't handle.
	if((*pByte & 0x20) != 0x00){
		TRACE((L"Parser::ParseBytes error Pre-processing required"));
		return hr;
	}

	// We do not handle decryption
	m_FlvTag.bFilter = FALSE;

	m_FlvTag.btFlvTag = *pByte++ & 0x1F;

	m_FlvTag.dwDataSize = MAKE_24DWORD(pByte);

	if(m_FlvTag.dwDataSize == 0){
		TRACE((L"Parser::ParseHeader size error"));
		return hr;
	}

	if((cbLen - FLV_TAG_MINSIZE) < m_FlvTag.dwDataSize){

		*dwNextRequest = m_FlvTag.dwDataSize;
		return hr = S_FALSE;
	}

	pByte += 3;

	m_FlvTag.dwTimeStamp = MAKE_24DWORD(pByte);
	pByte += 3;

	m_FlvTag.dwTimeStamp |= ((*pByte++) << 24);

	// Should be zero
	m_FlvTag.dwStreamID = MAKE_24DWORD(pByte);
	pByte += 3;

	switch(m_FlvTag.btFlvTag){

	case FLV_TAG_AUDIO:
		hr = ReadAudioTag(pByte, cbLen, pdwAte);
		break;

	case FLV_TAG_VIDEO:
		hr = ReadVideoTagStreaming(pByte, cbLen, pdwAte);
		break;

	case FLV_TAG_SCRIPT:
		m_FlvTag.bHasPacket = TRUE;
		m_FlvTag.dwPayloadSize = m_FlvTag.dwDataSize;
		*pdwAte = FLV_TAG_MINSIZE;
		hr = S_OK;
		break;

	default:
		assert(FALSE);
		break;
	}

	m_dwDebugLastSize = m_FlvTag.dwDataSize + 11;

	return hr;
}

HRESULT CFlvParser::ReadVideoTag(BYTE* pByte, const DWORD dwLen, DWORD* pdwAte){

	TRACE_PARSER((L"Parser::ReadVideoTag"));

	if(dwLen < m_FlvTag.dwDataSize){
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	m_FlvVideoHeader.btVideoFrame = (*pByte & 0xf0) >> 4;
	m_FlvVideoHeader.btVideoCodec = (*pByte & 0x0f);

	pByte++;

	*pdwAte = FLV_TAG_MINSIZE + 2;
	DWORD dwRemaining = m_FlvTag.dwDataSize - 2;

	if(m_FlvVideoHeader.btVideoCodec == FLV_VIDEOCODEC_AVC){

		m_FlvVideoHeader.btAVPacketType = *pByte++;

		if(m_FlvVideoHeader.btAVPacketType == FLV_ENDSEQUENCE_AVC){
			TRACE((L"ReadVideoTag FLV_ENDSEQUENCE_AVC"));
		}

		if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_NALU){
			m_FlvVideoHeader.dwCompositionTime = MAKE_24DWORD(pByte);
		}
		else{
			m_FlvVideoHeader.dwCompositionTime = 0;
		}

		pByte += 3;
		*pdwAte += 4;
		dwRemaining -= 4;
	}

	m_FlvVideoHeader.bIsKeyFrame = FALSE;

	switch(m_FlvVideoHeader.btVideoFrame){

	case FLV_INFO_FRAME:
		TRACE((L"ReadVideoTag FLV_INFO_FRAME"));
		hr = S_OK;
		*pdwAte += 1;

#ifdef _DEBUG
		dwRemaining -= 1;
		assert(dwRemaining == 0);
#endif
		return hr;

		//case FLV_KEY_FRAME: TRACE((L"ReadVideoTag FLV_KEY_FRAME")); break;
		//case FLV_INTER_FRAME: TRACE((L"ReadVideoTag FLV_INTER_FRAME")); break;
		//case FLV_DISINTER_FRAME: TRACE((L"ReadVideoTag FLV_DISINTER_FRAME")); break;
		//case FLV_GENKEY_FRAME: TRACE((L"ReadVideoTag FLV_GENKEY_FRAME")); break;
	case FLV_KEY_FRAME: m_FlvVideoHeader.bIsKeyFrame = TRUE; break;
	case FLV_INTER_FRAME: break;
	case FLV_DISINTER_FRAME: break;
	case FLV_GENKEY_FRAME: break;
	default: TRACE((L"ReadVideoTag KEY_FRAME error")); return hr;
	}

	// Check dwLen and Remain

	switch(m_FlvVideoHeader.btVideoCodec){

	case FLV_VIDEOCODEC_AVC:
		if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_AVC){

			m_FlvVideoHeader.dwConfigurationVersion = *pByte++;
			m_FlvVideoHeader.btAvcProfileIndication = *pByte++;
			m_FlvVideoHeader.btProfileCompatibility = *pByte++;
			m_FlvVideoHeader.btAvcLevelIndication = *pByte++;

			//m_FlvVideoHeader.btLengthSizeMinusOne = *pByte++ & 0x03;
			m_FlvVideoHeader.btLengthSizeMinusOne = (*pByte++ & 0x03) + 1;

			if(m_FlvVideoHeader.btLengthSizeMinusOne != 1 && m_FlvVideoHeader.btLengthSizeMinusOne != 2 && m_FlvVideoHeader.btLengthSizeMinusOne != 4){
				break;
			}

			m_FlvVideoHeader.wNumberOfSequenceParameterSets = *pByte++ & 0x1F;

			m_FlvVideoHeader.pVideoRecord[0] = 0x00;
			m_FlvVideoHeader.pVideoRecord[1] = 0x00;
			m_FlvVideoHeader.pVideoRecord[2] = 0x01;

			for(int i = 0; i < m_FlvVideoHeader.wNumberOfSequenceParameterSets; i++){

				m_FlvVideoHeader.wSequenceParameterSetLength = ((*pByte++) << 8);
				m_FlvVideoHeader.wSequenceParameterSetLength += *pByte++;

				memcpy(m_FlvVideoHeader.pVideoRecord + 3, pByte, m_FlvVideoHeader.wSequenceParameterSetLength);

				pByte += m_FlvVideoHeader.wSequenceParameterSetLength;
			}

			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 3] = 0x00;
			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 4] = 0x00;
			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 5] = 0x01;

			m_FlvVideoHeader.btPictureParameterSets = *pByte++;

			for(int i = 0; i < m_FlvVideoHeader.btPictureParameterSets; i++){

				m_FlvVideoHeader.wPictureParameterSetLength = ((*pByte++) << 8);
				m_FlvVideoHeader.wPictureParameterSetLength += *pByte++;

				memcpy(m_FlvVideoHeader.pVideoRecord + 6 + m_FlvVideoHeader.wSequenceParameterSetLength, pByte, m_FlvVideoHeader.wPictureParameterSetLength);
				pByte += m_FlvVideoHeader.wPictureParameterSetLength;
			}

			m_FlvVideoHeader.bHasVideoInfo = TRUE;
			*pdwAte += dwRemaining;
			hr = S_OK;
		}
		else if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_NALU){

			pByte++;
			*pByte++ = 0x00;
			*pByte++ = 0x00;
			*pByte++ = 0x01;

			if(*pByte == 0x06){

				*pdwAte += 5;
				dwRemaining -= 5;
				// Check remain...

				LOG_HRESULT(CheckNoVlc(pByte, pdwAte, &dwRemaining));
			}

			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		else if(m_FlvVideoHeader.btAVPacketType == FLV_ENDSEQUENCE_AVC){
			TRACE((L"ReadVideoTag FLV_ENDSEQUENCE_AVC"));
		}
		else{
			TRACE((L"ReadVideoTag unknown flv packet type for h264"));
		}
		break;

	case FLV_VIDEOCODEC_H263:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_H263", L"Video Codec", MB_OK);
		//TRACE((L"ReadVideoTag FLV_VIDEOCODEC_H263"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;

			  //BYTE bt[50];
			  //memcpy(bt, pByte, 50);

		pByte += 2;

		if((*pByte & 0x80) != 0x80){
			break;
		}

		pByte += 1;
		m_FlvVideoHeader.uiWidth = 0;
		m_FlvVideoHeader.uiHeight = 0;

		{

			BYTE PictureSize = *pByte++ & 0x3;
			PictureSize |= ((*pByte & 0x80) >> 7);

			switch(PictureSize){

			case 0:
				m_FlvVideoHeader.uiWidth = ((*pByte++ & 0x7f) << 1);
				m_FlvVideoHeader.uiWidth |= ((*pByte & 0x80) >> 7);
				m_FlvVideoHeader.uiHeight = ((*pByte++ & 0x7f) << 1);
				m_FlvVideoHeader.uiHeight |= ((*pByte & 0x80) >> 7);
				break;

			case 1:
				m_FlvVideoHeader.uiWidth = ((*pByte++ & 0x7f) << 9);
				m_FlvVideoHeader.uiWidth |= ((*pByte++ & 0xff) << 1);
				m_FlvVideoHeader.uiWidth |= ((*pByte & 0x80) >> 7);
				m_FlvVideoHeader.uiHeight = ((*pByte++ & 0x7f) << 9);
				m_FlvVideoHeader.uiHeight |= ((*pByte++ & 0xff) << 1);
				m_FlvVideoHeader.uiHeight |= ((*pByte & 0x80) >> 7);
				break;

			case 2:
			case 3:
			case 4:
				m_FlvVideoHeader.uiWidth = 704 / PictureSize;
				m_FlvVideoHeader.uiHeight = 576 / PictureSize;
				break;

			case 5:
			case 6:
				PictureSize -= 3;
				m_FlvVideoHeader.uiWidth = 640 / PictureSize;
				m_FlvVideoHeader.uiHeight = 480 / PictureSize;
				break;
			}

			if(!m_FlvVideoHeader.uiWidth || !m_FlvVideoHeader.uiHeight)
				break;

			m_FlvVideoHeader.bHasVideoInfo = TRUE;
			*pdwAte += dwRemaining;
			hr = S_OK;
		}
		break;

	case FLV_VIDEOCODEC_SCREEN:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_SCREEN", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTag FLV_VIDEOCODEC_SCREEN"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	case FLV_VIDEOCODEC_VP6:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_VP6", L"Video Codec", MB_OK);
			  //TRACE((L"ReadVideoTag FLV_VIDEOCODEC_VP6"));

		pByte++;
		//*pdwAte += 1;
		//dwRemaining--;

		if((*pByte & 0x80) == 0x80){

			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		else{

			bool bSeparatedCoeff = *pByte++ & 0x01;
			int iFilterHeader = (*pByte++ & 0x06) >> 1;

			if(bSeparatedCoeff || !iFilterHeader){
				pByte += 2;
			}

			m_FlvVideoHeader.uiHeight = (*pByte++) * 16;
			m_FlvVideoHeader.uiWidth = (*pByte++) * 16;

			/*UINT32 ary = (*pByte++) * 16;
			UINT32 arx = (*pByte++) * 16;

			if(arx && arx != m_FlvVideoHeader.uiWidth || ary && ary != m_FlvVideoHeader.uiHeight){

					// Aspect Ration = arx / ary
			}*/

			m_FlvVideoHeader.bHasVideoInfo = TRUE;
			*pdwAte += dwRemaining;
			hr = S_OK;
		}
		break;

	case FLV_VIDEOCODEC_VP6A:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_VP6A", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTag FLV_VIDEOCODEC_VP6A"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	case FLV_VIDEOCODEC_SCREEN2:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_SCREEN2", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTag FLV_VIDEOCODEC_SCREEN2"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	default:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_UNKNOWN", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTag unknown codec error"));
		break;
	}

	return hr;
}

HRESULT CFlvParser::ReadVideoTagStreaming(BYTE* pByte, const DWORD dwLen, DWORD* pdwAte){

	TRACE_PARSER((L"Parser::ReadVideoTagStreaming"));

	if(dwLen < m_FlvTag.dwDataSize){
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	m_FlvVideoHeader.btVideoFrame = (*pByte & 0xf0) >> 4;
	m_FlvVideoHeader.btVideoCodec = (*pByte & 0x0f);

	pByte++;

	*pdwAte = FLV_TAG_MINSIZE + 2;
	DWORD dwRemaining = m_FlvTag.dwDataSize - 2;

	if(m_FlvVideoHeader.btVideoCodec == FLV_VIDEOCODEC_AVC){

		m_FlvVideoHeader.btAVPacketType = *pByte++;

		if(m_FlvVideoHeader.btAVPacketType == FLV_ENDSEQUENCE_AVC){
			TRACE((L"ReadVideoTagStreaming FLV_ENDSEQUENCE_AVC"));
		}

		if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_NALU){
			m_FlvVideoHeader.dwCompositionTime = MAKE_24DWORD(pByte);
		}
		else{
			m_FlvVideoHeader.dwCompositionTime = 0;
		}

		pByte += 3;
		*pdwAte += 4;
		dwRemaining -= 4;
	}

	m_FlvVideoHeader.bIsKeyFrame = FALSE;

	switch(m_FlvVideoHeader.btVideoFrame){

	case FLV_INFO_FRAME:
		TRACE((L"ReadVideoTagStreaming FLV_INFO_FRAME"));
		hr = S_OK;
		*pdwAte += 1;

#ifdef _DEBUG
		dwRemaining -= 1;
		assert(dwRemaining == 0);
#endif
		return hr;

		//case FLV_KEY_FRAME: TRACE((L"ReadVideoTag FLV_KEY_FRAME")); break;
		//case FLV_INTER_FRAME: TRACE((L"ReadVideoTag FLV_INTER_FRAME")); break;
		//case FLV_DISINTER_FRAME: TRACE((L"ReadVideoTag FLV_DISINTER_FRAME")); break;
		//case FLV_GENKEY_FRAME: TRACE((L"ReadVideoTag FLV_GENKEY_FRAME")); break;
	case FLV_KEY_FRAME: m_FlvVideoHeader.bIsKeyFrame = TRUE; break;
	case FLV_INTER_FRAME: break;
	case FLV_DISINTER_FRAME: break;
	case FLV_GENKEY_FRAME: break;
	default: TRACE((L"ReadVideoTagStreaming KEY_FRAME error")); return hr;
	}

	switch(m_FlvVideoHeader.btVideoCodec){

	case FLV_VIDEOCODEC_AVC:
		if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_AVC){

			//m_FlvVideoHeader.dwConfigurationVersion = MAKE_DWORD(pByte);
			//pByte += 4;

			m_FlvVideoHeader.dwConfigurationVersion = *pByte++;
			m_FlvVideoHeader.btAvcProfileIndication = *pByte++;
			m_FlvVideoHeader.btProfileCompatibility = *pByte++;
			m_FlvVideoHeader.btAvcLevelIndication = *pByte++;

			//m_FlvVideoHeader.btLengthSizeMinusOne = *pByte++ & 0x03;
			m_FlvVideoHeader.btLengthSizeMinusOne = (*pByte++ & 0x03) + 1;

			if(m_FlvVideoHeader.btLengthSizeMinusOne != 1 && m_FlvVideoHeader.btLengthSizeMinusOne != 2 && m_FlvVideoHeader.btLengthSizeMinusOne != 4){
				break;
			}

			m_FlvVideoHeader.wNumberOfSequenceParameterSets = *pByte++ & 0x1F;

			m_FlvVideoHeader.pVideoRecord[0] = 0x00;
			m_FlvVideoHeader.pVideoRecord[1] = 0x00;
			m_FlvVideoHeader.pVideoRecord[2] = 0x01;

			for(int i = 0; i < m_FlvVideoHeader.wNumberOfSequenceParameterSets; i++){

				m_FlvVideoHeader.wSequenceParameterSetLength = ((*pByte++) << 8);
				m_FlvVideoHeader.wSequenceParameterSetLength += *pByte++;

				memcpy(m_FlvVideoHeader.pVideoRecord + 3, pByte, m_FlvVideoHeader.wSequenceParameterSetLength);

				pByte += m_FlvVideoHeader.wSequenceParameterSetLength;
			}

			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 3] = 0x00;
			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 4] = 0x00;
			m_FlvVideoHeader.pVideoRecord[m_FlvVideoHeader.wSequenceParameterSetLength + 5] = 0x01;

			m_FlvVideoHeader.btPictureParameterSets = *pByte++;

			for(int i = 0; i < m_FlvVideoHeader.btPictureParameterSets; i++){

				m_FlvVideoHeader.wPictureParameterSetLength = ((*pByte++) << 8);
				m_FlvVideoHeader.wPictureParameterSetLength += *pByte++;

				memcpy(m_FlvVideoHeader.pVideoRecord + 6 + m_FlvVideoHeader.wSequenceParameterSetLength, pByte, m_FlvVideoHeader.wPictureParameterSetLength);
				pByte += m_FlvVideoHeader.wPictureParameterSetLength;
			}

			m_FlvVideoHeader.bHasVideoInfo = TRUE;
			*pdwAte += dwRemaining;
			hr = S_OK;
		}
		else if(m_FlvVideoHeader.btAVPacketType == FLV_SEQUENCE_NALU){

			pByte++;
			*pByte++ = 0x00;
			*pByte++ = 0x00;
			*pByte++ = 0x01;

			if(*pByte == 0x06){

				*pdwAte += 5;
				dwRemaining -= 5;
				// Check remain...

				LOG_HRESULT(CheckNoVlc(pByte, pdwAte, &dwRemaining));
			}

			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		else if(m_FlvVideoHeader.btAVPacketType == FLV_ENDSEQUENCE_AVC){
			TRACE((L"ReadVideoTagStreaming FLV_ENDSEQUENCE_AVC"));
		}
		else{
			TRACE((L"ReadVideoTagStreaming unknown flv packet type for h264"));
		}
		break;

	case FLV_VIDEOCODEC_H263:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_H263", L"Video Codec", MB_OK);
		//TRACE((L"ReadVideoTagStreaming FLV_VIDEOCODEC_H263"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		*pdwAte -= 1;
		dwRemaining += 1;
		m_FlvTag.bHasPacket = TRUE;
		m_FlvTag.dwPayloadSize = dwRemaining;
		hr = S_OK;
		break;

	case FLV_VIDEOCODEC_SCREEN:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_SCREEN", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTagStreaming FLV_VIDEOCODEC_SCREEN"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	case FLV_VIDEOCODEC_VP6:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_VP6", L"Video Codec", MB_OK);
			  //TRACE((L"ReadVideoTag FLV_VIDEOCODEC_VP6"));

		pByte++;
		//*pdwAte += 1;
		//dwRemaining--;

		if((*pByte & 0x80) == 0x80){

			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		else{

			bool bSeparatedCoeff = *pByte++ & 0x01;
			int iFilterHeader = (*pByte++ & 0x06) >> 1;

			if(bSeparatedCoeff || !iFilterHeader){
				pByte += 2;
			}

			m_FlvVideoHeader.uiHeight = (*pByte++) * 16;
			m_FlvVideoHeader.uiWidth = (*pByte++) * 16;

			/*UINT32 ary = (*pByte++) * 16;
			UINT32 arx = (*pByte++) * 16;

			if(arx && arx != m_FlvVideoHeader.uiWidth || ary && ary != m_FlvVideoHeader.uiHeight){

					// Aspect Ration = arx / ary
			}*/

			m_FlvVideoHeader.bIsKeyFrame = TRUE;
			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		break;

	case FLV_VIDEOCODEC_VP6A:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_VP6A", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTagStreaming FLV_VIDEOCODEC_VP6A"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	case FLV_VIDEOCODEC_SCREEN2:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_SCREEN2", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTagStreaming FLV_VIDEOCODEC_SCREEN2"));
		//*pdwAte += dwRemaining;
		//hr = S_OK;
		break;

	default:
		//MessageBox(NULL, L"FLV_VIDEOCODEC_UNKNOWN", L"Video Codec", MB_OK);
		TRACE((L"ReadVideoTagStreaming unknown codec error"));
		break;
	}

	return hr;
}

HRESULT CFlvParser::ReadAudioTag(BYTE* pByte, const DWORD dwLen, DWORD* pdwAte){

	TRACE_PARSER((L"Parser::ReadAudioTag"));

	if(dwLen < m_FlvTag.dwDataSize){
		return S_FALSE;
	}

	HRESULT hr = E_FAIL;

	*pdwAte = FLV_TAG_MINSIZE + 1;
	DWORD dwRemaining = m_FlvTag.dwDataSize - 1;

	m_FlvAudioHeader.btSoundFMT = (*pByte & 0xf0) >> 4;

	WORD wSampling = (*pByte & 0x0C) >> 2;

	if(wSampling == 0)
		m_FlvAudioHeader.uiSampleRate = 5500;
	else if(wSampling == 1)
		m_FlvAudioHeader.uiSampleRate = 11000;
	else if(wSampling == 2)
		m_FlvAudioHeader.uiSampleRate = 22000;
	else if(wSampling == 3)
		m_FlvAudioHeader.uiSampleRate = 44000;
	else
		return hr;

	m_FlvAudioHeader.wBitSample = ((*pByte & 0x02) == 0x02 ? 16 : 8);
	m_FlvAudioHeader.bStereo = ((*pByte++ & 0x01) == 0x01 ? TRUE : FALSE);

	BOOL bRaw = TRUE;

	if(m_FlvAudioHeader.btSoundFMT == FLV_AUDIOCODEC_AAC){

		*pdwAte += 1;
		dwRemaining -= 1;

		bRaw = ((*pByte++ & 0x01) == 0x01 ? TRUE : FALSE);

		if(bRaw){

			//TRACE((L"Parser::ReadAudioTag : raw AAC"));
			m_FlvTag.bHasPacket = TRUE;
			m_FlvTag.dwPayloadSize = dwRemaining;
			hr = S_OK;
		}
		else{

			// Seems not tobr always true...
			//if(dwRemaining == 2){

					//TRACE((L"Parser::ReadAudioTag : AAC"));

			m_FlvAudioHeader.btAACHeader[0] = *pByte;
			m_FlvAudioHeader.btAACHeader[1] = *(pByte + 1);

			WORD wSampleRate = ((*pByte++ & 0x07) << 1);
			wSampleRate += ((*pByte & 0x80) >> 7);
			WORD wChannel = ((*pByte & 0x78) >> 3);

			if(wSampleRate < 12 && wChannel < 8){

				m_FlvAudioHeader.uiSampleRate = g_uiSampleRates[wSampleRate];
				m_FlvAudioHeader.wChannel = g_wChannels[wChannel];
				m_FlvAudioHeader.wBitSample = 16;
				//m_FlvAudioHeader.wFormatTag = WAVE_FORMAT_AAC;

				*pdwAte += dwRemaining;
				//*pdwAte += 2;
				m_FlvAudioHeader.bHasAudioInfo = TRUE;
				//m_FlvTag.bHasPacket = TRUE;
				hr = S_OK;
			}
			//}
		}
	}
	else if(m_FlvAudioHeader.btSoundFMT == FLV_AUDIOCODEC_MP3){

		do{

			while(*pByte != 0xff){
				pByte++;
				*pdwAte += 1;
				dwRemaining--;
			}

			pByte++;
			*pdwAte += 1;
			dwRemaining--;

			if((*pByte & 0xe0) == 0xe0)
				break;
		} while(true);

		*pdwAte -= 1;
		dwRemaining++;

		Mpeg12AudioFrameHeader Mpeg12Header;

		Mpeg12Header.btVersion = (*pByte & 0x18) >> 3;
		Mpeg12Header.btLayer = (*pByte & 0x06) >> 1;
		Mpeg12Header.bProtection = *pByte++ & 0x01;
		Mpeg12Header.btBitrateIndex = (*pByte & 0xf0) >> 4;
		Mpeg12Header.btSamplingIndex = (*pByte & 0x0C) >> 2;
		Mpeg12Header.bPadding = *pByte & 0x02;
		Mpeg12Header.bPrivate = *pByte++ & 0x01;
		Mpeg12Header.btChannelMode = (*pByte & 0xC0) >> 6;
		Mpeg12Header.btModeExtension = (*pByte & 0x30) >> 4;
		Mpeg12Header.bCopyright = *pByte & 0x08;
		Mpeg12Header.bOriginal = *pByte & 0x04;
		Mpeg12Header.btEmphasis = *pByte & 0x03;

		if(Mpeg12Header.btVersion == 0x01){
			IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(Mpeg12Header.btLayer == 0x00){
			IF_FAILED_RETURN(hr = E_FAIL);
		}
		else if(Mpeg12Header.btLayer == 0x01){
			Mpeg12Header.Layer = Mpeg12_Audio_Layer3;
		}
		else if(Mpeg12Header.btLayer == 0x02){
			Mpeg12Header.Layer = Mpeg12_Audio_Layer2;
		}
		else{
			Mpeg12Header.Layer = Mpeg12_Audio_Layer1;
		}

		if(Mpeg12Header.btSamplingIndex == 0x03){
			IF_FAILED_RETURN(hr = E_FAIL);
		}

		if(Mpeg12Header.btVersion == 0x03){
			IF_FAILED_RETURN(hr = GetAudioBitRateMpeg1(Mpeg12Header.Layer, Mpeg12Header.btBitrateIndex, &Mpeg12Header.uiBitrate));
		}
		else{
			IF_FAILED_RETURN(hr = GetAudioBitRateMpeg2(Mpeg12Header.Layer, Mpeg12Header.btBitrateIndex, &Mpeg12Header.uiBitrate));
		}

		IF_FAILED_RETURN(hr = GetSamplingFrequency(Mpeg12Header.btVersion, Mpeg12Header.btSamplingIndex, &Mpeg12Header.uiSamplePerSec));

		if(Mpeg12Header.btChannelMode == 3){
			Mpeg12Header.uiChannel = 1;
		}
		else{
			Mpeg12Header.uiChannel = 2;
		}

		m_FlvAudioHeader.uiBitrate = Mpeg12Header.uiBitrate;
		m_FlvAudioHeader.uiSampleRate = Mpeg12Header.uiSamplePerSec;
		m_FlvAudioHeader.wChannel = (WORD)Mpeg12Header.uiChannel;
		m_FlvAudioHeader.bHasAudioInfo = TRUE;
		m_FlvTag.bHasPacket = TRUE;

		assert(*pdwAte <= dwRemaining);

		//*pdwAte += dwRemaining;
		m_FlvAudioHeader.bHasAudioInfo = TRUE;
		m_FlvTag.bHasPacket = TRUE;
		m_FlvTag.dwPayloadSize = dwRemaining;

		hr = S_OK;
	}
	else{

		switch(m_FlvAudioHeader.btSoundFMT){

		case FLV_AUDIOCODEC_PCM: MessageBox(NULL, L"FLV_AUDIOCODEC_PCM", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_ADPCM: MessageBox(NULL, L"FLV_AUDIOCODEC_ADPCM", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_LPCM: MessageBox(NULL, L"FLV_AUDIOCODEC_LPCM", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_NELLY16: MessageBox(NULL, L"FLV_AUDIOCODEC_NELLY16", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_NELLY8: MessageBox(NULL, L"FLV_AUDIOCODEC_NELLY8", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_NELLY: MessageBox(NULL, L"FLV_AUDIOCODEC_NELLY", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_G711A: MessageBox(NULL, L"FLV_AUDIOCODEC_G711A", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_G711M: MessageBox(NULL, L"FLV_AUDIOCODEC_G711M", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_SPEEX: MessageBox(NULL, L"FLV_AUDIOCODEC_SPEEX", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_MP38: MessageBox(NULL, L"FLV_AUDIOCODEC_MP38", L"Audio Codec", MB_OK); break;
		case FLV_AUDIOCODEC_SPECIFIC: MessageBox(NULL, L"FLV_AUDIOCODEC_SPECIFIC", L"Audio Codec", MB_OK); break;
		default: MessageBox(NULL, L"FLV_AUDIOCODEC_UNKNOWN", L"Video Codec", MB_OK); break;
		}

		//assert(FALSE);
	}

	return hr;
}

HRESULT CFlvParser::CheckNoVlc(BYTE* pData, DWORD* pdwAte, DWORD* pdwRemain){

	HRESULT hr = E_FAIL;

	pData++;
	DWORD dwSeiSize = 0;

	while(*pData == 0xff){

		pData++;
		*pdwAte += 1;
		*pdwRemain -= 1;

		if(*pdwAte >= *pdwRemain)
			return hr;
	}

	pData++;
	*pdwAte += 1;
	*pdwRemain -= 1;

	while(*pData == 0xff){

		pData++;
		*pdwAte += 1;
		*pdwRemain -= 1;
		dwSeiSize += 255;

		if(*pdwAte >= *pdwRemain)
			return hr;
	}

	dwSeiSize += *pData++;

	if((dwSeiSize + *pdwAte) < *pdwRemain){

		*pdwAte += (2 + dwSeiSize);
		*pdwRemain -= (2 + dwSeiSize);

		pData += (dwSeiSize + 2);

		*pData++ = 0x00;
		*pData++ = 0x00;
		*pData++ = 0x01;

		hr = S_OK;
	}

	return hr;
}

HRESULT CFlvParser::GetAudioBitRateMpeg1(Mpeg12AudioLayer layer, BYTE index, UINT* puiBitRate){

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

HRESULT CFlvParser::GetAudioBitRateMpeg2(Mpeg12AudioLayer layer, BYTE index, UINT* puiBitRate){

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

HRESULT CFlvParser::GetSamplingFrequency(const BYTE btVersion, const BYTE btIndex, UINT* pdwSamplesPerSec){

	HRESULT hr = S_OK;

	assert(btIndex < 0x03);

	const DWORD SamplingRateMpeg1[3] = {44100, 48000, 32000};
	const DWORD SamplingRateMpeg2[3] = {22050, 24000, 16000};
	const DWORD SamplingRateMpeg25[3] = {11025, 12000,  8000};

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