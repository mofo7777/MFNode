//----------------------------------------------------------------------------------------------
// FlvParser.h
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
#ifndef FLVPARSER_H
#define FLVPARSER_H

class CFlvParser{

public:

	CFlvParser();
	~CFlvParser(){ TRACE_PARSER((L"Parser::DTOR")); }

	HRESULT ParseHeader(BYTE*);
	HRESULT ParseBytes(BYTE*, const DWORD, DWORD*, DWORD*);
	HRESULT ParseBytesStreaming(BYTE*, const DWORD, DWORD*, DWORD*);

	BOOL HasSystemHeader(){ return m_FlvHeader.btVersion != 0x00; }
	BOOL HasVideoInfo(){ return m_FlvVideoHeader.bHasVideoInfo; }
	BOOL HasAudioInfo(){ return m_FlvAudioHeader.bHasAudioInfo; }

	BOOL HasPacket(){ return m_FlvTag.bHasPacket; }
	DWORD GetTimeStamp(){ return m_FlvTag.dwTimeStamp; }
	DWORD GetComposition(){ return m_FlvVideoHeader.dwCompositionTime; }
	DWORD PayloadSize(){ return m_FlvTag.dwPayloadSize; }
	BYTE GetPacketType(){ return m_FlvTag.btFlvTag; }
	BYTE GetAudioFmt(){ return m_FlvAudioHeader.btSoundFMT; }
	BYTE GetVideoFmt(){ return m_FlvVideoHeader.btVideoCodec; }
	BYTE* GetVideoRecord(){ return m_FlvVideoHeader.pVideoRecord; }
	void ClearPacket(){ m_FlvTag.bHasPacket = FALSE; }
	void ClearInfo(){ m_FlvVideoHeader.bHasVideoInfo = FALSE; m_FlvAudioHeader.bHasAudioInfo = FALSE; }
	void Clear(){ m_dwDebugLastSize = 0; m_FlvHeader.btVersion = 0x00; m_FlvTag.bHasPacket = FALSE; }
	//void ClearVideoInfo(){ m_FlvVideoHeader.bHasVideoInfo = FALSE; }
	//void ClearAudioInfo(){ m_FlvAudioHeader.bHasAudioInfo = FALSE; }

	FLV_AUDIO_HEADER* GetAudioTag(){ return &m_FlvAudioHeader; }
	FLV_VIDEO_HEADER* GetVideoTag(){ return &m_FlvVideoHeader; }

	const BOOL IsKeyFrame() const{ return m_FlvVideoHeader.bIsKeyFrame; }

private:

	FLV_HEADER m_FlvHeader;
	FLV_TAG m_FlvTag;
	FLV_VIDEO_HEADER m_FlvVideoHeader;
	FLV_AUDIO_HEADER m_FlvAudioHeader;

	DWORD m_dwDebugLastSize;

	HRESULT ReadVideoTag(BYTE*, const DWORD, DWORD*);
	HRESULT ReadVideoTagStreaming(BYTE*, const DWORD, DWORD*);
	HRESULT ReadAudioTag(BYTE*, const DWORD, DWORD*);
	HRESULT CheckNoVlc(BYTE*, DWORD*, DWORD*);

	enum Mpeg12AudioLayer{

		Mpeg12_Audio_Layer1 = 0,
		Mpeg12_Audio_Layer2,
		Mpeg12_Audio_Layer3
	};

	struct Mpeg12AudioFrameHeader{

		BYTE btVersion;
		BYTE btLayer;
		BOOL bProtection;
		BYTE btBitrateIndex;
		BYTE btSamplingIndex;
		BOOL bPadding;
		BOOL bPrivate;
		BYTE btChannelMode;
		BYTE btModeExtension;
		BOOL bCopyright;
		BOOL bOriginal;
		BYTE btEmphasis;

		Mpeg12AudioLayer Layer;
		UINT uiChannel;
		UINT uiSamplePerSec;
		UINT uiBitrate;
	};

	HRESULT GetAudioBitRateMpeg1(Mpeg12AudioLayer, BYTE, UINT*);
	HRESULT GetAudioBitRateMpeg2(Mpeg12AudioLayer, BYTE, UINT*);
	HRESULT GetSamplingFrequency(const BYTE, const BYTE, UINT*);
};

#endif