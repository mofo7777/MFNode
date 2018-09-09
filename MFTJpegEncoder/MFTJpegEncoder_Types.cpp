//----------------------------------------------------------------------------------------------
// MFTJpegEncoder_Types.cpp
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

HRESULT CMFTJpegEncoder::GetOutputType(IMFMediaType** ppType){

		HRESULT hr = S_OK;
		IMFMediaType* pOutputType = NULL;
		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;
		GUID SubType = {0};
		int iRGBFormat = 3;
		MFRatio Fps = {0};
		MFRatio Pixel = {0};

		try{

				// m_pInputType must not be NULL (see GetOutputAvailableType).
				IF_FAILED_THROW(hr = m_pInputType->GetGUID(MF_MT_SUBTYPE, &SubType));
				IF_FAILED_THROW(hr = MFGetAttributeSize(m_pInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
				IF_FAILED_THROW(hr = MFGetAttributeRatio(m_pInputType, MF_MT_FRAME_RATE, (UINT32*)&Fps.Numerator, (UINT32*)&Fps.Denominator));
				IF_FAILED_THROW(hr = MFGetAttributeRatio(m_pInputType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&Pixel.Numerator, (UINT32*)&Pixel.Denominator));

				IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));
				
				IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
				IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_MJPG));
				IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
				IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
				IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));

				if(SubType == MFVideoFormat_RGB32)
						iRGBFormat = 4;

				UINT32 uiSampleSize = uiWidth + uiHeight * iRGBFormat;

				IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, uiSampleSize));

				// The MFSrScreenCapture do not stride, but over MFSource could...
				//IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, ???));

    IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, uiWidth, uiHeight));
    IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, Fps.Numerator, Fps.Denominator));

				IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, Pixel.Numerator, Pixel.Denominator));

				*ppType = pOutputType;
				(*ppType)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pOutputType);
		return hr;
}

HRESULT CMFTJpegEncoder::OnCheckInputType(IMFMediaType* pType){

		// Check if the type is already set and if so reject any type that's not identical.
		if(m_pInputType){

				DWORD dwFlags = 0;

				if(m_pInputType->IsEqual(pType, &dwFlags) == S_OK){
						return S_OK;
				}
				else{
						return MF_E_INVALIDTYPE;
				}
		}

		HRESULT hr = S_OK;

		GUID MajorType = {0};
		GUID SubType = {0};
		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;
		MFRatio Fps = {0};
		MFRatio Pixel = {0};

		IF_FAILED_RETURN(hr = pType->GetMajorType(&MajorType));
		IF_FAILED_RETURN(hr = pType->GetGUID(MF_MT_SUBTYPE, &SubType));
		IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
		IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&Fps.Numerator, (UINT32*)&Fps.Denominator));
		IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&Pixel.Numerator, (UINT32*)&Pixel.Denominator));
		IF_FAILED_RETURN(hr = MFFrameRateToAverageTimePerFrame(Fps.Numerator, Fps.Denominator, &m_ui64AvgPerFrame));

		IF_FAILED_RETURN(hr = (MajorType != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
		IF_FAILED_RETURN(hr = ((SubType != MFVideoFormat_RGB24 && SubType != MFVideoFormat_RGB32) ? MF_E_INVALIDTYPE : S_OK));

		return hr;
}

HRESULT CMFTJpegEncoder::OnCheckOutputType(IMFMediaType* pType){

		// Check if the type is already set and if so reject any type that's not identical.
		if(m_pOutputType){

				DWORD dwFlags = 0;

				if(m_pOutputType->IsEqual(pType, &dwFlags) == S_OK){
						return S_OK;
				}
				else{
						return MF_E_INVALIDTYPE;
				}
		}

		// Input type must be set first.
		if(m_pInputType == NULL){
				return MF_E_TRANSFORM_TYPE_NOT_SET;
		}

		HRESULT hr = S_OK;
		IMFMediaType* pOurType = NULL;

		try{

				BOOL bMatch = FALSE;

				// Make sure their type is a superset of our proposed output type.
				IF_FAILED_THROW(hr = GetOutputType(&pOurType));

				IF_FAILED_THROW(hr = pOurType->Compare(pType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch));

				if(!bMatch)
						throw hr = MF_E_INVALIDTYPE;
		}
		catch(HRESULT){}

		SAFE_RELEASE(pOurType);
		return hr;
}