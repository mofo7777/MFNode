//----------------------------------------------------------------------------------------------
// Vp6Decoder_Types.cpp
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

MFOffset MakeOffset(float v){

	MFOffset offset;
	offset.value = short(v);
	offset.fract = WORD(65536 * (v - offset.value));

	return offset;
}

MFVideoArea MakeArea(float x, float y, DWORD width, DWORD height){

	MFVideoArea area;
	area.OffsetX = MakeOffset(x);
	area.OffsetY = MakeOffset(y);
	area.Area.cx = width;
	area.Area.cy = height;

	return area;
}

HRESULT CVp6Decoder::GetOutputType(IMFMediaType** ppType){

	TRACE_TRANSFORM((L"VP6Decoder::GetOutputType"));

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		// MFVideoFormat_I420 MFVideoFormat_YV12 MFVideoFormat_RGB24 MFVideoFormat_RGB32
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_I420));
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_dwSampleSize));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, m_uiWidthInPixels));
		IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_uiWidthInPixels, m_uiHeightInPixels));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1));

		MFVideoArea area = MakeArea(0, 0, m_uiWidthInPixels, m_uiHeightInPixels);

		IF_FAILED_THROW(hr = pOutputType->SetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*)&area, sizeof(MFVideoArea)));

		//IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_FrameRate.Numerator, m_FrameRate.Denominator));

					// Todo check ratio
		//IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 4, 3));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CVp6Decoder::OnCheckInputType(IMFMediaType* pmt){

	TRACE_TRANSFORM((L"VP6Decoder::OnCheckInputType"));

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
	UINT32 width = 0;
	UINT32 height = 0;
	//MFRatio fps = {0};

	IF_FAILED_RETURN(hr = pmt->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = MFGetAttributeSize(pmt, MF_MT_FRAME_SIZE, &width, &height));
	//IF_FAILED_RETURN(hr = MFGetAttributeRatio(pmt, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = (subtype != MFVideoFormat_VP62 ? MF_E_INVALIDTYPE : S_OK));

	//IF_FAILED_RETURN(hr = ((width > MAX_VIDEO_WIDTH_HEIGHT || height > MAX_VIDEO_WIDTH_HEIGHT) ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CVp6Decoder::OnSetInputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"VP6Decoder::OnSetInputType"));

	HRESULT hr = S_OK;

	SAFE_RELEASE(m_pInputType);

	IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uiWidthInPixels, &m_uiHeightInPixels));

	//IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&m_FrameRate.Numerator, (UINT32*)&m_FrameRate.Denominator));
		  //IF_FAILED_RETURN(hr = MFFrameRateToAverageTimePerFrame(m_FrameRate.Numerator, m_FrameRate.Denominator, &m_rtAvgPerFrameInput));

	m_dwSampleSize = (m_uiHeightInPixels + (m_uiHeightInPixels / 2)) * m_uiWidthInPixels;
	//m_dwSampleSize = m_uiWidthInPixels * m_uiHeightInPixels * 3;

	m_pInputType = pType;
	m_pInputType->AddRef();

	return hr;
}

HRESULT CVp6Decoder::OnCheckOutputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"VP6Decoder::OnCheckOutputType"));

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