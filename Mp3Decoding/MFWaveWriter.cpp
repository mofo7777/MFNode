//----------------------------------------------------------------------------------------------
// MFWaveWriter.cpp
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
#include "Stdafx.h"

BOOL CMFWaveWriter::Initialize(const WCHAR* wszFile){

		BOOL bRet = FALSE;
		CLOSE_HANDLE_IF(m_hFile);

		m_hFile = CreateFile(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if(m_hFile == INVALID_HANDLE_VALUE){
				IF_ERROR_RETURN(bRet);
		}

		BYTE WavHeader[WAVE_HEAD_LEN];
		memset(WavHeader, 0, sizeof(WavHeader));

		DWORD dwWritten;
		
		if(!WriteFile(m_hFile, (LPCVOID)WavHeader, WAVE_HEAD_LEN, &dwWritten, 0) || dwWritten != WAVE_HEAD_LEN){

				IF_ERROR_RETURN(bRet);
  }

		return bRet = TRUE;
}

BOOL CMFWaveWriter::WriteWaveData(const BYTE* pData, const DWORD dwLength){

		BOOL bRet = FALSE;
		DWORD dwWritten;

		if(!WriteFile(m_hFile, (LPCVOID)pData, dwLength, &dwWritten, 0) || dwWritten != dwLength){

				IF_ERROR_RETURN(bRet);
  }

		return bRet = TRUE;
}

BOOL CMFWaveWriter::FinalizeHeader(const WAVEFORM& waveData, const UINT32 uiFileLength){

		BOOL bRet = FALSE;
		DWORD dwMove;
		DWORD dwWritten;

		BYTE WavHeader[WAVE_HEAD_LEN];
		memset(WavHeader, 0, sizeof(WavHeader));

		if((dwMove = SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN)) == INVALID_SET_FILE_POINTER){
				IF_ERROR_RETURN(bRet);
		}

		if(!SetWaveHeader(waveData, uiFileLength, WavHeader)){
				IF_ERROR_RETURN(bRet);
  }

		if(!WriteFile(m_hFile, (LPCVOID)WavHeader, WAVE_HEAD_LEN, &dwWritten, 0) || dwWritten != WAVE_HEAD_LEN){
				IF_ERROR_RETURN(bRet);
  }

		return bRet = TRUE;
}

BOOL CMFWaveWriter::SetWaveHeader(const WAVEFORM& waveData, const UINT32 uiDataLen, BYTE* head){

		if(uiDataLen == 0)
				return FALSE;

		RIFFCHUNK* pch;
		RIFFLIST  *priff;
		WAVEFORM *pwave;

		memset(head, 0x00, WAVE_HEAD_LEN);

		priff=(RIFFLIST*)head;
		priff->fcc=SWAP32('RIFF');
		
		priff->cb              = uiDataLen + WAVE_HEAD_LEN - sizeof(RIFFCHUNK);
		priff->fccListType     = SWAP32('WAVE');

		pwave                  = (WAVEFORM*)(priff + 1);
		pwave->fcc             = SWAP32('fmt ');
		pwave->cb              = sizeof(WAVEFORM) - sizeof(RIFFCHUNK);
		pwave->wFormatTag      = WAVE_FORMAT_PCM;
		pwave->nChannels       = waveData.nChannels;//(UINT16)uiChannel;
		pwave->nSamplesPerSec  = waveData.nSamplesPerSec;//uiSampleRate;
		pwave->nAvgBytesPerSec = waveData.nAvgBytesPerSec;//pwave->nSamplesPerSec * uiChannel * 2;
		pwave->nBlockAlign     = waveData.nBlockAlign;//UINT16(uiChannel * 2);
		pwave->wBitsPerSample  = waveData.wBitsPerSample;//16;

		pch                    = (RIFFCHUNK*)(pwave + 1);
		pch->fcc               = SWAP32('data');
		pch->cb                = uiDataLen;

		return TRUE;
}