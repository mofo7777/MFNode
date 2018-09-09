//----------------------------------------------------------------------------------------------
// MFMpeg2Definition.h
//----------------------------------------------------------------------------------------------
#ifndef MFMPEG2DEFINITION_H
#define MFMPEG2DEFINITION_H

#define MAX_VIDEO_WIDTH_HEIGHT        4095
#define MIN_BYTE_TO_READ_MPEG_HEADER  12
#define MAX_SLICE                     175 // (1 to 175 : 0x00000001 to 0x000001AF)
#define SLICE_BUFFER_SIZE             32768

const DWORD D3DFMT_NV12 = MAKEFOURCC('N', 'V', '1', '2');
const DWORD D3DFMT_AYUV = MAKEFOURCC('A', 'Y', 'U', 'V');
const DWORD D3DFMT_YV12 = MAKEFOURCC('Y', 'V', '1', '2');
const DWORD D3DFMT_AIP8 = MAKEFOURCC('A', 'I', 'P', '8');
const DWORD D3DFMT_AI44 = MAKEFOURCC('A', 'I', '4', '4');

enum StreamType{

		StreamType_Unknown,
		StreamType_AllAudio,
		StreamType_AllVideo,
		StreamType_Reserved,
		StreamType_Private1,
		StreamType_Padding,
		StreamType_Private2,
		StreamType_Audio,
		StreamType_Video,
		StreamType_Data
};

struct MPEG2StreamHeader{

		BYTE stream_id;  // Raw stream_id field.
		StreamType type; // Stream type (audio, video, etc)
		BYTE number;     // Index within the stream type (audio 0, audio 1, etc)
		DWORD sizeBound;
};

struct MPEG2SystemHeader{

		DWORD   rateBound;
		BYTE    cAudioBound;
		BOOL    bFixed;
		BOOL    bCSPS;
		BOOL    bAudioLock;
		BOOL    bVideoLock;
		BYTE    cVideoBound;
		MPEG2StreamHeader streams[2];
};

struct MPEG2PacketHeader{

		BYTE        stream_id;      // Raw stream_id field.
		StreamType  type;           // Stream type (audio, video, etc)
		BYTE        number;         // Index within the stream type (audio 0, audio 1, etc)
		DWORD       cbPacketSize;   // Size of the entire packet (header + payload).
		DWORD       cbPayload;      // Size of the packet payload (packet size - header size).
		BOOL        bHasPTS;        // Did the packet header contain a Presentation Time Stamp (PTS)?
		LONGLONG    PTS;            // Presentation Time Stamp (in 90 kHz clock)
		BOOL        bHasDTS;
		LONGLONG    DTS;
};

const DWORD MPEG1_PACK_HEADER_SIZE = 12;
const DWORD MPEG2_PACK_HEADER_SIZE = 14;
const DWORD MPEG2_SYSTEM_HEADER_MIN_SIZE = 12;       // System header, excluding the stream info.
const DWORD MPEG2_SYSTEM_AND_PACK_HEADER_PREFIX = 6; // This value + header length = total size of the system header or the pack header.
const DWORD MPEG2_SYSTEM_HEADER_STREAM = 3;          // Size of each stream info in the system header.

//--------------------------------------------------------------------------------------------------------------
// Todo check value
const DWORD MPEG2_PACKET_HEADER_MAX_SIZE = 34;          // Maximum size of a packet header.
const DWORD MPEG2_PACKET_HEADER_MAX_STUFFING_BYTE = 16; // Maximum number of stuffing bytes in a packet header.

//const DWORD MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE = 12;       // Minimum length of the video sequence header.
const DWORD MPEG1_VIDEO_SEQ_HEADER_MAX_SIZE = 140;      // Maximum length of the video sequence header.
const DWORD MPEG2_AUDIO_FRAME_HEADER_SIZE = 4;
const DWORD AC3_AUDIO_FRAME_HEADER_SIZE = 7;
//--------------------------------------------------------------------------------------------------------------

struct MPEG2VideoSeqHeader{

		WORD    width;
		WORD    height;
		MFRatio pixelAspectRatio;
		MFRatio frameRate;
		DWORD   bitRate;
		WORD    cbVBV_Buffer;
		BOOL    bConstrained;
		DWORD   cbHeader;
		BYTE    header[MPEG1_VIDEO_SEQ_HEADER_MAX_SIZE];
};

enum MPEG2AudioLayer{

		MPEG2_Audio_Layer1 = 0,
		MPEG2_Audio_Layer2,
		MPEG2_Audio_Layer3
};

