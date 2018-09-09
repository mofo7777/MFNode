//----------------------------------------------------------------------------------------------
// MFParser.cpp
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

CMFParser::CMFParser()
	: m_bHasFoundSysHeader(FALSE),
	m_bHasSysHeader(FALSE),
	m_bHasPacket(FALSE),
	m_bEOS(FALSE),
	m_bMpeg1(FALSE),
	m_SCR(0),
	m_SCRExt(0),
	m_muxRate(0),
	m_dwPack(0),
	m_dwByteAte(0),
	m_dwStartCode(0)
{
	ZeroMemory(&m_curPacketHeader, sizeof(MPEG2PacketHeader));
}

HRESULT CMFParser::ParseBytes(const BYTE* pData, DWORD cbLen, DWORD* pAte){

	*pAte = 0;

	if(cbLen < 4){
		return S_FALSE;
	}

	HRESULT hr = S_OK;
	DWORD cbLengthToStartCode = 0;
	DWORD cbParsed = 0;

	m_bHasPacket = FALSE;

	hr = FindNextStartCode(pData, cbLen, &cbLengthToStartCode);

	if(hr == S_OK){

		m_dwStartCode++;

		cbLen -= cbLengthToStartCode;
		pData += cbLengthToStartCode;
		DWORD dwCode = MAKE_DWORD(pData);

		switch(dwCode){

		case MPEG2_PACK_START_CODE:
			// Start of pack.
			//TRACE((L"\rParsePackHeader"));
			hr = ParsePackHeader(pData, cbLen, &cbParsed);
			break;

		case MPEG2_SYSTEM_HEADER_CODE:
			// Start of system header.
			//TRACE((L"\rParseSystemHeader"));
			hr = ParseSystemHeader(pData, cbLen, &cbParsed);
			break;

		case MPEG2_STOP_CODE:
			// Stop code, end of stream.
			//TRACE((L"\rOnEndOfStream"));
			cbParsed = sizeof(DWORD);
			OnEndOfStream();
			break;

		default:

			if(dwCode < 0x1B8 || dwCode > 0x1f0){
				cbParsed = sizeof(DWORD);
				break;
			}
			// Start of packet.
			if(m_bMpeg1){
				//TRACE((L"\rParsePacketHeader Mpeg1"));
				hr = ParsePacketHeaderMpeg1(pData, cbLen, &cbParsed);
			}
			else{
				//TRACE((L"\rParsePacketHeader Mpeg2"));
				hr = ParsePacketHeaderMpeg2(pData, cbLen, &cbParsed);
			}
			break;
		}
	}

	if(hr == S_OK){
		*pAte = cbLengthToStartCode + cbParsed;
	}

	return hr;
}

HRESULT CMFParser::FindNextStartCode(const BYTE* pData, const DWORD cbLen, DWORD* pAte){

	HRESULT hr = S_FALSE;

	DWORD cbLeft = cbLen;

	while(cbLeft >= 4){

		if((MAKE_DWORD_HOSTORDER(pData) & 0x00FFFFFF) == 0x00010000){

			hr = S_OK;
			break;
		}

		cbLeft -= 1;
		pData += 1;
	}

	*pAte = (cbLen - cbLeft);

	return hr;
}

