//----------------------------------------------------------------------------------------------
// MFTCudaDecoder_Process.cpp
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

HRESULT CMFTCudaDecoder::Slice(BYTE* pData, const DWORD dwSize){

	HRESULT hr = S_OK;

	DWORD dwParsed;
	DWORD dwCode;
	DWORD dwSmallCode;

	IF_FAILED_RETURN(hr = m_InputBuffer.Reserve(dwSize));
	memcpy(m_InputBuffer.GetReadStartBuffer(), pData, dwSize);
	IF_FAILED_RETURN(hr = m_InputBuffer.SetEndPosition(dwSize));

	do{

		hr = FindNextStartCode(m_InputBuffer.GetStartBuffer(), m_InputBuffer.GetBufferSize(), &dwParsed);

		if(m_bSlice){

			if(FAILED(m_SliceBuffer.Reserve(dwParsed))){
				LOG_HRESULT(hr = E_FAIL);
				break;
			}

			memcpy(m_SliceBuffer.GetReadStartBuffer(), m_InputBuffer.GetStartBuffer(), dwParsed);

			if(FAILED(m_SliceBuffer.SetEndPosition(dwParsed))){
				LOG_HRESULT(hr = E_FAIL);
				break;
			}
		}

		if(FAILED(m_InputBuffer.SetStartPosition(dwParsed))){
			LOG_HRESULT(hr = E_FAIL);
			break;
		}

		if(hr == S_FALSE || m_InputBuffer.GetBufferSize() < MIN_BYTE_TO_READ_MPEG_HEADER){

			hr = S_OK;
			break;
		}

		m_bSlice = FALSE;
		dwCode = MAKE_DWORD(m_InputBuffer.GetStartBuffer());

		switch(dwCode){

		case MPEG_PICTURE_START_CODE:

			if(m_SliceBuffer.GetBufferSize() != 0){

				hr = CudaDecodePicture();

				m_SliceBuffer.Reset();
				m_VideoParam.nNumSlices = 0;
			}
			hr = PictureCode(m_InputBuffer.GetStartBuffer());
			break;

		case MPEG_SEQUENCE_HEADER_CODE:
			hr = SequenceCode(m_InputBuffer.GetStartBuffer());
			break;

		case MPEG_SEQUENCE_EXTENSION_CODE:
			hr = SequenceExCode(m_InputBuffer.GetStartBuffer());
			break;

		case MPEG_SEQUENCE_END_CODE:
			//Todo
			LOG_HRESULT(hr = E_FAIL);
			break;

		default:
		{
			dwSmallCode = dwCode & 0x000000ff;

			if(dwSmallCode > 0 && dwSmallCode <= MAX_SLICE){
				hr = SliceInit(m_InputBuffer.GetStartBuffer());
			}
		}
		}

		if(FAILED(m_InputBuffer.SetStartPosition(4))){
			LOG_HRESULT(hr = E_FAIL);
			break;
		}
	} while(hr == S_OK);

	return hr;
}

HRESULT CMFTCudaDecoder::FindNextStartCode(BYTE* pData, const DWORD dwSize, DWORD* dwAte){

	HRESULT hr = S_FALSE;

	DWORD dwLeft = dwSize;

	while(dwLeft >= 4){

		if((MAKE_DWORD_HOSTORDER(pData) & 0x00FFFFFF) == 0x00010000){

			hr = S_OK;
			break;
		}

		dwLeft -= 1;
		pData += 1;
	}

	*dwAte = (dwSize - dwLeft);

	return hr;
}

