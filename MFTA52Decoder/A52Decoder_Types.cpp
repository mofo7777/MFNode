//----------------------------------------------------------------------------------------------
// A52Decoder_Types.cpp
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

HRESULT CA52Decoder::GetOutputType(IMFMediaType** ppType){

	TRACE_TRANSFORM((L"A52Decoder::GetOutputType"));

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		// Check this value from the source
		const UINT32 uiBitsPerSample = 32;
		const UINT32 uiBlockAlign = m_uiChannels * (uiBitsPerSample / 8);
		const UINT32 uiAvgBytePerSec = uiBlockAlign * m_uiSamplePerSec;

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_PREFER_WAVEFORMATEX, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_uiSamplePerSec));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_uiChannels));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, uiBitsPerSample));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, uiBlockAlign));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, uiAvgBytePerSec));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CA52Decoder::OnCheckInputType(IMFMediaType* pmt){

	TRACE_TRANSFORM((L"A52Decoder::OnCheckInputType"));

	HRESULT hr = S_OK;

	if(m_pInputType){

		DWORD dwFlags = 0;

		if(m_pInputType->IsEqual(pmt, &dwFlags) == S_OK){
			return hr;
		}
		else{
			IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}
	}

	GUID majortype = {0};
	GUID subtype = {0};
	UINT32 uiChannels = 0;
	UINT32 uiSamplePerSec = 0;

	IF_FAILED_RETURN(hr = pmt->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &uiSamplePerSec));
	IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uiChannels));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Audio ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = (subtype != MFAudioFormat_Dolby_AC3 ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = ((uiSamplePerSec == 48000 || uiSamplePerSec == 44100 || uiSamplePerSec == 32000) ? S_OK : MF_E_INVALIDTYPE));
	IF_FAILED_RETURN(hr = ((uiChannels < 1 || uiChannels > 5) ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CA52Decoder::OnSetInputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"A52Decoder::OnSetInputType"));

	HRESULT hr = S_OK;

	SAFE_RELEASE(m_pInputType);

	IF_FAILED_RETURN(hr = pType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &m_uiSamplePerSec));
	IF_FAILED_RETURN(hr = pType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &m_uiChannels));

	m_pInputType = pType;
	m_pInputType->AddRef();

	return hr;
}

HRESULT CA52Decoder::OnCheckOutputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"A52Decoder::OnCheckOutputType"));

	HRESULT hr = S_OK;

	if(m_pOutputType){

		DWORD dwFlags = 0;

		if(m_pOutputType->IsEqual(pType, &dwFlags) == S_OK){
			return hr;
		}
		else{
			IF_FAILED_RETURN(hr = MF_E_INVALIDTYPE);
		}
	}

	if(m_pInputType == NULL){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	IMFMediaType* pOurType = NULL;
	BOOL bMatch = FALSE;

	try{

		IF_FAILED_THROW(hr = GetOutputType(&pOurType));

		IF_FAILED_THROW(hr = pOurType->Compare(pType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch));

		IF_FAILED_THROW(hr = (!bMatch ? MF_E_INVALIDTYPE : S_OK));
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOurType);
	return hr;
}