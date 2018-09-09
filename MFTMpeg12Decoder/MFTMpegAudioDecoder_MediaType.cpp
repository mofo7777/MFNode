//-----------------------------------------------------------------------------------------------
// MFTMpegAudioDecoder_MediaType.cpp
// Copyright (C) 2013 Dumonteil David
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
//-----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CMFTMpegAudioDecoder::GetInputType(IMFMediaType** ppmt){

	HRESULT hr = S_OK;
	IMFMediaType* pType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_MPEG));

		*ppmt = pType;
		(*ppmt)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pType);
	return hr;
}

HRESULT CMFTMpegAudioDecoder::GetOutputType(IMFMediaType** ppmt){

	IMFMediaType* pType = NULL;
	HRESULT hr = S_OK;
	MPEG1WAVEFORMAT* Mpeg1Format = NULL;
	UINT32 uiMpeg1FormatSize = 0;

	try{

		IF_FAILED_THROW(hr = MFCreateWaveFormatExFromMFMediaType(m_pInputType, (WAVEFORMATEX**)&Mpeg1Format, &uiMpeg1FormatSize));

		IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

		// Check this value : Mpeg1Format provide them after MFCreateWaveFormatExFromMFMediaType ?
		const UINT32 uiBitsPerSample = 16;
		const UINT32 uiBlockAlign = Mpeg1Format->wfx.nChannels * (uiBitsPerSample / 8);
		const UINT32 uiAvgBytePerSec = uiBlockAlign * Mpeg1Format->wfx.nSamplesPerSec;

		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, TRUE));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, Mpeg1Format->wfx.nSamplesPerSec));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, Mpeg1Format->wfx.nChannels));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, uiBitsPerSample));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, uiBlockAlign));
		IF_FAILED_THROW(hr = pType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, uiAvgBytePerSec));

		*ppmt = pType;
		(*ppmt)->AddRef();
	}
	catch(HRESULT){}

	CoTaskMemFree(Mpeg1Format);
	SAFE_RELEASE(pType);
	return hr;
}

HRESULT CMFTMpegAudioDecoder::OnCheckInputType(IMFMediaType* pmt){

	assert(pmt != NULL);

	HRESULT hr = S_OK;

	// If the output type is set, see if they match.
	if(m_pInputType){

		DWORD flags = 0;
		hr = pmt->IsEqual(m_pInputType, &flags);

		// IsEqual can return S_FALSE. Treat this as failure.
		if(hr != S_OK){
			hr = MF_E_INVALIDMEDIATYPE;
		}
	}
	else{
		// Output type is not set. Just check this type.
		hr = ValidateMpegAudioType(pmt);
	}
	return hr;
}

HRESULT CMFTMpegAudioDecoder::OnCheckOutputType(IMFMediaType* pmt){

	assert(pmt != NULL);

	HRESULT hr = S_OK;

	// If the input type is set, see if they match.
	if(m_pOutputType){

		DWORD flags = 0;
		hr = pmt->IsEqual(m_pOutputType, &flags);

		// IsEqual can return S_FALSE. Treat this as failure.
		if(hr != S_OK){
			hr = MF_E_INVALIDMEDIATYPE;
		}
	}

	if(m_pInputType == NULL){
		// Input type must be set first.
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	// OutputType type is not set. Just check this type.
	hr = ValidatePcmAudioType(pmt);

	return hr;
}

HRESULT CMFTMpegAudioDecoder::ValidateMpegAudioType(IMFMediaType* pmt){

	HRESULT hr = S_OK;
	GUID majorType = GUID_NULL;
	GUID subtype = GUID_NULL;
	MPEG1WAVEFORMAT* Mpeg1Format = NULL;
	UINT32 uiMpeg1FormatSize = 0;

	try{

		//Create the WAVEFORMATEX structure from the media type
		IF_FAILED_THROW(hr = MFCreateWaveFormatExFromMFMediaType(pmt, (WAVEFORMATEX**)&Mpeg1Format, &uiMpeg1FormatSize));

		IF_FAILED_THROW(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
		IF_FAILED_THROW(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));

		// Validate the values.
		if(majorType != MFMediaType_Audio){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(subtype != MFAudioFormat_MPEG){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(Mpeg1Format->fwHeadLayer != 1 && Mpeg1Format->fwHeadLayer != 2){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(Mpeg1Format->wfx.nChannels != 1 && Mpeg1Format->wfx.nChannels != 2){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(Mpeg1Format->wfx.wFormatTag != WAVE_FORMAT_MPEG){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Todo Check others values......
	}
	catch(HRESULT){}

	CoTaskMemFree(Mpeg1Format);

	return hr;
}

HRESULT CMFTMpegAudioDecoder::ValidatePcmAudioType(IMFMediaType* pmt){

	HRESULT hr = S_OK;
	GUID majorType = GUID_NULL;
	GUID subtype = GUID_NULL;

	UINT32 nChannels = 0;
	UINT32 nSamplesPerSec = 0;
	//UINT32 nAvgBytesPerSec = 0;
	m_uiAvgRate = 0;
	UINT32 nBlockAlign = 0;
	UINT32 wBitsPerSample = 0;

	// Get attributes from the media type. Each of these attributes is required for uncompressed PCM audio, so fail if any are not present.
	try{

		IF_FAILED_THROW(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
		IF_FAILED_THROW(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
		IF_FAILED_THROW(hr = pmt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &nChannels));
		IF_FAILED_THROW(hr = pmt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesPerSec));
		IF_FAILED_THROW(hr = pmt->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &m_uiAvgRate));
		IF_FAILED_THROW(hr = pmt->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &nBlockAlign));
		IF_FAILED_THROW(hr = pmt->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample));

		// Validate the values.
		if(nChannels != 1 && nChannels != 2){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(wBitsPerSample != 16){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Make sure block alignment was calculated correctly.
		if(nBlockAlign != nChannels * (wBitsPerSample / 8)){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Check possible overflow... Is (nSamplesPerSec * nBlockAlign > MAXDWORD) ?
		if(nSamplesPerSec > (DWORD)(MAXDWORD / nBlockAlign)){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Make sure average bytes per second was calculated correctly.
		if(m_uiAvgRate != nSamplesPerSec * nBlockAlign){
			IF_FAILED_THROW(hr = MF_E_INVALIDMEDIATYPE);
		}
	}
	catch(HRESULT){}

	return hr;
}