HRESULT CMFTCudaDecoder::PictureCode(const BYTE* pInputData){

	HRESULT hr = S_OK;

	// WTF : Mpeg2 container, but mpeg1 video format
	if(m_bMpegHeaderEx == FALSE){

		m_bMpegHeaderEx = TRUE;
		m_bMpeg1 = TRUE;

		InitQuanta();
		m_cCudaDecoder.Release();

		IF_FAILED_RETURN(hr = m_cCudaDecoder.InitDecoder(m_cCudaManager.GetVideoLock(), m_uiFrameWidth, m_uiFrameHeight, m_bMpeg1));
	}

	m_dwTemporalReference = pInputData[4] << 2;
	m_dwTemporalReference |= (pInputData[5] >> 6);

	m_VideoParam.CodecSpecific.mpeg2.picture_coding_type = (pInputData[5] & 0x38) >> 3;

	if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == PICTURE_TYPE_I){

		m_VideoParam.intra_pic_flag = 1;
		m_VideoParam.ref_pic_flag = 1;
		m_VideoParam.CodecSpecific.mpeg2.ForwardRefIdx = -1;
		m_VideoParam.CodecSpecific.mpeg2.BackwardRefIdx = -1;
		m_iLastForward = m_iLastPictureP;
		m_cCudaFrame.StatePictureI();

		if(m_bFirstPictureI == FALSE){
			m_bFirstPictureI = TRUE;
		}
	}
	else if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == PICTURE_TYPE_P){

		m_VideoParam.intra_pic_flag = 0;
		m_VideoParam.ref_pic_flag = 1;
		m_VideoParam.CodecSpecific.mpeg2.ForwardRefIdx = m_iLastPictureP;
		m_VideoParam.CodecSpecific.mpeg2.BackwardRefIdx = -1;
		m_iLastForward = m_iLastPictureP;

		if(m_bMpeg1){

			// Todo : for now, don't know how to use it. Few files use this...
			BOOL bFullVectorForward = pInputData[7] & 0x04;
			if(bFullVectorForward)
				TRACE((L"P-bFullVectorForward"));

			m_VideoParam.CodecSpecific.mpeg2.f_code[0][0] = (((pInputData[7] << 1) | (pInputData[8] >> 7)) & 7);
			m_VideoParam.CodecSpecific.mpeg2.f_code[0][1] = m_VideoParam.CodecSpecific.mpeg2.f_code[0][0];
		}
	}
	else if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == PICTURE_TYPE_B){

		m_VideoParam.intra_pic_flag = 0;
		m_VideoParam.ref_pic_flag = 0;
		m_VideoParam.CodecSpecific.mpeg2.ForwardRefIdx = m_iLastForward;
		m_VideoParam.CodecSpecific.mpeg2.BackwardRefIdx = m_iLastPictureP;

		if(m_bMpeg1){

			BOOL bFullVectorForward = pInputData[7] & 0x04;
			BOOL bFullVectorBackward = pInputData[8] & 0x40;

			// Todo : for now, don't know how to use it. Few files use this...
			if(bFullVectorForward)
				TRACE((L"B-bFullVectorForward"));
			if(bFullVectorBackward)
				TRACE((L"B-bFullVectorBackward"));

			m_VideoParam.CodecSpecific.mpeg2.f_code[0][0] = (((pInputData[7] << 1) | (pInputData[8] >> 7)) & 7);
			m_VideoParam.CodecSpecific.mpeg2.f_code[0][1] = m_VideoParam.CodecSpecific.mpeg2.f_code[0][0];
			m_VideoParam.CodecSpecific.mpeg2.f_code[1][0] = ((pInputData[8] >> 3) & 7);
			m_VideoParam.CodecSpecific.mpeg2.f_code[1][1] = m_VideoParam.CodecSpecific.mpeg2.f_code[1][0];
		}
	}
	else{
		// picture_coding_type != 1-2-3 (Todo)
		LOG_HRESULT(hr = E_FAIL);
		return hr;
	}

	if(m_bFirstPictureI)
		m_cCudaFrame.CheckPictureTime();

	return hr;
}

HRESULT CMFTCudaDecoder::SequenceCode(const BYTE* pInputData){

	HRESULT hr = S_OK;

	m_uiFrameWidth = pInputData[4] << 4;
	m_uiFrameWidth |= ((pInputData[5] & 0xf0) >> 4);

	m_uiFrameHeight = ((pInputData[5] & 0x0f) << 8);
	m_uiFrameHeight |= pInputData[6];

	m_VideoParam.PicWidthInMbs = (m_uiFrameWidth + 15) / 16;
	m_VideoParam.FrameHeightInMbs = (m_uiFrameHeight + 15) / 16;

	if(!m_bMpeg1){

		//uiQuanta = pInputData[11] & 0x04;

		if(pInputData[11] & 0x04){

			// QuantaIntra != 0 (Todo)
			// Seems not to be fatal...
			/*LOG_HRESULT(hr = E_FAIL);
			return hr;*/

			//UINT uiQuanta;
			//int iIndex = 11;

			/*for(int i = 0; i < 64; i++){

			  uiQuanta = (pInputData[iIndex++] & 0x03) << 6;
					uiQuanta |= (pInputData[iIndex] >> 2);
					m_VideoParam.CodecSpecific.mpeg2.QuantMatrixIntra[i] = (BYTE)uiQuanta;
			}*/
		}

		//uiQuanta = ((pInputData[11] & 0x02) >> 1);

		if(pInputData[11] & 0x02){

			// QuantaInter != 0 (Todo)
							  // Seems not to be fatal...
			//LOG_HRESULT(hr = E_FAIL);
		}
	}

	return hr;
}

