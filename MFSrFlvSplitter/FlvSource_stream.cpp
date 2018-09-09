//----------------------------------------------------------------------------------------------
// FlvSource_stream.cpp
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

HRESULT CFlvSource::CreateAudioStream(){

	TRACE_SOURCE((L"Source::CreateAudioStream"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;
	IMFStreamDescriptor* pSD = NULL;
	CFlvStream* pStream = NULL;
	IMFMediaTypeHandler* pHandler = NULL;
	BYTE bAudioFmt = m_cFlvParser.GetAudioFmt();

	try{

		if(bAudioFmt == FLV_AUDIOCODEC_AAC){
			IF_FAILED_THROW(hr = CreateAudioAACMediaType(&pType));
		}
		else if(bAudioFmt == FLV_AUDIOCODEC_MP3){
			IF_FAILED_THROW(hr = CreateAudioMpegMediaType(&pType));
		}
		else{
			IF_FAILED_THROW(hr = E_UNEXPECTED);
		}

		IF_FAILED_THROW(hr = MFCreateStreamDescriptor(FLV_AUDIO_ID, 1, &pType, &pSD));
		IF_FAILED_THROW(hr = pSD->GetMediaTypeHandler(&pHandler));
		IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));

		IF_FAILED_THROW(hr = CFlvStream::CreateInstance(&pStream, this, pSD, FLV_AUDIO_ID, hr));

		IF_FAILED_THROW(hr = (pStream == NULL ? E_OUTOFMEMORY : S_OK));

		//IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, pStream));

		m_pAudioStream = pStream;
		m_pAudioStream->AddRef();

		IF_FAILED_THROW(hr = InitPresentationDescriptor());
	}
	catch(HRESULT){}

	SAFE_RELEASE(pHandler);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pSD);
	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateVideoStream(){

	TRACE_SOURCE((L"Source::CreateVideoStream"));

	HRESULT hr = S_OK;

	IMFMediaType* pType = NULL;
	IMFStreamDescriptor* pSD = NULL;
	CFlvStream* pStream = NULL;
	IMFMediaTypeHandler* pHandler = NULL;

	try{

		if(m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_AVC){
			IF_FAILED_THROW(hr = CreateVideoH264MediaType(&pType));
		}
		else if(m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_VP6){
			IF_FAILED_THROW(hr = CreateVideoVP6MediaType(&pType));
		}
		else if(m_cFlvParser.GetVideoFmt() == FLV_VIDEOCODEC_H263){
			//IF_FAILED_THROW(hr = CreateVideoH263MediaType(&pType));
			IF_FAILED_THROW(hr = CreateVideoVP6MediaType(&pType));
		}
		else{
			IF_FAILED_THROW(hr = E_FAIL);
		}

		IF_FAILED_THROW(hr = MFCreateStreamDescriptor(FLV_VIDEO_ID, 1, &pType, &pSD));
		IF_FAILED_THROW(hr = pSD->GetMediaTypeHandler(&pHandler));
		IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));

		IF_FAILED_THROW(hr = CFlvStream::CreateInstance(&pStream, this, pSD, FLV_VIDEO_ID, hr));

		IF_FAILED_THROW(hr = (pStream == NULL ? E_OUTOFMEMORY : S_OK));

		//IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MENewStream, GUID_NULL, hr, pStream));

		m_pVideoStream = pStream;
		m_pVideoStream->AddRef();

		IF_FAILED_THROW(hr = InitPresentationDescriptor());
	}
	catch(HRESULT){}

	SAFE_RELEASE(pHandler);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pSD);
	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateAudioAACMediaType(IMFMediaType** ppType){

	TRACE_SOURCE((L"Source::CreateAudioAACMediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		// MEDIASUBTYPE_RAW_AAC1 MFAudioFormat_AAC
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MEDIASUBTYPE_RAW_AAC1));
		//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, 1));
		//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, m_cFlvParser.GetAudioTag()->));
		//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0));
		//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, 0));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_cFlvParser.GetAudioTag()->uiSampleRate));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_cFlvParser.GetAudioTag()->wChannel));
		//IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, 1));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, m_cFlvParser.GetAudioTag()->wBitSample));

		IF_FAILED_THROW(hr = pType->SetBlob(MF_MT_USER_DATA, m_cFlvParser.GetAudioTag()->btAACHeader, 2));

		// MF_MT_AUDIO_AVG_BYTES_PER_SECOND
		// MF_MT_AVG_BITRATE
		//LogMediaType(pType);

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateAudioMpegMediaType(IMFMediaType** ppType){

	TRACE_SOURCE((L"Source::CreateAudioMpegMediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_MP3));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, TRUE));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_cFlvParser.GetAudioTag()->uiSampleRate));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_cFlvParser.GetAudioTag()->wChannel));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_cFlvParser.GetAudioTag()->uiBitrate));

		//LogMediaType(pType);

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateVideoH264MediaType(IMFMediaType** ppType){

	TRACE_SOURCE((L"Source::CreateVideoH264MediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		FLV_VIDEO_HEADER* pFlvVideoHeader = m_cFlvParser.GetVideoTag();

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
		//IF_FAILED_THROW(hr = MFSetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_MixedInterlaceOrProgressive));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_MPEG2_LEVEL, pFlvVideoHeader->btAvcLevelIndication));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_MPEG2_PROFILE, pFlvVideoHeader->btAvcProfileIndication));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_SAMPLE_SIZE, 1));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY, 0));

		UINT32 uiBlobSize = pFlvVideoHeader->wSequenceParameterSetLength + pFlvVideoHeader->wPictureParameterSetLength + 6;

		if(uiBlobSize > MAX_VIDEO_RECORED_SIZE){
			TRACE((L"Source::CreateVideoAVC1MediaType uiBlobSize = %d", uiBlobSize));
		}

		assert(uiBlobSize <= MAX_VIDEO_RECORED_SIZE);

		IF_FAILED_THROW(pType->SetBlob(MF_MT_MPEG_SEQUENCE_HEADER, m_cFlvParser.GetVideoRecord(), uiBlobSize));

		//LogMediaType(pType);

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateVideoVP6MediaType(IMFMediaType** ppType){

	TRACE_SOURCE((L"Source::CreateVideoVP6MediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		FLV_VIDEO_HEADER* pFlvVideoHeader = m_cFlvParser.GetVideoTag();

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_VP62));
		IF_FAILED_THROW(hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, pFlvVideoHeader->uiWidth, pFlvVideoHeader->uiHeight));

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::CreateVideoH263MediaType(IMFMediaType** ppType){

	TRACE_SOURCE((L"Source::CreateVideoH263MediaType"));

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		FLV_VIDEO_HEADER* pFlvVideoHeader = m_cFlvParser.GetVideoTag();

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		// MFVideoFormat_MP43 MFVideoFormat_MP4S MFVideoFormat_MP4V MFVideoFormat_M4S2
		// Todo : does not work
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_MP43));
		IF_FAILED_THROW(hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE, pFlvVideoHeader->uiWidth, pFlvVideoHeader->uiHeight));

		*ppType = pType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);

	return hr;
}

HRESULT CFlvSource::InitPresentationDescriptor(){

	TRACE_SOURCE((L"Source::InitPresentationDescriptor"));

	HRESULT hr = S_OK;

	assert(m_pPresentationDescriptor == NULL);
	assert(m_State == SourceOpening);

	if(m_pAudioStream == NULL || m_pVideoStream == NULL)
		return hr;

	IMFStreamDescriptor** ppSD = new (std::nothrow)IMFStreamDescriptor*[2];

	IF_FAILED_RETURN(ppSD == NULL ? E_OUTOFMEMORY : S_OK);

	ZeroMemory(ppSD, 2 * sizeof(IMFStreamDescriptor*));

	try{

		IF_FAILED_THROW(hr = m_pAudioStream->GetStreamDescriptor(&ppSD[0]));
		IF_FAILED_THROW(hr = m_pVideoStream->GetStreamDescriptor(&ppSD[1]));

		IF_FAILED_THROW(hr = MFCreatePresentationDescriptor(2, ppSD, &m_pPresentationDescriptor));
		IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(0));
		IF_FAILED_THROW(hr = m_pPresentationDescriptor->SelectStream(1));

		m_State = SourceStopped;

		IF_FAILED_THROW(hr = CompleteOpen(S_OK));
	}
	catch(HRESULT){}

	if(ppSD){

		SAFE_RELEASE(ppSD[0]);
		SAFE_RELEASE(ppSD[1]);
		delete[] ppSD;
	}

	return hr;
}