//----------------------------------------------------------------------------------------------
// FlvDefinition.h
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
#ifndef FLVDEFINITION_H
#define FLVDEFINITION_H

#define FLV_TAG_AUDIO  0x08
#define FLV_TAG_VIDEO  0x09
#define FLV_TAG_SCRIPT 0x12

#define FLV_SYSTEM_HEADER 9
#define FLV_TAG_MINSIZE  15

#define FLV_KEY_FRAME       1
#define FLV_INTER_FRAME     2
#define FLV_DISINTER_FRAME  3
#define FLV_GENKEY_FRAME    4
#define FLV_INFO_FRAME      5

#define FLV_VIDEOCODEC_H263         2
#define FLV_VIDEOCODEC_SCREEN       3
#define FLV_VIDEOCODEC_VP6          4
#define FLV_VIDEOCODEC_VP6A         5
#define FLV_VIDEOCODEC_SCREEN2      6
#define FLV_VIDEOCODEC_AVC          7

#define FLV_SEQUENCE_AVC         0
#define FLV_SEQUENCE_NALU        1
#define FLV_ENDSEQUENCE_AVC      3

#define FLV_AUDIOCODEC_PCM           0
#define FLV_AUDIOCODEC_ADPCM         1
#define FLV_AUDIOCODEC_MP3           2
#define FLV_AUDIOCODEC_LPCM          3
#define FLV_AUDIOCODEC_NELLY16       4
#define FLV_AUDIOCODEC_NELLY8        5
#define FLV_AUDIOCODEC_NELLY         6
#define FLV_AUDIOCODEC_G711A         7
#define FLV_AUDIOCODEC_G711M         8
#define FLV_AUDIOCODEC_AAC          10
#define FLV_AUDIOCODEC_SPEEX        11
#define FLV_AUDIOCODEC_MP38         14
#define FLV_AUDIOCODEC_SPECIFIC     15

#define FLV_AUDIO_ID  0
#define FLV_VIDEO_ID  1

// Todo ; don't know max VideoRecord size... use pointer...
#define MAX_VIDEO_RECORED_SIZE 128

struct FLV_HEADER{
		
		BYTE btVersion;
		BOOL bVideoPresent;
		BOOL bAudioPresent;
};

struct FLV_TAG{
		
		BYTE btFlvTag;
		BOOL bHasPacket;
		BOOL bFilter;
		DWORD dwPreviousTagSize;
		DWORD dwDataSize;
		DWORD dwTimeStamp;
		DWORD dwStreamID;
		DWORD dwPayloadSize;
};

struct FLV_VIDEO_HEADER{

		BYTE btVideoFrame;
		BYTE btVideoCodec;
		BYTE btAVPacketType;
		DWORD dwCompositionTime;
		BOOL bHasVideoInfo;
		BOOL bIsKeyFrame;

		DWORD dwConfigurationVersion;
		BYTE btAvcProfileIndication;
		BYTE btProfileCompatibility;
		BYTE btAvcLevelIndication;

		BYTE btLengthSizeMinusOne;
		WORD wNumberOfSequenceParameterSets;
		WORD wSequenceParameterSetLength;
		BYTE btPictureParameterSets;
		WORD wPictureParameterSetLength;

		UINT32 uiWidth;
		UINT32 uiHeight;

		BYTE pVideoRecord[MAX_VIDEO_RECORED_SIZE];
};

struct FLV_AUDIO_HEADER{

		BYTE btAACHeader[2];
		BYTE btSoundFMT;
		UINT uiSampleRate;
		UINT uiBitrate;
		WORD wBitSample;
		WORD wChannel;
		WORD wFormatTag;
		BOOL bStereo;
		BOOL bHasAudioInfo;
};

const UINT g_uiSampleRates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350};
const WORD g_wChannels[] = {0, 1, 2, 3, 4, 5, 6, 8};

inline DWORD MAKE_24DWORD(const BYTE* pData){
		
		return (pData[0] << 16) | (pData[1] << 8) | pData[2];
}

const DWORD FLV_READ_SIZE = 4096;

#endif