HRESULT CMFParser::ParsePackHeader(const BYTE* pData, DWORD cbLen, DWORD* pAte){

	// Choose MPEG2_PACK_HEADER_SIZE because don't know if mpeg1 or mpeg2
	if(cbLen < MPEG2_PACK_HEADER_SIZE){
		//TRACE((L"ParsePackHeader MPEG2_PACK_HEADER_SIZE"));
		return S_FALSE;
	}

	HRESULT hr = S_OK;

	if((pData[4] & 0xc4) == 0x44 && (pData[6] & 0x04) == 0x04 && (pData[8] & 0x04) == 0x04 && (pData[9] & 0x01) == 0x01 && (pData[12] & 0x03) == 0x03){

		//TRACE((L"MPEG2"));
		m_bMpeg1 = FALSE;
		*pAte = MPEG2_PACK_HEADER_SIZE;

		m_muxRate = (pData[12] >> 2) | (pData[11] << 8) | (pData[10] << 16);

		// to do simplfy like mpeg1 m_SCR
		m_SCR = ((pData[8] & 0xf8) >> 3) | (pData[7] << 5) | ((pData[6] & 0x03) << 13) | (((pData[6] & 0xf8) >> 3) << 15) |
			(pData[5] << 20) | ((pData[4] & 0x03) << 28) | (((pData[4] & 0x38) >> 3) << 30);

		m_SCRExt = (pData[9] >> 1) | ((pData[8] & 0x03) << 7);

		WORD wStufByte = pData[13] & 0x07;
		//TRACE((L"wStufByte = %d\n", wStufByte));
		*pAte += wStufByte;
	}
	else if((pData[4] & 0xF1) == 0x21 && (pData[6] & 0x01) == 0x01 && (pData[8] & 0x01) == 0x01 && (pData[9] & 0x80) == 0x80 && (pData[11] & 0x01) == 0x01){

		//TRACE((L"MPEG1"));
		m_bMpeg1 = TRUE;
		*pAte = MPEG1_PACK_HEADER_SIZE;

		m_SCR = (pData[8] >> 1) | ((pData[7]) << 7) | ((pData[6] & 0xFE) << 14) | ((pData[5]) << 22) | ((pData[4] & 0x0E) << 29);
		m_muxRate = (pData[11] >> 1) | ((pData[10]) << 7) | ((pData[9] & 0x7F) << 15);
	}
	else{

		TRACE((L"ParsePackHeader Unknown format"));
		hr = E_FAIL;
	}

	m_dwPack++;

	//TRACE((L"m_bMpeg1 = %d\nm_muxRate = %d\nm_SCR = %I64d\nm_SCRExt = %d\n", m_bMpeg1, m_muxRate, m_SCR, m_SCRExt));

	return hr;
}

HRESULT CMFParser::ParseSystemHeader(const BYTE* pData, DWORD cbLen, DWORD* pAte){

	//TRACE((L"0x%08x\r", MAKE_DWORD(pData)));

	if(cbLen < MPEG2_SYSTEM_HEADER_MIN_SIZE){
		//TRACE((L"Parser MPEG2_SYSTEM_HEADER_MIN_SIZE"));
		return S_FALSE;
	}

	DWORD cbHeaderLen = MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX + MAKE_WORD(pData[4], pData[5]);

	if(cbHeaderLen < MPEG2_SYSTEM_HEADER_MIN_SIZE - MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX){
		TRACE((L"ParseSystemHeader cbHeaderLen ERROR"));
		return E_FAIL;
	}

	if(cbLen < cbHeaderLen){
		//TRACE((L"ParseSystemHeader cbHeaderLen"));
		return S_FALSE;
	}

	DWORD cStreamInfo = (cbHeaderLen - MPEG2_SYSTEM_HEADER_MIN_SIZE) / MPEG2_SYSTEM_HEADER_STREAM;

	if(((pData[6] & 0x80) != 0x80) || ((pData[8] & 0x01) != 0x01) || ((pData[10] & 0x20) != 0x20)){
		TRACE((L"ParseSystemHeader m_bMpeg1-2 marker bits ERROR"));
		return E_FAIL;
	}

	if(m_bMpeg1 && pData[11] != 0xFF){
		TRACE((L"ParseSystemHeader m_bMpeg1 marker bits ERROR"));
		return E_FAIL;
	}
	else if((pData[11] & 0x7F) != 0x7F){
		TRACE((L"ParseSystemHeader m_bMpeg2 marker bits ERROR"));
		return E_FAIL;
	}

	m_SysHeader.rateBound = ((pData[6] & 0x7F) << 15) | (pData[7] << 7) | (pData[8] >> 1);
	m_SysHeader.cAudioBound = pData[9] >> 2;
	m_SysHeader.bFixed = HAS_FLAG(pData[9], 0x02);
	m_SysHeader.bCSPS = HAS_FLAG(pData[9], 0x01);
	m_SysHeader.bAudioLock = HAS_FLAG(pData[10], 0x80);
	m_SysHeader.bVideoLock = HAS_FLAG(pData[10], 0x40);
	m_SysHeader.cVideoBound = pData[10] & 0x1F;

	/*TRACE((L"cStreamInfo = %d\rrateBound = %d\rcAudioBound = %d\rbFixed = %d\rbCSPS = %d\rbAudioLock = %d\rbVideoLock = %d\rcVideoBound = %d\r",
			cStreamInfo, m_SysHeader.rateBound, m_SysHeader.cAudioBound, m_SysHeader.bFixed, m_SysHeader.bCSPS,
			m_SysHeader.bAudioLock, m_SysHeader.bVideoLock, m_SysHeader.cVideoBound));*/

	m_bHasFoundSysHeader = TRUE;
	m_bHasSysHeader = TRUE;

	const BYTE* pStreamInfo = pData + MPEG2_SYSTEM_HEADER_MIN_SIZE;
	MPEG2StreamHeader StreamHeader;

	for(DWORD i = 0; i < cStreamInfo; i++){

		if(FAILED(ParseStreamData(pStreamInfo, StreamHeader))){
			TRACE((L"ParseSystemHeader ParseStreamData ERROR"));
			return E_FAIL;
		}

		pStreamInfo += MPEG2_SYSTEM_HEADER_STREAM;
	}

	*pAte = cbHeaderLen;

	return S_OK;
}

