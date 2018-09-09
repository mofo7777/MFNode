//----------------------------------------------------------------------------------------------
// Dxva2Decoder_Dxva2.cpp
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

HRESULT CDxva2Decoder::GetVideoDecoder(IDirect3DDeviceManager9* pDeviceManager){

	TRACE_TRANSFORM((L"MFTDxva2::GetVideoDecoder"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pDeviceManager == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (m_pDeviceManager != NULL || m_pDecoderService != NULL || m_pVideoDecoder != NULL ? E_UNEXPECTED : S_OK));

	GUID* pGuids = NULL;
	D3DFORMAT* pFormats = NULL;
	UINT uiCount;
	BOOL bDecoderMpeg2 = FALSE;

	try{

		IF_FAILED_THROW(hr = pDeviceManager->OpenDeviceHandle(&m_hD3d9Device));
		IF_FAILED_THROW(hr = pDeviceManager->GetVideoService(m_hD3d9Device, IID_IDirectXVideoDecoderService, reinterpret_cast<void**>(&m_pDecoderService)));

		IF_FAILED_THROW(hr = m_pDecoderService->GetDecoderDeviceGuids(&uiCount, &pGuids));

		for(UINT ui = 0; ui < uiCount; ui++){

#ifdef TRACE_DXVA2_DECODER_CAPS
			TRACE((L"%s", MFDxva2String(pGuids[ui])));
#endif

			GUID GuidTest = pGuids[ui];

			if(pGuids[ui] == m_gMpegVld){
				bDecoderMpeg2 = TRUE;
#ifndef TRACE_DXVA2_DECODER_CAPS
				break;
#endif
			}
		}

		IF_FAILED_THROW(hr = (bDecoderMpeg2 ? S_OK : E_FAIL));
		bDecoderMpeg2 = FALSE;

		IF_FAILED_THROW(hr = m_pDecoderService->GetDecoderRenderTargets(m_gMpegVld, &uiCount, &pFormats));

		for(UINT ui = 0; ui < uiCount; ui++){

#ifdef TRACE_DXVA2_DECODER_CAPS
			TRACE((L"%s", MFFormatString(pFormats[ui])));
#endif

			if(pFormats[ui] == D3DFMT_NV12){
				bDecoderMpeg2 = TRUE;
#ifndef TRACE_DXVA2_DECODER_CAPS
				break;
#endif
			}
		}

		IF_FAILED_THROW(hr = (bDecoderMpeg2 ? S_OK : E_FAIL));

		m_pDeviceManager = pDeviceManager;
		m_pDeviceManager->AddRef();
	}
	catch(HRESULT){}

	if(FAILED(hr)){

		if(m_hD3d9Device){

			LOG_HRESULT(pDeviceManager->CloseDeviceHandle(m_hD3d9Device));
			m_hD3d9Device = NULL;
		}

		SAFE_RELEASE(m_pDecoderService);
		SAFE_RELEASE(m_pDeviceManager);
	}

	CoTaskMemFree(pFormats);
	CoTaskMemFree(pGuids);

	return hr;
}

HRESULT CDxva2Decoder::ConfigureDecoder(){

	TRACE_TRANSFORM((L"MFTDxva2::ConfigureDecoder"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (m_pDecoderService == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = Flush());

	if(m_pConfigs){
		CoTaskMemFree(m_pConfigs);
		m_pConfigs = NULL;
	}

	UINT uiCount;

	memset(&m_Dxva2Desc, 0, sizeof(DXVA2_VideoDesc));

	DXVA2_Frequency Dxva2Freq;
	Dxva2Freq.Numerator = m_FrameRate.Numerator;
	Dxva2Freq.Denominator = m_FrameRate.Denominator;

	m_Dxva2Desc.SampleWidth = m_uiWidthInPixels;
	m_Dxva2Desc.SampleHeight = m_uiHeightInPixels;

	m_Dxva2Desc.SampleFormat.SampleFormat = (m_bProgressive ? MFVideoInterlace_Progressive : MFVideoInterlace_MixedInterlaceOrProgressive);

	m_Dxva2Desc.Format = static_cast<D3DFORMAT>(D3DFMT_NV12);
	m_Dxva2Desc.InputSampleFreq = Dxva2Freq;
	m_Dxva2Desc.OutputFrameFreq = Dxva2Freq;

	IF_FAILED_RETURN(hr = m_pDecoderService->GetDecoderConfigurations(m_gMpegVld, &m_Dxva2Desc, NULL, &uiCount, &m_pConfigs));

	// We will take the first configuration.
	IF_FAILED_RETURN(hr = (uiCount == 0 ? E_FAIL : S_OK));

	return hr;
}

HRESULT CDxva2Decoder::SchedulePicture(){

	TRACE_TRANSFORM((L"MFTDxva2::SchedulePicture"));

	HRESULT hr = S_OK;

	if(m_bFirstPictureI == FALSE){
		return hr;
	}

	assert(m_cTSScheduler.IsFull() == FALSE);

	m_VideoParam.iCurPictureId = m_iCurSurfaceIndex++;

	if(m_iCurSurfaceIndex == NUM_DXVA2_SURFACE)
		m_iCurSurfaceIndex = 0;

	m_VideoParam.uiBitstreamDataLen = m_SliceBuffer.GetBufferSize();
	m_VideoParam.pBitstreamData = m_SliceBuffer.GetStartBuffer();
	m_VideoParam.pSliceDataOffsets = m_pSliceOffset;

	m_VideoParam.iCurPictureId = m_cTSScheduler.GetFreeIDX();

	assert(m_cTSScheduler.AddFrame(m_VideoParam.iCurPictureId, m_VideoParam.picture_coding_type, m_dwTemporalReference));

	if(m_cTSScheduler.IsFull()){

		m_bHaveOuput = TRUE;
	}

#ifdef TRACE_TIME_REFERENCE
	if(m_MpegPictureParams.picture_coding_type == 1)
		TRACE((L"decodePicture : I"));
	else if(m_MpegPictureParams.picture_coding_type == 2)
		TRACE((L"decodePicture : P"));
	else if(m_MpegPictureParams.picture_coding_type == 3)
		TRACE((L"decodePicture : B"));
	else
		TRACE((L"decodePicture : ERROR"));
#endif

	IF_FAILED_RETURN(hr = Dxva2DecodePicture());

	if(m_VideoParam.picture_coding_type != PICTURE_TYPE_B)
		m_iLastPictureP = m_VideoParam.iCurPictureId;

	return hr;
}

HRESULT CDxva2Decoder::Dxva2DecodePicture(){

	TRACE_TRANSFORM((L"MFTDxva2::Dxva2DecodePicture"));

	HRESULT hr = S_OK;
	IDirect3DDevice9* pDevice = NULL;
	void* pBuffer = NULL;
	UINT uiSize = 0;

	try{

		IF_FAILED_THROW(hr = m_pDeviceManager->TestDevice(m_hD3d9Device));
		IF_FAILED_THROW(hr = m_pDeviceManager->LockDevice(m_hD3d9Device, &pDevice, TRUE));

		// We don't handle this case... Don't know why, but this isn't a problem...
		// It seems that EVR leaves the sample useable before we decode onto the surface, and before Invoke is called.
		// We need to schelude samples differently, to be sure there is a free surface (TimeStamp scheduler was first adapted to Cuda decoder).
		// assert(m_bFreeSurface[m_VideoParam.iCurPictureId]);

		// Can stay in infinite loop...
		do{

			hr = m_pVideoDecoder->BeginFrame(m_pSurface9[m_VideoParam.iCurPictureId], NULL);
		} while(hr != S_OK && hr == E_PENDING);

		IF_FAILED_THROW(hr);

		// Picture
		InitPictureParams();
		IF_FAILED_THROW(hr = m_pVideoDecoder->GetBuffer(DXVA2_PictureParametersBufferType, &pBuffer, &uiSize));
		assert(sizeof(DXVA_PictureParameters) <= uiSize);
		memcpy(pBuffer, &m_PictureParams, sizeof(DXVA_PictureParameters));
		IF_FAILED_THROW(hr = m_pVideoDecoder->ReleaseBuffer(DXVA2_PictureParametersBufferType));

		// QuantaMatrix
		InitQuantaMatrixParams();
		IF_FAILED_THROW(hr = m_pVideoDecoder->GetBuffer(DXVA2_InverseQuantizationMatrixBufferType, &pBuffer, &uiSize));
		assert(sizeof(DXVA_QmatrixData) <= uiSize);
		memcpy(pBuffer, &m_QuantaMatrix, sizeof(DXVA_QmatrixData));
		IF_FAILED_THROW(hr = m_pVideoDecoder->ReleaseBuffer(DXVA2_InverseQuantizationMatrixBufferType));

		// BitStream
		IF_FAILED_THROW(hr = m_pVideoDecoder->GetBuffer(DXVA2_BitStreamDateBufferType, &pBuffer, &uiSize));
		assert(m_VideoParam.uiBitstreamDataLen <= uiSize);
		memcpy(pBuffer, m_VideoParam.pBitstreamData, m_VideoParam.uiBitstreamDataLen);
		IF_FAILED_THROW(hr = m_pVideoDecoder->ReleaseBuffer(DXVA2_BitStreamDateBufferType));

		// Slices
		InitSliceParams();
		IF_FAILED_THROW(hr = m_pVideoDecoder->GetBuffer(DXVA2_SliceControlBufferType, &pBuffer, &uiSize));
		assert(m_VideoParam.uiNumSlices * sizeof(DXVA_SliceInfo) <= uiSize);
		memcpy(pBuffer, &m_SliceInfo, m_VideoParam.uiNumSlices * sizeof(DXVA_SliceInfo));
		IF_FAILED_THROW(hr = m_pVideoDecoder->ReleaseBuffer(DXVA2_SliceControlBufferType));

		// CompBuffers
		m_BufferDesc[2].DataSize = m_VideoParam.uiBitstreamDataLen;
		m_BufferDesc[3].DataSize = m_VideoParam.uiNumSlices * sizeof(DXVA_SliceInfo);

#ifdef TRACE_DXVA2
		TraceConfigPictureDecode();
		TracePictureParams();
		TraceQuantaMatrix();
		TraceSlices();
		TraceCompBuffers();
#endif

		IF_FAILED_THROW(hr = m_pVideoDecoder->Execute(&m_ExecuteParams));

		IF_FAILED_THROW(hr = m_pVideoDecoder->EndFrame(NULL));

		IF_FAILED_THROW(hr = m_pDeviceManager->UnlockDevice(m_hD3d9Device, FALSE));

		m_bFreeSurface[m_VideoParam.iCurPictureId] = FALSE;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pDevice);

	return hr;
}

WORD CDxva2Decoder::GetScaleCode(const BYTE* pData){

	TRACE_TRANSFORM((L"MFTDxva2::GetScaleCode"));

	/*if(pData[2] != 0x01)
	return 0;*/

	WORD code = pData[4] >> 3;

	if(pData[4] & 0x04){
		// Todo : extra slice flag
		LOG_HRESULT(E_UNEXPECTED);
	}

	//TRACE((L"Slice : %d - Code : %d", pData[3], code));

	return code;
}

void CDxva2Decoder::InitPictureParams(){

	TRACE_TRANSFORM((L"MFTDxva2::InitPictureParams"));

	m_PictureParams.wDecodedPictureIndex = m_VideoParam.iCurPictureId;
	m_PictureParams.wForwardRefPictureIndex = m_VideoParam.iForwardRefIdx;
	m_PictureParams.wBackwardRefPictureIndex = m_VideoParam.iBackwardRefIdx;

	m_PictureParams.bPicIntra = m_VideoParam.intra_pic_flag;
	m_PictureParams.bPicBackwardPrediction = !m_VideoParam.ref_pic_flag;
	m_PictureParams.bPicScanMethod = m_VideoParam.alternate_scan;

	m_PictureParams.wBitstreamFcodes = (m_VideoParam.f_code[0][0] << 12) | (m_VideoParam.f_code[0][1] << 8) | (m_VideoParam.f_code[1][0] << 4) | (m_VideoParam.f_code[1][1]);

	m_PictureParams.wBitstreamPCEelements = (m_VideoParam.intra_dc_precision << 14) | (m_VideoParam.picture_structure << 12) | (m_VideoParam.top_field_first << 11) |
		(m_VideoParam.frame_pred_frame_dct << 10) | (m_VideoParam.concealment_motion_vectors << 9) | (m_VideoParam.q_scale_type << 8) | (m_VideoParam.intra_vlc_format << 7) |
		(m_VideoParam.alternate_scan << 6) | (m_VideoParam.repeat_first_field << 5) | (m_VideoParam.chroma_420_type << 4) | (m_VideoParam.progressive_frame << 3) |
		(m_VideoParam.full_pel_forward_vector << 2) | (m_VideoParam.full_pel_backward_vector << 1);

	// Can be set only once
	m_PictureParams.wPicWidthInMBminus1 = m_VideoParam.PicWidthInMbs - 1;
	m_PictureParams.wPicHeightInMBminus1 = m_VideoParam.FrameHeightInMbs - 1;
	m_PictureParams.bMacroblockWidthMinus1 = 15;
	m_PictureParams.bMacroblockHeightMinus1 = 15;
	m_PictureParams.bBlockWidthMinus1 = 7;
	m_PictureParams.bBlockHeightMinus1 = 7;
	m_PictureParams.bBPPminus1 = 7;
	m_PictureParams.bPicStructure = 3;
	m_PictureParams.bChromaFormat = 1;
	m_PictureParams.bPicScanFixed = m_bIsMpeg1 ? 0 : 1;
	m_PictureParams.bReservedBits = m_bIsMpeg1;
}

void CDxva2Decoder::InitSliceParams(){

	TRACE_TRANSFORM((L"MFTDxva2::InitSliceParams"));

	assert(m_VideoParam.uiNumSlices <= MAX_SLICE);

	int iMB = 0;
	int iLastSlice = m_VideoParam.uiNumSlices - 2;

	for(int i = 0; i < m_VideoParam.uiNumSlices; i++){

		m_SliceInfo[i].wHorizontalPosition = 0;
		m_SliceInfo[i].wVerticalPosition = i;
		m_SliceInfo[i].dwSliceBitsInBuffer = 8 * (i < iLastSlice ? (m_pSliceOffset[i + 1] - m_pSliceOffset[i]) : (m_VideoParam.uiBitstreamDataLen - m_pSliceOffset[i]));// Todo extra flag...
		m_SliceInfo[i].dwSliceDataLocation = m_pSliceOffset[i];
		m_SliceInfo[i].bStartCodeBitOffset = 0;
		m_SliceInfo[i].bReservedBits = 0;
		m_SliceInfo[i].wMBbitOffset = 38;// We must check intra slice flag information, see GetScaleCode...
		m_SliceInfo[i].wNumberMBsInSlice = iMB;
		m_SliceInfo[i].wQuantizerScaleCode = GetScaleCode(&m_VideoParam.pBitstreamData[m_pSliceOffset[i]]);
		m_SliceInfo[i].wBadSliceChopping = 0;

		iMB += m_VideoParam.PicWidthInMbs;
	}
}

void CDxva2Decoder::InitQuantaMatrixParams(){

	TRACE_TRANSFORM((L"MFTDxva2::InitQuantaMatrixParams"));

	m_QuantaMatrix.bNewQmatrix;

	for(int i = 0; i < 4; i++)
		m_QuantaMatrix.bNewQmatrix[i] = 1;

	for(int i = 0; i < 64; i++){

		m_QuantaMatrix.Qmatrix[0][i] = m_VideoParam.QuantMatrixIntra[i];
		m_QuantaMatrix.Qmatrix[1][i] = m_VideoParam.QuantMatrixInter[i];
		m_QuantaMatrix.Qmatrix[2][i] = m_VideoParam.QuantMatrixIntra[i];
		m_QuantaMatrix.Qmatrix[3][i] = m_VideoParam.QuantMatrixInter[i];
	}
}