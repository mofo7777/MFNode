//----------------------------------------------------------------------------------------------
// MFMpeg12Definition.h
//----------------------------------------------------------------------------------------------
#ifndef MFMPEG12DEFINITION_H
#define MFMPEG12DEFINITION_H

// Bug : IMFByteStream with 4096 sometimes in Read
// there is no bug with a constant buffer...
#define MPEG12_READ_SIZE 4096 + 16

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

#endif