HRESULT CMFParser::ParsePacketHeaderMpeg1(const BYTE* pData, DWORD cbLen, DWORD* pAte){

	if(!m_bHasFoundSysHeader){
		TRACE((L"ParsePacketHeader no system header ERROR"));
		return E_FAIL;
	}

	if(cbLen < MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX){
		//TRACE((L"ParsePacketHeader MPEG1_PACKET_HEADER_MIN_SIZE"));
		return S_FALSE;
	}

	DWORD cbPacketLen = MAKE_WORD(pData[4], pData[5]) + MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;

	if(cbLen < cbPacketLen && cbLen < MPEG2_PACKET_HEADER_MAX_SIZE){
		//TRACE((L"ParsePacketHeader MPEG1_PACKET_HEADER_MAX_SIZE"));
		return S_FALSE;
	}

	BYTE id = 0;
	StreamType type = StreamType_Unknown;
	BYTE num = 0;

	ZeroMemory(&m_curPacketHeader, sizeof(m_curPacketHeader));

	id = pData[3];

	if(FAILED(ParseStreamId(id, &type, &num))){
		TRACE((L"ParsePacketHeader ParseStreamId ERROR"));
		return E_FAIL;
	}

	DWORD cbLeft = cbPacketLen - MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;
	pData = pData + MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;

	if(type != StreamType_Audio && type != StreamType_Video){

		m_curPacketHeader.stream_id = id;
		m_curPacketHeader.type = type;
		m_curPacketHeader.number = num;
		m_curPacketHeader.cbPacketSize = cbPacketLen;
		m_curPacketHeader.cbPayload = cbLeft;
		//m_curPacketHeader.bHasPTS = bHasPTS;
		//m_curPacketHeader.PTS = pts;
		//m_curPacketHeader.bHasDTS = bHasDTS;
		//m_curPacketHeader.DTS = dts;

		/*TRACE((L"stream_id = %d\rtype = %d\rnumber = %d\rcbPacketSize = %d\rcbPayload = %d\rbHasPTS = %d\rPTS = %d\r",
				m_curPacketHeader.stream_id, m_curPacketHeader.type, m_curPacketHeader.number, m_curPacketHeader.cbPacketSize,
				m_curPacketHeader.cbPayload, m_curPacketHeader.bHasPTS, m_curPacketHeader.PTS));*/

		m_bHasPacket = TRUE;

		*pAte = MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;

		return S_OK;
	}

	DWORD cbPadding = 0;
	BOOL bHasPTS = FALSE;
	BOOL bHasDTS = FALSE;
	LONGLONG pts = 0;
	LONGLONG dts = 0;

	while((cbLeft > 0) && (*pData == 0xFF)){

		assert(AdvanceBufferPointer(pData, cbLeft, 1) == S_OK);
		++cbPadding;
	}

	if(cbPadding > MPEG2_PACKET_HEADER_MAX_STUFFING_BYTE){
		TRACE((L"ParsePacketHeader MPEG2_PACKET_HEADER_MAX_STUFFING_BYTE ERROR"));
		return E_FAIL;
	}

	if(FAILED(ValidateBufferSize(cbLeft, 1))){
		TRACE((L"ParsePacketHeader ValidateBufferSize 1 ERROR"));
		return E_FAIL;
	}

	if((*pData & 0xC0) == 0x40){

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 2))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 1 ERROR"));
			return E_FAIL;
		}
	}

	if(FAILED(ValidateBufferSize(cbLeft, 1))){
		TRACE((L"ParsePacketHeader ValidateBufferSize 2 ERROR"));
		return E_FAIL;
	}

	if((*pData & 0xF1) == 0x21){

		// PTS
		if(FAILED(ValidateBufferSize(cbLeft, 5))){
			TRACE((L"ParsePacketHeader ValidateBufferSize 3 ERROR"));
			return E_FAIL;
		}

		ParsePTS(pData, &pts);
		bHasPTS = TRUE;

#ifdef TRACE_PTSDTS
		if(type == StreamType_Video)
			TRACE((L"ParsePTS = %I64d", pts * 10000 / 90));
#endif

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 5))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 2 ERROR"));
			return E_FAIL;
		}
	}
	else if((*pData & 0xF1) == 0x31){

		// PTS + DTS
		if(FAILED(ValidateBufferSize(cbLeft, 10))){
			TRACE((L"ParsePacketHeader ValidateBufferSize 4 ERROR"));
			return E_FAIL;
		}

		// Parse PTS but skip DTS. Perhaps need DTS, i don't know how to use it.
		ParseDTSPTS(pData, &pts, &dts);
		bHasPTS = TRUE;
		bHasDTS = TRUE;

#ifdef TRACE_PTSDTS
		if(type == StreamType_Video)
			TRACE((L"ParseDTSPTS = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
#endif

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 10))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 3 ERROR"));
			return E_FAIL;
		}
	}
	else if((*pData) == 0x0F){

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 1))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 4 ERROR"));
			return E_FAIL;
		}
	}
	else{

		// Some file are OK even if error here.
		TRACE((L"ParsePacketHeader Unexpected bit field ERROR"));
		//return E_FAIL;
	}

	m_curPacketHeader.stream_id = id;
	m_curPacketHeader.type = type;
	m_curPacketHeader.number = num;
	m_curPacketHeader.cbPacketSize = cbPacketLen;
	m_curPacketHeader.cbPayload = cbLeft;
	m_curPacketHeader.bHasPTS = bHasPTS;
	m_curPacketHeader.PTS = pts;
	m_curPacketHeader.bHasDTS = bHasDTS;
	m_curPacketHeader.DTS = dts;

	/*TRACE((L"\rstream_id = %d\rtype = %d\rnumber = %d\rcbPacketSize = %d\rcbPayload = %d\rbHasPTS = %d\rPTS = %d\r",
			m_curPacketHeader.stream_id, m_curPacketHeader.type, m_curPacketHeader.number, m_curPacketHeader.cbPacketSize,
			m_curPacketHeader.cbPayload, m_curPacketHeader.bHasPTS, m_curPacketHeader.PTS));*/

	m_bHasPacket = TRUE;

	*pAte = cbPacketLen - cbLeft;