enum MPEG2AudioMode{

		MPEG2_Audio_Stereo = 0,
		MPEG2_Audio_JointStereo,
		MPEG2_Audio_DualChannel,
		MPEG2_Audio_SingleChannel
};

enum MPEG2AudioFlags{

		MPEG2_AUDIO_PRIVATE_BIT = 0x01,  // = ACM_MPEG_PRIVATEBIT
		MPEG2_AUDIO_COPYRIGHT_BIT = 0x02,  // = ACM_MPEG_COPYRIGHT
		MPEG2_AUDIO_ORIGINAL_BIT = 0x04,  // = ACM_MPEG_ORIGINALHOME
		MPEG2_AUDIO_PROTECTION_BIT = 0x08,  // = ACM_MPEG_PROTECTIONBIT
};

struct MPEG2AudioFrameHeader{

		MPEG2AudioLayer layer;
		DWORD           dwBitRate;
		DWORD           dwSamplesPerSec;
		WORD            nBlockAlign;
		WORD            nChannels;
		MPEG2AudioMode  mode;
		BYTE            modeExtension;
		BYTE            emphasis;
		WORD            wFlags;
};

// Codes
const DWORD MPEG2_START_CODE_PREFIX = 0x00000100;
const DWORD MPEG2_PACK_START_CODE = 0x000001BA;
const DWORD MPEG2_SYSTEM_HEADER_CODE = 0x000001BB;
const DWORD MPEG2_SEQUENCE_HEADER_CODE = 0x000001B3;
const DWORD MPEG2_STOP_CODE = 0x000001B9;

// Stream ID codes
const BYTE MPEG2_STREAMTYPE_ALL_AUDIO = 0xB8;
const BYTE MPEG2_STREAMTYPE_ALL_VIDEO = 0xB9;
const BYTE MPEG2_STREAMTYPE_RESERVED = 0xBC;
const BYTE MPEG2_STREAMTYPE_PRIVATE1 = 0xBD;
const BYTE MPEG2_STREAMTYPE_PADDING = 0xBE;
const BYTE MPEG2_STREAMTYPE_PRIVATE2 = 0xBF;
const BYTE MPEG2_STREAMTYPE_AUDIO_MASK = 0xC0;
const BYTE MPEG2_STREAMTYPE_VIDEO_MASK = 0xE0;
const BYTE MPEG2_STREAMTYPE_DATA_MASK = 0xF0;

const DWORD MPEG_PICTURE_START_CODE = 0x00000100;
const DWORD MPEG_SEQUENCE_HEADER_CODE = 0x000001B3;
const DWORD MPEG_SEQUENCE_EXTENSION_CODE = 0x000001B5;
const DWORD MPEG_SEQUENCE_END_CODE = 0x000001B7;

const DWORD SEQUENCE_EXTENSION_CODE = 0x00000001;
const DWORD SEQUENCE_DISPLAY_EXTENSION_CODE = 0x00000002;
const DWORD QUANTA_EXTENSION_CODE = 0x00000003;
const DWORD SEQUENCE_SCALABLE_EXTENSION_CODE = 0x00000005;
const DWORD PICTURE_DISPLAY_EXTENSION_CODE = 0x00000007;
const DWORD PICTURE_EXTENSION_CODE = 0x00000008;
const DWORD PICTURE_SPATIAL_EXTENSION_CODE = 0x00000009;
const DWORD PICTURE_TEMPORAL_EXTENSION_CODE = 0x00000010;

const DWORD MPEG_CHROMAT_FORMAT_RESERVED = 0x00000000;
const DWORD MPEG_CHROMAT_FORMAT_420 = 0x00000001;
const DWORD MPEG_CHROMAT_FORMAT_422 = 0x00000010;
const DWORD MPEG_CHROMAT_FORMAT_444 = 0x00000011;

#define PICTURE_TYPE_I    1
#define PICTURE_TYPE_P    2
#define PICTURE_TYPE_B    3

#define HAS_FLAG(b, flag) (((b) & (flag)) == (flag))

inline WORD MAKE_WORD(BYTE b1, BYTE b2){

		return ((b1 << 8) | b2);
}

inline DWORD MAKE_DWORD_HOSTORDER(const BYTE* pData){

		return ((DWORD*)pData)[0];
}

inline DWORD MAKE_DWORD(const BYTE* pData){

		return htonl(MAKE_DWORD_HOSTORDER(pData));
}

#endif