HRESULT CMFTCudaDecoder::SequenceExCode(const BYTE* pInputData){

	HRESULT hr = S_OK;

	m_bMpegHeaderEx = TRUE;
	DWORD dwCodeEx = ((pInputData[4] & 0xf0) >> 4);

	if(dwCodeEx == PICTURE_EXTENSION_CODE){

		m_VideoParam.CodecSpecific.mpeg2.f_code[0][0];

		m_VideoParam.CodecSpecific.mpeg2.f_code[0][0] = (pInputData[4] & 0x0f);
		m_VideoParam.CodecSpecific.mpeg2.f_code[0][1] = ((pInputData[5] & 0xf0) >> 4);
		m_VideoParam.CodecSpecific.mpeg2.f_code[1][0] = (pInputData[5] & 0x0f);
		m_VideoParam.CodecSpecific.mpeg2.f_code[1][1] = ((pInputData[6] & 0xf0) >> 4);

		m_VideoParam.CodecSpecific.mpeg2.intra_dc_precision = ((pInputData[6] & 0x0c) >> 2);
		//m_VideoParam.CodecSpecific.mpeg2.uiPictureStructure = (pInputData[6] & 0x03);
		m_VideoParam.CodecSpecific.mpeg2.top_field_first = ((pInputData[7] & 0x80) >> 7);
		m_VideoParam.CodecSpecific.mpeg2.frame_pred_frame_dct = ((pInputData[7] & 0x40) >> 6);
		m_VideoParam.CodecSpecific.mpeg2.concealment_motion_vectors = ((pInputData[7] & 0x20) >> 5);
		m_VideoParam.CodecSpecific.mpeg2.q_scale_type = ((pInputData[7] & 0x10) >> 4);
		m_VideoParam.CodecSpecific.mpeg2.intra_vlc_format = ((pInputData[7] & 0x08) >> 3);
		m_VideoParam.CodecSpecific.mpeg2.alternate_scan = ((pInputData[7] & 0x04) >> 2);
		//m_VideoParam.CodecSpecific.mpeg2.uiRepeatFirstField = ((pInputData[7] & 0x02) >> 1);
		//m_VideoParam.CodecSpecific.mpeg2.uiChromatType = (pInputData[7] & 0x01);
		//m_VideoParam.CodecSpecific.mpeg2.uiProgressiveFrame = ((pInputData[8] & 0x80) >> 7);
		//m_VideoParam.CodecSpecific.mpeg2.uiCompositeDisplayFlag = ((pInputData[8] & 0x40) >> 6);
	}
	else if(dwCodeEx == SEQUENCE_EXTENSION_CODE){

		if((pInputData[5] & 0x08) == 0x08){

			m_bProgressive = TRUE;
			m_VideoParam.field_pic_flag = 0;
			m_VideoParam.bottom_field_flag = 0;
			m_VideoParam.second_field = 0;
		}
		else{

			m_bProgressive = FALSE;
			// Progressive != 1 (Todo)
			LOG_HRESULT(hr = E_FAIL);
		}

		UINT uiChromatFmt = ((pInputData[5] & 0x06) >> 1);

		if(uiChromatFmt != MPEG_CHROMAT_FORMAT_420){

			// Chromat != 420 (Todo)
			LOG_HRESULT(hr = E_FAIL);
		}
	}
	else{

		// QUANTA_EXTENSION_CODE (Todo)
		if(dwCodeEx == QUANTA_EXTENSION_CODE){
			LOG_HRESULT(hr = E_FAIL);
		}
	}

	return hr;
}

