//----------------------------------------------------------------------------------------------
// Dxva2Decoder_Slice.cpp
// Copyright (C) 2015 Dumonteil David
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

static const unsigned char ff_zigzag_direct[64] = {
		0, 1, 8, 16, 9, 2, 3, 10,
		17, 24, 32, 25, 18, 11, 4, 5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13, 6, 7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63
};

HRESULT CDxva2Decoder::Slice(BYTE* pData, const DWORD dwSize){

		TRACE_TRANSFORM((L"MFTDxva2::Slice"));

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

										LOG_HRESULT(hr = SchedulePicture());

										m_SliceBuffer.Reset();
										m_VideoParam.uiNumSlices = 0;

										if(m_bHaveOuput)
												return hr;
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
								//LOG_HRESULT(hr = E_FAIL);
								return hr;
								//break;

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
		}
		while(hr == S_OK);

		return hr;
}

HRESULT CDxva2Decoder::FindNextStartCode(BYTE* pData, const DWORD dwSize, DWORD* dwAte){

		TRACE_TRANSFORM((L"MFTDxva2::FindNextStartCode"));

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

HRESULT CDxva2Decoder::PictureCode(const BYTE* pInputData){

		TRACE_TRANSFORM((L"MFTDxva2::PictureCode"));

		HRESULT hr = S_OK;

		// WTF : Mpeg2 container, but mpeg1 video format
		if(m_bMpegHeaderEx == FALSE){

				m_bMpegHeaderEx = TRUE;
				m_bIsMpeg1 = TRUE;
				m_bProgressive = TRUE;

				InitDxva2Buffers();
		}

		m_dwTemporalReference = pInputData[4] << 2;
		m_dwTemporalReference |= (pInputData[5] >> 6);

		m_VideoParam.picture_coding_type = (pInputData[5] & 0x38) >> 3;

		if(m_VideoParam.picture_coding_type == PICTURE_TYPE_I){

				m_VideoParam.intra_pic_flag = 1;
				m_VideoParam.ref_pic_flag = 1;
				m_VideoParam.iForwardRefIdx = -1;
				m_VideoParam.iBackwardRefIdx = -1;
				m_iLastForward = m_iLastPictureP;
				m_cTSScheduler.StatePictureI();

				if(m_bIsMpeg1){
						m_VideoParam.full_pel_forward_vector = 0;
						m_VideoParam.full_pel_backward_vector = 0;
				}

				if(m_bFirstPictureI == FALSE){
						m_bFirstPictureI = TRUE;
				}
		}
		else if(m_VideoParam.picture_coding_type == PICTURE_TYPE_P){

				m_VideoParam.intra_pic_flag = 0;
				m_VideoParam.ref_pic_flag = 1;
				m_VideoParam.iForwardRefIdx = m_iLastPictureP;
				m_VideoParam.iBackwardRefIdx = -1;
				m_iLastForward = m_iLastPictureP;

				if(m_bIsMpeg1){

						m_VideoParam.full_pel_forward_vector = pInputData[7] & 0x04;
						m_VideoParam.full_pel_backward_vector = 0;

						m_VideoParam.f_code[0][0] = (((pInputData[7] << 1) | (pInputData[8] >> 7)) & 7);
						m_VideoParam.f_code[0][1] = m_VideoParam.f_code[0][0];
				}
		}
		else if(m_VideoParam.picture_coding_type == PICTURE_TYPE_B){

				m_VideoParam.intra_pic_flag = 0;
				m_VideoParam.ref_pic_flag = 0;
				m_VideoParam.iForwardRefIdx = m_iLastForward;
				m_VideoParam.iBackwardRefIdx = m_iLastPictureP;

				if(m_bIsMpeg1){

						m_VideoParam.full_pel_forward_vector = pInputData[7] & 0x04;
						m_VideoParam.full_pel_backward_vector = pInputData[8] & 0x40;

						m_VideoParam.f_code[0][0] = (((pInputData[7] << 1) | (pInputData[8] >> 7)) & 7);
						m_VideoParam.f_code[0][1] = m_VideoParam.f_code[0][0];
						m_VideoParam.f_code[1][0] = ((pInputData[8] >> 3) & 7);
						m_VideoParam.f_code[1][1] = m_VideoParam.f_code[1][0];
				}
		}
		else{
				// picture_coding_type != 1-2-3 (Todo)
				LOG_HRESULT(hr = E_FAIL);
				return hr;
		}

		if(m_bFirstPictureI)
				m_cTSScheduler.CheckPictureTime();

		return hr;
}

HRESULT CDxva2Decoder::SequenceCode(const BYTE* pInputData){

		TRACE_TRANSFORM((L"MFTDxva2::SequenceCode"));

		HRESULT hr = S_OK;

		UINT uiFrameWidth = pInputData[4] << 4;
		uiFrameWidth |= ((pInputData[5] & 0xf0) >> 4);

		UINT uiFrameHeight = ((pInputData[5] & 0x0f) << 8);
		uiFrameHeight |= pInputData[6];

		m_VideoParam.PicWidthInMbs = (uiFrameWidth + 15) / 16;
		m_VideoParam.FrameHeightInMbs = (uiFrameHeight + 15) / 16;

		if(pInputData[11] & 0x02){

				const BYTE* pIntra = &pInputData[11];
				int j;

				for(int i = 0; i < 64; i++){

						j = ff_zigzag_direct[i];
						m_VideoParam.QuantMatrixIntra[j] = ((pIntra[i] & 0x01) << 7) || (pIntra[i + 1] >> 1);
				}

				if(pInputData[75] & 0x01){

						const BYTE* pInter = &pInputData[76];

						for(int i = 0; i < 64; i++){

								j = ff_zigzag_direct[i];
								m_VideoParam.QuantMatrixInter[j] = pInter[i];
						}
				}
		}
		else if(pInputData[11] & 0x01){

				const BYTE* pQuanta = &pInputData[12];
				int j;

				for(int i = 0; i < 64; i++){

						j = ff_zigzag_direct[i];
						m_VideoParam.QuantMatrixInter[j] = pQuanta[i];
				}
		}

		return hr;
}

HRESULT CDxva2Decoder::SequenceExCode(const BYTE* pInputData){

		TRACE_TRANSFORM((L"MFTDxva2::SequenceExCode"));

		HRESULT hr = S_OK;

		m_bMpegHeaderEx = TRUE;
		DWORD dwCodeEx = ((pInputData[4] & 0xf0) >> 4);

		if(dwCodeEx == PICTURE_EXTENSION_CODE){

				m_VideoParam.f_code[0][0] = (pInputData[4] & 0x0f);
				m_VideoParam.f_code[0][1] = ((pInputData[5] & 0xf0) >> 4);
				m_VideoParam.f_code[1][0] = (pInputData[5] & 0x0f);
				m_VideoParam.f_code[1][1] = ((pInputData[6] & 0xf0) >> 4);

				BYTE bt[2];
				bt[0] = pInputData[6];
				bt[1] = pInputData[7];

				m_VideoParam.intra_dc_precision = ((pInputData[6] & 0x0c) >> 2);
				m_VideoParam.picture_structure = (pInputData[6] & 0x03);
				m_VideoParam.top_field_first = ((pInputData[7] & 0x80) >> 7);
				m_VideoParam.frame_pred_frame_dct = ((pInputData[7] & 0x40) >> 6);
				m_VideoParam.concealment_motion_vectors = ((pInputData[7] & 0x20) >> 5);
				m_VideoParam.q_scale_type = ((pInputData[7] & 0x10) >> 4);
				m_VideoParam.intra_vlc_format = ((pInputData[7] & 0x08) >> 3);
				m_VideoParam.alternate_scan = ((pInputData[7] & 0x04) >> 2);
				m_VideoParam.repeat_first_field = ((pInputData[7] & 0x02) >> 1);
				m_VideoParam.chroma_420_type = (pInputData[7] & 0x01);
				m_VideoParam.progressive_frame = ((pInputData[8] & 0x80) >> 7);
				//m_VideoParam.uiCompositeDisplayFlag = ((pInputData[8] & 0x40) >> 6);

				if(m_VideoParam.picture_structure != 3){
						// Interlaced (Todo)
						LOG_HRESULT(hr = E_FAIL);
				}
		}
		else if(dwCodeEx == SEQUENCE_EXTENSION_CODE){

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

HRESULT CDxva2Decoder::SliceInit(const BYTE* pInputData){

		TRACE_TRANSFORM((L"MFTDxva2::SliceInit"));

		HRESULT hr = S_OK;

		m_pSliceOffset[m_VideoParam.uiNumSlices] = m_SliceBuffer.GetBufferSize();
		m_VideoParam.uiNumSlices++;
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

void CDxva2Decoder::InitQuanta(){

		TRACE_TRANSFORM((L"MFTDxva2::InitQuanta"));

		memset(m_VideoParam.QuantMatrixInter, 16, 64);

		// Zigzag apply, need to learn more...
		BYTE btQuantIntra[64] = {
				8, 16, 16, 19, 16, 19, 22, 22, 22, 22,
				22, 22, 26, 24, 26, 27, 27, 27, 26, 26,
				26, 26, 27, 27, 27, 29, 29, 29, 34, 34,
				34, 29, 29, 29, 27, 27, 29, 29, 32, 32,
				34, 34, 37, 38, 37, 35, 35, 34, 35, 38,
				38, 40, 40, 40, 48, 48, 46, 46, 56, 56,
				58, 69, 69, 83
		};

		memcpy(m_VideoParam.QuantMatrixIntra, btQuantIntra, 64);
}