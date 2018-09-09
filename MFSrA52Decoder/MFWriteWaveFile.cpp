//----------------------------------------------------------------------------------------------
// MFWriteWaveFile.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

BOOL CMFWriteWaveFile::WriteWaveHeader(const DWORD dwTotalSize, const UINT16 uiChannels, const UINT32 uiSamplesPerSec){

		_llseek((HFILE)m_hFile, 0, 0);

		BOOL bOK = FALSE;
		DWORD dwWritten = 0;
		DWORD dwTmp = 0;

		WAVEFORM OutputHeader;
		ZeroMemory(&OutputHeader, sizeof(OutputHeader));

		OutputHeader.uiDataSize = dwTotalSize;
		OutputHeader.uiChunkSize = 40;
		OutputHeader.usFormatTag = 0xFFFE;//WAVE_FORMAT_EXTENSIBLE;
		OutputHeader.usChannels = uiChannels;
		OutputHeader.uiSamplesPerSec = uiSamplesPerSec;
		OutputHeader.usBlockAlign = uiChannels * 32 / 8;
		OutputHeader.uiAvgBytesPerSec = OutputHeader.usBlockAlign * uiSamplesPerSec;
		OutputHeader.usBitsPerSample = 32;
		OutputHeader.usChunkSizeEx = 22;
		OutputHeader.usValidBits = 32;
		OutputHeader.uiChannelMask = 3;

		OutputHeader.SubFormat[0] = 0x03;
		OutputHeader.SubFormat[1] = 0x00;
		OutputHeader.SubFormat[2] = 0x00;
		OutputHeader.SubFormat[3] = 0x00;
		OutputHeader.SubFormat[4] = 0x00;
		OutputHeader.SubFormat[5] = 0x00;
		OutputHeader.SubFormat[6] = 0x10;
		OutputHeader.SubFormat[7] = 0x00;
		OutputHeader.SubFormat[8] = 0x80;
		OutputHeader.SubFormat[9] = 0x00;
		OutputHeader.SubFormat[10] = 0x00;
		OutputHeader.SubFormat[11] = 0xaa;
		OutputHeader.SubFormat[12] = 0x00;
		OutputHeader.SubFormat[13] = 0x38;
		OutputHeader.SubFormat[14] = 0x9b;
		OutputHeader.SubFormat[15] = 0x71;

		bOK = WriteFile(m_hFile, (LPCVOID)"RIFF", 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		dwTmp = OutputHeader.uiDataSize + 72;
		bOK = WriteFile(m_hFile, (LPCVOID)&dwTmp, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)"WAVE", 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)"fmt ", 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.uiChunkSize, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usFormatTag, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usChannels, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.uiSamplesPerSec, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.uiAvgBytesPerSec, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usBlockAlign, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usBitsPerSample, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usChunkSizeEx, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.usValidBits, 2, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.uiChannelMask, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)OutputHeader.SubFormat, 16, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)"fact", 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		dwTmp = 4;
		bOK = WriteFile(m_hFile, (LPCVOID)&dwTmp, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		dwTmp = OutputHeader.uiDataSize / OutputHeader.usBlockAlign;
		bOK = WriteFile(m_hFile, (LPCVOID)&dwTmp, 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)"data", 4, &dwWritten, 0);

		if(bOK == FALSE)
				return bOK;

		bOK = WriteFile(m_hFile, (LPCVOID)&OutputHeader.uiDataSize, 4, &dwWritten, 0);

		return bOK;
}