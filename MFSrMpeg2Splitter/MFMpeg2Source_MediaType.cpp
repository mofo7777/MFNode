//----------------------------------------------------------------------------------------------
// MFMpeg2Source_MediaType.cpp
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

HRESULT CMFMpeg2Source::CreateVideoMediaType(const MPEG2VideoSeqHeader& videoSeqHdr, IMFMediaType** ppType, const BOOL IsMpeg1){
		
		TRACE_MEDIATYPESOURCE((L"Source::CreateVideoMediaType"));

		HRESULT hr = S_OK;
		IMFMediaType* pType = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
				
				GUID gSubType = IsMpeg1 ? MFVideoFormat_MPG1 : MFVideoFormat_MPEG2;

				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, gSubType));
				IF_FAILED_THROW(hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, videoSeqHdr.width, videoSeqHdr.height));
				IF_FAILED_THROW(hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE, videoSeqHdr.frameRate.Numerator, videoSeqHdr.frameRate.Denominator));
				IF_FAILED_THROW(hr = MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, videoSeqHdr.pixelAspectRatio.Numerator, videoSeqHdr.pixelAspectRatio.Denominator));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AVG_BITRATE, videoSeqHdr.bitRate));
				IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Unknown));
				IF_FAILED_THROW(hr = pType->SetBlob(MF_MT_MPEG_SEQUENCE_HEADER, videoSeqHdr.header, videoSeqHdr.cbHeader));

				//LogMediaType(pType);

				*ppType = pType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);

		return hr;
}

HRESULT CMFMpeg2Source::CreateAudioMediaType(const MPEG2AudioFrameHeader& audioHeader, IMFMediaType** ppType){
		
		TRACE_MEDIATYPESOURCE((L"Source::CreateAudioMediaType"));

		HRESULT hr = S_OK;
		IMFMediaType* pType = NULL;

		MPEG1WAVEFORMAT format;
		ZeroMemory(&format, sizeof(format));

		format.wfx.wFormatTag = WAVE_FORMAT_MPEG;
		format.wfx.nChannels = audioHeader.nChannels;
		format.wfx.nSamplesPerSec = audioHeader.dwSamplesPerSec;
		
		/*if(audioHeader.dwBitRate > 0){
				format.wfx.nAvgBytesPerSec = (audioHeader.dwBitRate * 1000) / 8;
		}*/

		format.wfx.nBlockAlign = audioHeader.nChannels * 2;
		format.wfx.wBitsPerSample = 0; // Not used.
		format.wfx.cbSize = sizeof(MPEG1WAVEFORMAT) - sizeof(WAVEFORMATEX);
		format.wfx.nAvgBytesPerSec = format.wfx.nBlockAlign * format.wfx.nSamplesPerSec;

		switch(audioHeader.layer){
		
		case MPEG2_Audio_Layer1:
				format.fwHeadLayer = ACM_MPEG_LAYER1;
				break;

		case MPEG2_Audio_Layer2:
				format.fwHeadLayer = ACM_MPEG_LAYER2;
				break;

		case MPEG2_Audio_Layer3:
				format.fwHeadLayer = ACM_MPEG_LAYER3;
				break;
		};

		format.dwHeadBitrate = audioHeader.dwBitRate * 1000;

		switch(audioHeader.mode){
		
		  case MPEG2_Audio_Stereo:
				  format.fwHeadMode = ACM_MPEG_STEREO;
				  break;

				case MPEG2_Audio_JointStereo:
						format.fwHeadMode = ACM_MPEG_JOINTSTEREO;
						break;

				case MPEG2_Audio_DualChannel:
						format.fwHeadMode = ACM_MPEG_DUALCHANNEL;
						break;

				case MPEG2_Audio_SingleChannel:
						format.fwHeadMode = ACM_MPEG_SINGLECHANNEL;
						break;
		};

		if(audioHeader.mode == MPEG2_Audio_JointStereo){
				
				if(audioHeader.modeExtension <= 0x03){
						format.fwHeadModeExt = 0x01 << audioHeader.modeExtension;
				}
		}

		if(audioHeader.emphasis <= 0x03){
				format.wHeadEmphasis = audioHeader.emphasis + 1;
		}

		format.fwHeadFlags = audioHeader.wFlags;
		format.fwHeadFlags |= ACM_MPEG_ID_MPEG1;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));
				IF_FAILED_THROW(hr = MFInitMediaTypeFromWaveFormatEx(pType, (const WAVEFORMATEX*)&format, sizeof(format)));

				// Needed to work with Microsoft MP3 Decoder MFT
				if(audioHeader.layer == MPEG2_Audio_Layer3)
						IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_MP3));

				//LogMediaType(pType);

				*ppType = pType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);
		
		return hr;
}

HRESULT CMFMpeg2Source::CreateAC3AudioMediaType(const WAVEFORMATEX& audioHeader, IMFMediaType** ppType){
		
		TRACE_MEDIATYPESOURCE((L"Source::CreateAC3AudioMediaType"));

		HRESULT hr = S_OK;
		IMFMediaType* pType = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Dolby_AC3));
		  IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, audioHeader.nChannels));
		  IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, audioHeader.nSamplesPerSec));

				//LogMediaType(pType);

				*ppType = pType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);
		
		return hr;
}