HRESULT CMFTCudaDecoder::SliceInit(const BYTE* pInputData){

	HRESULT hr = S_OK;

	m_pSliceOffset[m_VideoParam.nNumSlices] = m_SliceBuffer.GetBufferSize();
	m_VideoParam.nNumSlices++;
	m_bSlice = TRUE;

	if(FAILED(hr = m_SliceBuffer.Reserve(4))){
		LOG_HRESULT(hr);
		return hr;
	}

	memcpy(m_SliceBuffer.GetReadStartBuffer(), pInputData, 4);

	if(FAILED(hr = m_SliceBuffer.SetEndPosition(4))){
		LOG_HRESULT(hr);
	}

	return hr;
}

HRESULT CMFTCudaDecoder::CudaDecodePicture(){

	HRESULT hr = S_OK;

	if(m_bFirstPictureI == FALSE){
		return hr;
	}

	m_VideoParam.CurrPicIdx = m_iCurIDX++;

	if(m_iCurIDX == MAX_CUDA_DECODED_SURFACES)
		m_iCurIDX = 0;

	m_VideoParam.nBitstreamDataLen = m_SliceBuffer.GetBufferSize();
	m_VideoParam.pBitstreamData = m_SliceBuffer.GetStartBuffer();
	m_VideoParam.pSliceDataOffsets = m_pSliceOffset;

#ifdef TRACE_PICTURE_PARAM
	TracePictureParam();
#endif

#ifdef TRACE_SLICE
	TraceSlice();
#endif

	if(m_cCudaFrame.IsFull()){

		SCUDAFRAME* pFrame = m_cCudaFrame.GetNextFrame();

		IF_FAILED_RETURN(hr = DecodePicture(pFrame));

#ifdef TRACE_PICTURE_OUTPUT
		TracePicture(pFrame->iIndex, pFrame->iPType, pFrame->iTemporal, pFrame->rtTime);
#endif
	}

	m_VideoParam.CurrPicIdx = m_cCudaFrame.GetFreeIDX();

	assert(m_cCudaFrame.AddFrame(m_VideoParam.CurrPicIdx, m_VideoParam.CodecSpecific.mpeg2.picture_coding_type, m_dwTemporalReference));

#ifdef TRACE_TIME_REFERENCE
	if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == 1)
		TRACE((L"decodePicture : I"));
	else if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == 2)
		TRACE((L"decodePicture : P"));
	else if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == 3)
		TRACE((L"decodePicture : B"));
	else
		TRACE((L"decodePicture : ERROR"));
#endif

	IF_FAILED_RETURN(hr = m_cCudaDecoder.DecodePicture(&m_VideoParam));

	if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type != PICTURE_TYPE_B)
		m_iLastPictureP = m_VideoParam.CurrPicIdx;

	return hr;
}

