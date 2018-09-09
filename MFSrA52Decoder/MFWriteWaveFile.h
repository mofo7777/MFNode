//----------------------------------------------------------------------------------------------
// MFWriteWaveFile.h
//----------------------------------------------------------------------------------------------
#ifndef MFWRITEWAVEFILE_H
#define MFWRITEWAVEFILE_H

class CMFWriteWaveFile{

public:

	CMFWriteWaveFile() : m_hFile(INVALID_HANDLE_VALUE){}
	~CMFWriteWaveFile(){ if(m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_hFile); }

	BOOL MFCreateFile(const WCHAR* wszFile){

		BOOL bRet = FALSE;

		if(m_hFile != INVALID_HANDLE_VALUE){
			CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
		}

		m_hFile = CreateFile(wszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		if(m_hFile == INVALID_HANDLE_VALUE){
			return bRet;
		}

		return bRet = TRUE;
	}

	BOOL MFWriteFile(const void* pData, const DWORD dwLength){

		BOOL bRet = FALSE;
		DWORD dwWritten;

		if(!WriteFile(m_hFile, (LPCVOID)pData, dwLength, &dwWritten, 0) || dwWritten != dwLength){
			return bRet;
		}

		return bRet = TRUE;
	}

	BOOL WriteWaveHeader(const DWORD, const UINT16, const UINT32);

	void Close(){ if(m_hFile != INVALID_HANDLE_VALUE){ CloseHandle(m_hFile); m_hFile = INVALID_HANDLE_VALUE; } }

	const BOOL IsOpened() const{ return m_hFile != INVALID_HANDLE_VALUE; }

private:

	HANDLE m_hFile;

	struct WAVEFORM{

		UINT32 uiDataSize;
		UINT32 uiChunkSize;
		UINT16 usFormatTag;
		UINT16 usChannels;
		UINT32 uiSamplesPerSec;
		UINT32 uiAvgBytesPerSec;
		UINT16 usBlockAlign;
		UINT16 usBitsPerSample;

		UINT16 usChunkSizeEx;
		UINT16 usValidBits;
		UINT32 uiChannelMask;
		BYTE SubFormat[16];
	};
};

#endif