//----------------------------------------------------------------------------------------------
// MFTCudaDecoder_Types.cpp
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

HRESULT CMFTCudaDecoder::GetOutputType(IMFMediaType** ppType, const DWORD dwTypeIndex){

	TRACE_TRANSFORM((L"MFTCuda::GetOutputType"));

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		// MF_MT_DEFAULT_STRIDE

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));

		// MFVideoFormat_YV12 MFVideoFormat_NV12 MFVideoFormat_IYUV MFVideoFormat_I420
		if(dwTypeIndex == 0){

			IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
			m_bNV12 = TRUE;
		}
		else{

			IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_YV12));
			m_bNV12 = FALSE;
		}

		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_DEFAULT_STRIDE, m_uiFrameWidth));

		UINT32 uiSampleSize = (m_uiFrameHeight + (m_uiFrameHeight / 2)) * m_uiFrameWidth;

		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, uiSampleSize));

		IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_uiFrameWidth, m_uiFrameHeight));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_FrameRate.Numerator, m_FrameRate.Denominator));

		// Todo : set correct aspect ratio
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, m_Pixel.Numerator, m_Pixel.Denominator));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CMFTCudaDecoder::OnCheckInputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"MFTCuda::OnCheckInputType"));

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

	GUID majortype = {0};
	GUID subtype = {0};
	UINT32 width = 0;
	UINT32 height = 0;
	MFRatio fps = {0};
	MFRatio pixel = {0};
	UINT32 cbSeqHeader = 0;

	IF_FAILED_RETURN(hr = pType->GetMajorType(&majortype));
	IF_FAILED_RETURN(hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height));
	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator));
	IF_FAILED_RETURN(hr = pType->GetBlobSize(MF_MT_MPEG_SEQUENCE_HEADER, &cbSeqHeader));

	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&pixel.Numerator, (UINT32*)&pixel.Denominator));

	IF_FAILED_RETURN(hr = (majortype != MFMediaType_Video ? MF_E_INVALIDTYPE : S_OK));
	IF_FAILED_RETURN(hr = ((subtype != MFVideoFormat_MPG1 && subtype != MFVideoFormat_MPEG2) ? MF_E_INVALIDTYPE : S_OK));

	m_bMpeg1 = (subtype == MFVideoFormat_MPG1);

	if(m_bMpeg1){
		m_bMpegHeaderEx = TRUE;
	}

	IF_FAILED_RETURN(hr = ((width > MAX_VIDEO_WIDTH_HEIGHT || height > MAX_VIDEO_WIDTH_HEIGHT) ? MF_E_INVALIDTYPE : S_OK));

	IF_FAILED_RETURN(hr = (cbSeqHeader < MIN_BYTE_TO_READ_MPEG_HEADER ? MF_E_INVALIDTYPE : S_OK));

	return hr;
}

HRESULT CMFTCudaDecoder::OnSetInputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"MFTCuda::OnSetInputType"));

	HRESULT hr = S_OK;

	SAFE_RELEASE(m_pInputType);

	IF_FAILED_RETURN(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &m_uiFrameWidth, &m_uiFrameHeight));
	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, (UINT32*)&m_FrameRate.Numerator, (UINT32*)&m_FrameRate.Denominator));

	IF_FAILED_RETURN(hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&m_Pixel.Numerator, (UINT32*)&m_Pixel.Denominator));

	IF_FAILED_RETURN(hr = MFFrameRateToAverageTimePerFrame(m_FrameRate.Numerator, m_FrameRate.Denominator, &m_rtAvgPerFrame));

#ifdef TRACE_TIME_REFERENCE
	TRACE((L"AvgPerFrame = %I64d", m_rtAvgPerFrame));
#endif

	m_dwSampleSize = (m_uiFrameHeight + (m_uiFrameHeight / 2)) * m_uiFrameWidth;

	GUID subtype = {0};
	IF_FAILED_RETURN(hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype));
	IF_FAILED_RETURN(hr = ((subtype != MFVideoFormat_MPG1 && subtype != MFVideoFormat_MPEG2) ? MF_E_INVALIDTYPE : S_OK));

	// ToDo : assign when input media type
	//m_VideoParam.CodecSpecific.mpeg2 = m_Mpeg2VideoParam;

	m_pInputType = pType;
	m_pInputType->AddRef();

	return hr;
}

HRESULT CMFTCudaDecoder::OnCheckOutputType(IMFMediaType* pType){

	TRACE_TRANSFORM((L"MFTCuda::OnCheckOutputType"));

	if(m_pOutputType){

		DWORD dwFlags = 0;

		if(m_pOutputType->IsEqual(pType, &dwFlags) == S_OK){
			return S_OK;
		}
		else{
			return MF_E_INVALIDTYPE;
		}
	}

	if(m_pInputType == NULL){
		return MF_E_TRANSFORM_TYPE_NOT_SET;
	}

	HRESULT hr = S_OK;
	IMFMediaType* pOurType = NULL;

	try{

		BOOL bMatch = FALSE;

		IF_FAILED_THROW(hr = GetOutputType(&pOurType, m_bNV12 ? 0 : 1));

		IF_FAILED_THROW(hr = pOurType->Compare(pType, MF_ATTRIBUTES_MATCH_OUR_ITEMS, &bMatch));

		if(!bMatch)
			throw hr = MF_E_INVALIDTYPE;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOurType);
	return hr;
}