HRESULT CMFTCudaDecoder::DecodePicture(const SCUDAFRAME* pFrame){

	HRESULT hr = S_OK;

	CUVIDPROCPARAMS ProcessParam;
	UINT uiDecodedPitch;
	CUdeviceptr pDecodedFrame = 0;

	memset(&ProcessParam, 0, sizeof(CUVIDPROCPARAMS));

	if(m_bProgressive){

		ProcessParam.progressive_frame = 1;
		ProcessParam.second_field = 0;
		ProcessParam.top_field_first = 0;
		ProcessParam.unpaired_field = 1;
	}
	else{
		// Todo interlaced
		IF_FAILED_RETURN(hr = E_FAIL);
	}

	CCtxAutoLock lck(m_cCudaManager.GetVideoLock());
	CUresult Result = cuCtxPushCurrent(m_cCudaManager.GetCudaContext());

	IF_FAILED_RETURN(hr = (Result != CUDA_SUCCESS ? E_FAIL : S_OK));

	IMFMediaBuffer* pOutputBuffer = NULL;
	BYTE* pbOutputData = NULL;
	DWORD dwOutputLength = 0;
	BOOL bNeedUnmap = FALSE;

	IMFSample* pSample = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateSample(&pSample));

		IF_FAILED_THROW(hr = m_cCudaDecoder.MapFrame(pFrame->iIndex, &pDecodedFrame, &uiDecodedPitch, &ProcessParam));
		bNeedUnmap = TRUE;

		if(m_dwCudaFrameSize == 0){

			m_dwCudaFrameSize = (uiDecodedPitch * m_uiFrameHeight * 3 / 2);

			IF_FAILED_THROW(hr = m_cCudaManager.InitFrameCuda(m_uiFrameHeight, uiDecodedPitch));
		}

		CUresult Result = cuMemcpyDtoHAsync(m_cCudaManager.GetFrame(), pDecodedFrame, m_dwCudaFrameSize, m_cCudaManager.GetCudaStream());

		IF_FAILED_THROW(hr = (Result != CUDA_SUCCESS ? E_FAIL : S_OK));

		bNeedUnmap = FALSE;
		IF_FAILED_THROW(hr = m_cCudaDecoder.UnmapFrame(pDecodedFrame));

		Result = cuCtxPopCurrent(NULL);
		IF_FAILED_THROW(hr = (Result != CUDA_SUCCESS ? E_FAIL : S_OK));

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(m_dwSampleSize, &pOutputBuffer));
		IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(m_dwSampleSize));
		IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pbOutputData, NULL, &dwOutputLength));

		if(m_bNV12){
			IF_FAILED_THROW(hr = MFCopyImage(pbOutputData, m_uiFrameWidth, m_cCudaManager.GetFrame(), uiDecodedPitch, m_uiFrameWidth, m_uiFrameHeight * 3 / 2));
		}
		else{
			m_cCudaManager.SetNV12ToYV12(&pbOutputData, m_uiFrameWidth, m_uiFrameHeight, uiDecodedPitch);
		}

		IF_FAILED_THROW(hr = pOutputBuffer->Unlock());
		pbOutputData = NULL;

		IF_FAILED_THROW(hr = pSample->AddBuffer(pOutputBuffer));

		UINT32 uiPicture = 0;

		switch(pFrame->iPType){

		case PICTURE_TYPE_I: uiPicture = 1; break;
		case PICTURE_TYPE_P: uiPicture = 2; break;
		case PICTURE_TYPE_B: uiPicture = 3; break;
		}

		IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_PictureType, uiPicture));
		IF_FAILED_THROW(hr = pSample->SetSampleTime(pFrame->rtTime));

		/*if(pFrame->iPType == 1){
				IF_FAILED_THROW(hr = pSample->SetSampleTime(pFrame->rtTime));
		}
		else{
				IF_FAILED_THROW(hr = pSample->SetSampleTime(-1));
		}*/

		// Can fail...
		m_qSampleOut.push(pSample);
		pSample->AddRef();
	}
	catch(HRESULT){}

	if(bNeedUnmap){
		LOG_HRESULT(m_cCudaDecoder.UnmapFrame(pDecodedFrame));
	}

	if(pOutputBuffer && pbOutputData){
		LOG_HRESULT(pOutputBuffer->Unlock());
	}

	SAFE_RELEASE(pOutputBuffer);
	SAFE_RELEASE(pSample);

	return hr;
}

#ifdef TRACE_PICTURE_OUTPUT
void CMFTCudaDecoder::TracePicture(const int iCurIDX, const int iPictureType, const DWORD dwTemporalReference, const REFERENCE_TIME rtTime){

	switch(iPictureType){

	case PICTURE_TYPE_I: TRACE((L"I-%d-%d Time = %d", dwTemporalReference, iCurIDX, rtTime)); break;
	case PICTURE_TYPE_P: TRACE((L"P-%d-%d Time = %d", dwTemporalReference, iCurIDX, rtTime)); break;
	case PICTURE_TYPE_B: TRACE((L"B-%d-%d Time = %d", dwTemporalReference, iCurIDX, rtTime)); break;
	default: TRACE((L"Unknown-%d-%d Time = %d", dwTemporalReference, iCurIDX));
	}
}
#endif