#ifdef TRACE_OUTPUT_PTS
	if(type == StreamType_Video)
		TRACE((L"Video Mpeg1 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
	else if(type == StreamType_Audio)
		TRACE((L"Audio Mpeg1 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
	else
		TRACE((L"Other Mpeg1 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
#endif

	return S_OK;
}

HRESULT CMFParser::ParsePacketHeaderMpeg2(const BYTE* pData, DWORD cbLen, DWORD* pAte){

	if(!m_bHasFoundSysHeader){
		TRACE((L"ParsePacketHeader no system header ERROR\r"));
		return E_FAIL;
	}

	if(cbLen < MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX){
		//TRACE((L"ParsePacketHeader MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX\r"));
		return S_FALSE;
	}

	DWORD cbPacketLen = MAKE_WORD(pData[4], pData[5]) + MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;

	if(cbLen < cbPacketLen && cbLen < MPEG2_PACKET_HEADER_MAX_SIZE){
		//TRACE((L"ParsePacketHeader MPEG2_PACKET_HEADER_MAX_SIZE\r"));
		return S_FALSE;
	}

	BYTE id = 0;
	StreamType type = StreamType_Unknown;
	BYTE num = 0;

	ZeroMemory(&m_curPacketHeader, sizeof(m_curPacketHeader));

	id = pData[3];

	if(FAILED(ParseStreamId(id, &type, &num))){
		TRACE((L"ParsePacketHeader ParseStreamId ERROR\r"));
		return E_FAIL;
	}

	DWORD cbLeft = cbPacketLen - MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;
	pData = pData + MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX;

	/*if(type != StreamType_Video && type != StreamType_Audio && type != StreamType_Padding && type != StreamType_Reserved){

			MessageBox(NULL, L"StreamType ERROR", L"StreamType", MB_OK);
			return E_FAIL;
	}*/

	//if(type == StreamType_Padding || type == StreamType_Reserved){
	if(type != StreamType_Audio && type != StreamType_Video && type != StreamType_Private1){

		/*while((cbLeft > 0) && (*pData == 0xFF)){

				assert(AdvanceBufferPointer(pData, cbLeft, 1) == S_OK);
				++cbPadding;
		}*/

		m_curPacketHeader.stream_id = id;
		m_curPacketHeader.type = type;
		m_curPacketHeader.number = num;
		m_curPacketHeader.cbPacketSize = cbPacketLen;
		m_curPacketHeader.cbPayload = cbLeft;
		//m_curPacketHeader.bHasPTS = bHasPTS;
		//m_curPacketHeader.PTS = pts;
		//m_curPacketHeader.bHasDTS = bHasDTS;
		//m_curPacketHeader.DTS = dts;

		/*TRACE((L"stream_id = %d\rtype = %d\rnumber = %d\rcbPacketSize = %d\rcbPayload = %d\rbHasPTS = %d\rPTS = %d\r",
				m_curPacketHeader.stream_id, m_curPacketHeader.type, m_curPacketHeader.number, m_curPacketHeader.cbPacketSize,
				m_curPacketHeader.cbPayload, m_curPacketHeader.bHasPTS, m_curPacketHeader.PTS));*/

		m_bHasPacket = TRUE;

		*pAte = cbPacketLen - cbLeft;

		return S_OK;
	}

	DWORD cbPadding = 0;
	BOOL bHasPTS = FALSE;
	BOOL bHasDTS = FALSE;
	LONGLONG pts = 0;
	LONGLONG dts = 0;

	if(FAILED(ValidateBufferSize(cbLeft, 2))){
		TRACE((L"ParsePacketHeader ValidateBufferSize 1 ERROR\r"));
		return E_FAIL;
	}

	if((pData[0] & 0x80) != 0x80){
		TRACE((L"ParsePacketHeader 0x80 ERROR\r"));
		return E_FAIL;
	}

	// For further use
	BYTE PES_scrambling_control = (pData[0] & 0x30) >> 4;
	bool	bPES_priority = (pData[0] & 0x08 ? true : false);
	bool	bData_alignment_indicator = (pData[0] & 0x04 ? true : false);
	bool	bCopyright = (pData[0] & 0x02 ? true : false);
	bool	bOriginal_or_copy = pData[0] & 0x01;
	BYTE	bPTS_DTS_flags = (pData[1] & 0xc0) >> 6;
	bool	bESCR_flag = (pData[1] & 0x20 ? true : false);
	bool	bES_rate_flag = (pData[1] & 0x10 ? true : false);
	bool	bDSM_trick_mode_flag = (pData[1] & 0x08 ? true : false);
	bool	bAdditional_copy_info_flag = (pData[1] & 0x04 ? true : false);
	bool	bPES_CRC_flag = (pData[1] & 0x02 ? true : false);
	bool	bPES_extension_flag = pData[1] & 0x01;

	DWORD cbPTSLen = pData[2];

	if(FAILED(AdvanceBufferPointer(pData, cbLeft, 3))){
		TRACE((L"ParsePacketHeader AdvanceBufferPointer 1 ERROR\r"));
		return E_FAIL;
	}

	if(bPTS_DTS_flags == 0x0002){

		if(FAILED(ValidateBufferSize(cbLeft, 5))){
			TRACE((L"ParsePacketHeader ValidateBufferSize 3 ERROR\r"));
			return E_FAIL;
		}

		ParsePTS(pData, &pts);
		bHasPTS = TRUE;

#ifdef TRACE_PTSDTS
		if(type == StreamType_Video)
			TRACE((L"ParsePTS = %I64d", pts * 10000 / 90));
#endif

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 5))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 3 ERROR\r"));
			return E_FAIL;
		}
	}
	else if(bPTS_DTS_flags == 0x0003){

		if(FAILED(ValidateBufferSize(cbLeft, 10))){
			TRACE((L"ParsePacketHeader ValidateBufferSize 2 ERROR\r"));
			return E_FAIL;
		}

		ParseDTSPTS(pData, &pts, &dts);
		bHasPTS = TRUE;
		bHasDTS = TRUE;

#ifdef TRACE_PTSDTS
		if(type == StreamType_Video)
			TRACE((L"ParseDTSPTS = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
#endif

		if(FAILED(AdvanceBufferPointer(pData, cbLeft, 10))){
			TRACE((L"ParsePacketHeader AdvanceBufferPointer 2 ERROR\r"));
			return E_FAIL;
		}
	}
	else if(bPTS_DTS_flags != 0){

		TRACE((L"ParsePacketHeader bPTS_DTS_flags != 0 ERROR\r"));
		return E_FAIL;
	}

	//-------------------------------------------------------------------------------
	if(bESCR_flag){
		MessageBox(NULL, L"bESCR_flag", L"Flag", MB_OK);
		return E_FAIL;
	}

	if(bES_rate_flag){
		MessageBox(NULL, L"bES_rate_flag", L"Flag", MB_OK);
		return E_FAIL;
	}

	if(bDSM_trick_mode_flag){
		MessageBox(NULL, L"bDSM_trick_mode_flag", L"Flag", MB_OK);
		return E_FAIL;
	}

	if(bAdditional_copy_info_flag){
		MessageBox(NULL, L"bAdditional_copy_info_flag", L"Flag", MB_OK);
		return E_FAIL;
	}

	if(bPES_CRC_flag){
		MessageBox(NULL, L"bPES_CRC_flag", L"Flag", MB_OK);
		return E_FAIL;
	}
	//-------------------------------------------------------------------------------

	if(bPES_extension_flag){

		if(FAILED(ValidateBufferSize(cbLeft, 1))){
			TRACE((L"ParsePacketHeader ValidateBufferSize 4 ERROR\r"));
			return E_FAIL;
		}

		bool bPES_private_data_flag = (pData[0] & 0x80 ? true : false);
		bool bPack_header_field_flag = (pData[0] & 0x40 ? true : false);
		bool bProgram_packet_sequence_counter_flag = (pData[0] & 0x20 ? true : false);
		bool bP_STD_buffer_flag = (pData[0] & 0x10 ? true : false);
		// reserved : pData[0] & 0x0E
		bool bPES_extension_flag_2 = pData[0] & 0x01;

		//-------------------------------------------------------------------------------
		if(bPES_private_data_flag){
			MessageBox(NULL, L"bPES_private_data_flag", L"Flag", MB_OK);
			return E_FAIL;
		}

		if(bPack_header_field_flag){
			MessageBox(NULL, L"bPack_header_field_flag", L"Flag", MB_OK);
			return E_FAIL;
		}

		if(bProgram_packet_sequence_counter_flag){
			MessageBox(NULL, L"bProgram_packet_sequence_counter_flag", L"Flag", MB_OK);
			return E_FAIL;
		}

		if(bPES_extension_flag_2){
			MessageBox(NULL, L"bPES_extension_flag_2", L"Flag", MB_OK);
			return E_FAIL;
		}
		//-------------------------------------------------------------------------------
	}

	/*while(pData[0] == 0xff){

			if(FAILED(AdvanceBufferPointer(pData, cbLeft, 1))){
					TRACE((L"ParsePacketHeader AdvanceBufferPointer 3 ERROR\r"));
					return E_FAIL;
			}
	}*/

	m_curPacketHeader.stream_id = id;
	m_curPacketHeader.type = type;
	m_curPacketHeader.number = num;
	m_curPacketHeader.cbPacketSize = cbPacketLen;
	m_curPacketHeader.cbPayload = cbLeft;
	m_curPacketHeader.bHasPTS = bHasPTS;
	m_curPacketHeader.PTS = pts;
	m_curPacketHeader.bHasDTS = bHasDTS;
	m_curPacketHeader.DTS = dts;

	m_bHasPacket = TRUE;

	DWORD dwSizeAte = 9 + cbPTSLen;

	m_curPacketHeader.cbPayload = cbPacketLen - dwSizeAte;

	*pAte = dwSizeAte;

	/*while(pData[0] == 0xff){

			if(FAILED(ValidateBufferSize(cbLeft, 1))){
					TRACE((L"ParsePacketHeader ValidateBufferSize 5 ERROR\r"));
					return E_FAIL;
			}

			if(FAILED(AdvanceBufferPointer(pData, cbLeft, 1))){
					TRACE((L"ParsePacketHeader AdvanceBufferPointer 3 ERROR\r"));
					return E_FAIL;
			}

			m_curPacketHeader.cbPayload--;
	}

	*pAte = cbPacketLen - cbLeft;*/

#ifdef TRACE_OUTPUT_PTS
	if(type == StreamType_Video)
		TRACE((L"Video Mpeg2 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
	else if(type == StreamType_Audio)
		TRACE((L"Audio Mpeg2 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
	else
		TRACE((L"Other Mpeg2 = %I64d - %I64d", pts * 10000 / 90, dts * 10000 / 90));
#endif

	return S_OK;
}

HRESULT CMFParser::ParseStreamData(const BYTE* pStreamInfo, MPEG2StreamHeader& header){

	if((pStreamInfo[1] & 0xC0) != 0xC0){
		TRACE((L"ParseStreamData marker bits ERROR"));
		return E_FAIL;
	}

	BYTE id = 0;
	BYTE num = 0;
	DWORD bound = 0;
	StreamType type = StreamType_Unknown;

	id = pStreamInfo[0];

	if(FAILED(ParseStreamId(id, &type, &num))){
		TRACE((L"ParseStreamData ParseStreamId ERROR"));
		return E_FAIL;
	}

	bound = pStreamInfo[2] | ((pStreamInfo[1] & 0x1F) << 8);

	if(pStreamInfo[1] & 0x20){
		bound *= 1024;
	}
	else{
		bound *= 128;
	}

	header.stream_id = id;
	header.type = type;
	header.number = num;
	header.sizeBound = bound;

	return S_OK;
}

HRESULT CMFParser::ParseStreamId(BYTE id, StreamType* pType, BYTE* pStreamNum){

	switch(id){

	case MPEG2_STREAMTYPE_ALL_AUDIO:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_ALL_AUDIO"));
		*pType = StreamType_AllAudio;
		break;

	case MPEG2_STREAMTYPE_ALL_VIDEO:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_ALL_VIDEO"));
		*pType = StreamType_AllVideo;
		break;

	case MPEG2_STREAMTYPE_RESERVED:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_RESERVED"));
		*pType = StreamType_Reserved;
		break;

	case MPEG2_STREAMTYPE_PRIVATE1:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_PRIVATE1"));
		*pType = StreamType_Private1;
		break;

	case MPEG2_STREAMTYPE_PADDING:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_PADDING"));
		*pType = StreamType_Padding;
		break;

	case MPEG2_STREAMTYPE_PRIVATE2:
		//TRACE((L"stream ID: MPEG1_STREAMTYPE_PRIVATE2"));
		*pType = StreamType_Private2;
		break;

	default:
		if((id & 0xE0) == MPEG2_STREAMTYPE_AUDIO_MASK){
			//TRACE((L"stream ID: MPEG1_STREAMTYPE_AUDIO_MASK"));
			*pType = StreamType_Audio;
			*pStreamNum = id & 0x1F;
		}
		else if((id & 0xF0) == MPEG2_STREAMTYPE_VIDEO_MASK){
			//TRACE((L"stream ID: MPEG1_STREAMTYPE_VIDEO_MASK"));
			*pType = StreamType_Video;
			*pStreamNum = id & 0x0F;
		}
		else if((id & 0xF0) == MPEG2_STREAMTYPE_DATA_MASK){
			//TRACE((L"stream ID: MPEG1_STREAMTYPE_DATA_MASK"));
			*pType = StreamType_Data;
			*pStreamNum = id & 0x0F;
		}
		else{
			TRACE((L"ParseStreamId : Unknown stream ID: %d", id));
			return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CMFParser::ParsePTS(const BYTE* pData, LONGLONG* pPTS){

	BYTE byte1 = pData[0];
	WORD word1 = MAKE_WORD(pData[1], pData[2]);
	WORD word2 = MAKE_WORD(pData[3], pData[4]);

	// Check marker bits. The first byte can be '0010xxx1' or '0x11xxxx1'
	if(((byte1 & 0xF1) != 0x21) ||
		((word1 & 0x01) != 0x01) ||
		((word2 & 0x01) != 0x01))
	{
		// Todo some Marker are wrong but file is ok to play
		//TRACE((L"ParsePTS marker bits ERROR"));
		//return E_FAIL;
	}

	LARGE_INTEGER li;

	li.HighPart = (byte1 & 0x08) >> 3;

	li.LowPart = (static_cast<DWORD>(byte1 & 0x06) << 29) | (static_cast<DWORD>(word1 & 0xFFFE) << 14) | (static_cast<DWORD>(word2) >> 1);

	*pPTS = li.QuadPart;

	return S_OK;
}

HRESULT CMFParser::ParseDTSPTS(const BYTE* pData, LONGLONG* pPTS, LONGLONG* pDTS){

	BYTE byte1 = pData[0];
	WORD word1 = MAKE_WORD(pData[1], pData[2]);
	WORD word2 = MAKE_WORD(pData[3], pData[4]);

	// Check marker bits. The first byte can be '0010xxx1' or '0x11xxxx1'
	if(((byte1 & 0xF1) != 0x31) ||
		((word1 & 0x01) != 0x01) ||
		((word2 & 0x01) != 0x01))
	{
		// Todo some Marker are wrong in mpeg1 check also here
		//TRACE((L"ParseDTSPTS marker bits ERROR"));
		//return E_FAIL;
	}

	LARGE_INTEGER li;

	li.HighPart = (byte1 & 0x08) >> 3;

	li.LowPart = (static_cast<DWORD>(byte1 & 0x06) << 29) | (static_cast<DWORD>(word1 & 0xFFFE) << 14) | (static_cast<DWORD>(word2) >> 1);

	*pPTS = li.QuadPart;

	byte1 = pData[5];
	word1 = MAKE_WORD(pData[6], pData[7]);
	word2 = MAKE_WORD(pData[8], pData[9]);

	if(((byte1 & 0xF1) != 0x31) ||
		((word1 & 0x01) != 0x01) ||
		((word2 & 0x01) != 0x01))
	{
		// Todo some Marker are wrong in mpeg1 check also here
		//TRACE((L"ParseDTSPTS marker bits ERROR"));
		//return E_FAIL;
	}

	li.HighPart = (byte1 & 0x08) >> 3;

	li.LowPart = (static_cast<DWORD>(byte1 & 0x06) << 29) | (static_cast<DWORD>(word1 & 0xFFFE) << 14) | (static_cast<DWORD>(word2) >> 1);

	*pDTS = li.QuadPart;

	return S_OK;
}