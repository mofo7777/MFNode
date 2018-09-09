//----------------------------------------------------------------------------------------------
// MFTWaveMixer.cpp
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
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CMFTWaveMixer::CMFTWaveMixer() :

  m_pAudioType(NULL),
		m_pInput1AudioType(NULL),
		m_pSample0(NULL),
		m_pSample1(NULL),
  m_pInput0Buffer(NULL),
  m_pInput1Buffer(NULL),
		m_pbInput0Data(NULL),
		m_dwInput0Length(0),
		m_pbInput1Data(NULL),
		m_dwInput1Length(0),
		m_rtTimestamp(0),
		m_bValidTime(FALSE),
		m_bInput0TypeSet(FALSE),
		m_bInput1TypeSet(FALSE),
		m_bOutputTypeSet(FALSE){
}

CMFTWaveMixer::~CMFTWaveMixer(){

		AutoLock lock(m_CriticSection);

		ReleaseInputBuffer();
		SAFE_RELEASE(m_pAudioType);
		SAFE_RELEASE(m_pInput1AudioType);
}

HRESULT CMFTWaveMixer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		if(ppv == NULL){
				return E_POINTER;
		}

		if(pUnkOuter != NULL){
				return CLASS_E_NOAGGREGATION;
		}

		HRESULT hr = S_OK;

		CMFTWaveMixer* pMFT = new (std::nothrow)CMFTWaveMixer;

		if(pMFT == NULL){
				return E_OUTOFMEMORY;
		}

		hr = pMFT->QueryInterface(iid, ppv);

		SAFE_RELEASE(pMFT);

		return hr;
}

HRESULT CMFTWaveMixer::QueryInterface(REFIID riid, void** ppv){
		
		if(NULL == ppv){
				return E_POINTER;
		}
		else if(riid == __uuidof(IUnknown)){
				*ppv = static_cast<IUnknown*>(this);
		}
		else if(riid == __uuidof(IMFTransform)){
				*ppv = static_cast<IMFTransform*>(this);
		}
		else{
				*ppv = NULL;
				return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
}

HRESULT CMFTWaveMixer::OnBeginStream(){

		// The client must set media type before allocating streaming resources.
		if(!m_bInput0TypeSet || !m_bInput1TypeSet || !m_bOutputTypeSet || m_pAudioType == NULL){

				return MF_E_TRANSFORM_TYPE_NOT_SET;
		}
		else{

				// We don't need anymore when stream begins
				//SAFE_RELEASE(m_pInput1AudioType);

				return S_OK;
		}
}

HRESULT CMFTWaveMixer::GetMinimalType(IMFMediaType** ppType){

		IMFMediaType* pType = NULL;
		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = MFCreateMediaType(&pType));

				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
				IF_FAILED_THROW(hr = pType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));

				*ppType = pType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){ LOG_HRESULT(hr); }

  SAFE_RELEASE(pType);
		
		return hr;
}

HRESULT CMFTWaveMixer::OnCheckAudioType(IMFMediaType* pmt, const DWORD dwStreamID){

		HRESULT hr = S_OK;
		DWORD flags = 0;

		if(dwStreamID == 0 && m_pAudioType){

				hr = pmt->IsEqual(m_pAudioType, &flags);

				// IsEqual can return S_FALSE. Treat this as failure.
				if(hr != S_OK){
						hr = MF_E_INVALIDMEDIATYPE;
				}
				else if(m_pInput1AudioType){
						
						hr = pmt->IsEqual(m_pInput1AudioType, &flags);

						if(hr != S_OK){
								hr = MF_E_INVALIDMEDIATYPE;
						}
				}
		}
		else if(dwStreamID == 1){
				
				if(m_pInput1AudioType){

						hr = pmt->IsEqual(m_pInput1AudioType, &flags);

						if(hr != S_OK){
								hr = MF_E_INVALIDMEDIATYPE;
						}
				}
		}
		else{
				
				// Audio type is not set. Just check this type.
				hr = ValidatePCMAudioType(pmt);
		}
		
		return hr;
}

HRESULT CMFTWaveMixer::OnSetMediaType(IMFMediaType* pType, StreamDirection dir, const DWORD dwStreamID){
		
		HRESULT hr = S_OK;
		BOOL bInputType = (dir == InputStream);

		if(pType){

				if(dwStreamID == 0){
						
						// Store the media type.
						SAFE_RELEASE(m_pAudioType);
						m_pAudioType = pType;
						m_pAudioType->AddRef();
				}
				else if(m_pInput1AudioType == NULL){
						
						// Store the media type, only once.
						SAFE_RELEASE(m_pInput1AudioType);
						m_pInput1AudioType = pType;
						m_pInput1AudioType->AddRef();
				}

				// Flag the stream (input or output) as set.
				if(bInputType){

						if(dwStreamID == 0)
								m_bInput0TypeSet = TRUE;
						else
								m_bInput1TypeSet = TRUE;
				}
				else{
						m_bOutputTypeSet = TRUE;
				}
		}
		else{

				// Clear the media type.
				if(bInputType){

						if(dwStreamID == 0)
								m_bInput0TypeSet = FALSE;
						else
								m_bInput1TypeSet = FALSE;
				}
				else{
						m_bOutputTypeSet = FALSE;
				}

				if(!m_bOutputTypeSet && !m_bInput0TypeSet && !m_bInput1TypeSet){
						SAFE_RELEASE(m_pAudioType);
				}
		}

		return hr;
}

HRESULT CMFTWaveMixer::ValidatePCMAudioType(IMFMediaType* pmt){
		
		HRESULT hr = S_OK;
		GUID majorType = GUID_NULL;
		GUID subtype = GUID_NULL;

		UINT32 nChannels = 0;
		UINT32 nSamplesPerSec = 0;
		UINT32 nAvgBytesPerSec = 0;
		UINT32 nBlockAlign = 0;
		UINT32 wBitsPerSample = 0;

		IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &majorType));
		IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
		IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &nChannels));
		IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &nSamplesPerSec));
		IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &nAvgBytesPerSec));
		IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &nBlockAlign));
		IF_FAILED_RETURN(hr = pmt->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &wBitsPerSample));

		if(nChannels != 1 && nChannels != 2){
				IF_FAILED_RETURN(hr = MF_E_INVALIDMEDIATYPE);
		}

		if(wBitsPerSample != 8 && wBitsPerSample != 16){
				IF_FAILED_RETURN(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Make sure block alignment was calculated correctly.
		if(nBlockAlign != nChannels * (wBitsPerSample / 8)){   
				IF_FAILED_RETURN(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Check possible overflow... Is (nSamplesPerSec * nBlockAlign > MAXDWORD) ?
		if(nSamplesPerSec  > (DWORD)(MAXDWORD / nBlockAlign)){
				IF_FAILED_RETURN(hr = MF_E_INVALIDMEDIATYPE);
		}

		// Make sure average bytes per second was calculated correctly.
		if(nAvgBytesPerSec != nSamplesPerSec * nBlockAlign){
				IF_FAILED_RETURN(hr = MF_E_INVALIDMEDIATYPE);
		}

		return hr;
}