#ifdef TRACE_PICTURE_PARAM
void CMFTCudaDecoder::TracePictureParam(){

	static int iSlice = 0;
	iSlice++;

	TRACE((L"Slice : %d\r\nPicWidthInMbs:%d", iSlice, m_VideoParam.PicWidthInMbs));
	TRACE((L"FrameHeightInMbs:%d", m_VideoParam.FrameHeightInMbs));
	TRACE((L"CurrPicIdx:%d", m_VideoParam.CurrPicIdx));
	TRACE((L"field_pic_flag:%d", m_VideoParam.field_pic_flag));
	TRACE((L"bottom_field_flag:%d", m_VideoParam.bottom_field_flag));
	TRACE((L"PicWidthInMbs:%d", m_VideoParam.second_field));
	TRACE((L"nBitstreamDataLen:%d", m_VideoParam.nBitstreamDataLen));
	TRACE_NO_END_LINE((L"nNumSlices:%d\r\npSliceDataOffsets", m_VideoParam.nNumSlices));

	for(unsigned int i = 0; i < m_VideoParam.nNumSlices; i++)
		TRACE_NO_END_LINE((L":%d", m_VideoParam.pSliceDataOffsets[i]));

	TRACE((L"\r\nref_pic_flag:%d", m_VideoParam.ref_pic_flag));
	TRACE((L"intra_pic_flag:%d", m_VideoParam.intra_pic_flag));

	CUVIDMPEG2PICPARAMS Mpeg2Param = m_VideoParam.CodecSpecific.mpeg2;

	TRACE((L"ForwardRefIdx:%d", Mpeg2Param.ForwardRefIdx));
	TRACE((L"BackwardRefIdx:%d", Mpeg2Param.BackwardRefIdx));
	TRACE((L"picture_coding_type:%d", Mpeg2Param.picture_coding_type));
	TRACE((L"full_pel_forward_vector:%d", Mpeg2Param.full_pel_forward_vector));
	TRACE_NO_END_LINE((L"full_pel_backward_vector:%d\r\nf_code", Mpeg2Param.full_pel_backward_vector));

	for(int i = 0; i < 2; i++)
		for(int j = 0; j < 2; j++)
			TRACE_NO_END_LINE((L":%d", Mpeg2Param.f_code[i][j]));

	TRACE((L"\r\nintra_dc_precision:%d", Mpeg2Param.intra_dc_precision));
	TRACE((L"frame_pred_frame_dct:%d", Mpeg2Param.frame_pred_frame_dct));
	TRACE((L"concealment_motion_vectors:%d", Mpeg2Param.concealment_motion_vectors));
	TRACE((L"q_scale_type:%d", Mpeg2Param.q_scale_type));
	TRACE((L"intra_vlc_format:%d", Mpeg2Param.intra_vlc_format));
	TRACE((L"alternate_scan:%d", Mpeg2Param.alternate_scan));
	TRACE_NO_END_LINE((L"top_field_first:%d\r\nQuantMatrixIntra", Mpeg2Param.top_field_first));

	for(int i = 0; i < 64; i++)
		TRACE_NO_END_LINE((L":%d", Mpeg2Param.QuantMatrixIntra[i]));

	TRACE_NO_END_LINE((L"\r\nQuantMatrixInter"));

	for(int i = 0; i < 64; i++)
		TRACE_NO_END_LINE((L":%d", Mpeg2Param.QuantMatrixInter[i]));

	TRACE((L"\r\n"));
}
#endif

#ifdef TRACE_SLICE
void CMFTCudaDecoder::TraceSlice(){

	static int iSlice = 0;
	iSlice++;

	WCHAR wszBuf[2] = {0};
	if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == PICTURE_TYPE_I)
		wszBuf[0] = 'I';
	else if(m_VideoParam.CodecSpecific.mpeg2.picture_coding_type == PICTURE_TYPE_P)
		wszBuf[0] = 'P';
	else
		wszBuf[0] = 'B';

	TRACE((L"Slice%d\r", iSlice));
	TRACE((L"Frame %S\r", wszBuf));
	TRACE((L"Number Slice : %d\rBuffer Size : %d\r", m_VideoParam.nNumSlices, m_VideoParam.nBitstreamDataLen));
	TRACE_NO_END_LINE((L"Offset"));

	for(DWORD i = 0; i < m_VideoParam.nNumSlices; i++)
		TRACE_NO_END_LINE((L":%d", m_VideoParam.pSliceDataOffsets[i]));

	TRACE((L"\r\r"));
}
#endif