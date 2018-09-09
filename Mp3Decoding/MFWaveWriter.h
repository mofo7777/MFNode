//----------------------------------------------------------------------------------------------
// MFWaveWriter.h
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
#ifndef MFWAVEWRITER_H
#define MFWAVEWRITER_H

const UINT32 WAVE_HEAD_LEN = 44;

#define SWAP32(val) (UINT32)((((UINT32)(val)) & 0x000000FF)<<24|	\
					                        (((UINT32)(val)) & 0x0000FF00)<<8 |	\
					                        (((UINT32)(val)) & 0x00FF0000)>>8 |	\
					                        (((UINT32)(val)) & 0xFF000000)>>24)

#pragma pack(push, 1)

struct RIFFCHUNK{

	UINT32 fcc;
	UINT32 cb;
};

struct RIFFLIST{

	UINT32 fcc;
	UINT32 cb;
	UINT32 fccListType;

};

struct WAVEFORM{

	UINT32 fcc;
	UINT32 cb;
	UINT16 wFormatTag; 
	UINT16 nChannels; 
	UINT32 nSamplesPerSec; 
	UINT32 nAvgBytesPerSec; 
	UINT16 nBlockAlign; 
	UINT16 wBitsPerSample; 
};

#pragma pack(pop)

class CMFWaveWriter{

public:

	CMFWaveWriter() : m_hFile(INVALID_HANDLE_VALUE){}
	~CMFWaveWriter(){ CLOSE_HANDLE_IF(m_hFile); }

	BOOL Initialize(const WCHAR*);
	BOOL WriteWaveData(const BYTE*, const DWORD);
	BOOL FinalizeHeader(const WAVEFORM&, const UINT32);

private:

	HANDLE m_hFile;

	BOOL SetWaveHeader(const WAVEFORM&, const UINT32, BYTE*);
};

#endif