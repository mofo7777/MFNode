//----------------------------------------------------------------------------------------------
// MFTVideoMpeg2_Process.cpp
// Copyright (C) 2012 Dumonteil David
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

HRESULT CMFTVideoMpeg2::Mpeg2ProcessInput(){

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((m_pMpeg2Decoder == NULL || m_pInputBuffer == NULL) ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = m_pMpeg2Decoder->Mpeg2SetInputBuffer(m_pbInputData, m_dwInputLength));

	bool bContinue = true;

	while(bContinue){

		int state = m_pMpeg2Decoder->Mpeg2DecodeFrame();

		switch(state){

			// Check invalid state
		case STATE_INVALID:
			//if(m_fWaitForKeyFrame)
			  //ResetMpeg2Decoder();
		case STATE_BUFFER:

			ReleaseInputBuffer();
			hr = S_OK;
			bContinue = false;
			break;

		case STATE_SEQUENCE:

		{
			DWORD dwPeriod = m_pMpeg2Decoder->GetPeriod();
			REFERENCE_TIME rtTest = 10i64 * dwPeriod / 27;

			if(rtTest != 0){
				m_rtAvgPerFrame = rtTest;
			}
			else{
				m_rtAvgPerFrame = m_rtAvgPerFrameInput;
			}
		}
		break;

		// Check picture state
		case STATE_PICTURE:

			//m_rtFrame = 0;

			/*{
					REFERENCE_TIME rtTest = m_pMpeg2Decoder->GetBitRate();

					if(rtTest == 0)
							rtTest = 0;

					//m_dec.m_rtStart = rtStart;
					//rtStart = _I64_MIN;
					//m_dec.m_fDelivered = false;

			}*/
			break;

		case STATE_SLICE:
		case STATE_END:

			if(m_pMpeg2Decoder->Mpeg2HaveImage()){

				m_dwNumVideoFrame++;
				LOG_HRESULT(hr = Mpeg2SetImage());
			}
			break;
		}
	}
	return hr;
}

HRESULT CMFTVideoMpeg2::Mpeg2SetImage(){

	//TRACE((L"Mpeg2SetImage"));

	assert(m_pSampleOut);

	const unsigned char* ucBufY = NULL;
	const unsigned char* ucBufU = NULL;
	const unsigned char* ucBufV = NULL;

	ucBufY = m_pMpeg2Decoder->Mpeg2GetDisplayBufferY();
	ucBufU = m_pMpeg2Decoder->Mpeg2GetDisplayBufferU();
	ucBufV = m_pMpeg2Decoder->Mpeg2GetDisplayBufferV();

	const unsigned int uiSize = m_pMpeg2Decoder->GetWidth();
	BOOL bStride = (uiSize != m_uiWidthInPixels2);

	HRESULT hr;
	IF_FAILED_RETURN(hr = ((ucBufY == NULL || ucBufU == NULL || ucBufV == NULL) ? E_FAIL : S_OK));

	IMFMediaBuffer* pOutputBuffer = NULL;
	BYTE* pbOutputData = NULL;
	DWORD dwOutputLength = 0;

	try{

		IF_FAILED_THROW(hr = MFCreateMemoryBuffer(m_dwSampleSize2, &pOutputBuffer));
		IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(m_dwSampleSize2));
		IF_FAILED_THROW(hr = pOutputBuffer->Lock(&pbOutputData, NULL, &dwOutputLength));

		// Compare performance...
		/*DWORD dwSize = m_uiWidthInPixels2 * m_uiHeightInPixels2;
		unsigned char* dest[3];
		dest[0] = pbOutputData;
		dest[1] = dest[0] + dwSize;
		dest[2] = dest[1] + (dwSize >> 2);

		memcpy(dest[0], ucBufY, dwSize);
		memcpy(dest[1], ucBufU, dwSize / 4);
		memcpy(dest[2], ucBufV, dwSize / 4);*/

		if(bStride)
			Mpeg2CopyFrameYV12Stride(uiSize, m_uiWidthInPixels2, m_uiHeightInPixels2, pbOutputData, ucBufY, ucBufU, ucBufV);
		else
			Mpeg2CopyFrameYV12(m_uiWidthInPixels2, m_uiHeightInPixels2, pbOutputData, ucBufY, ucBufU, ucBufV);

		//CMFWriteFile cMFWriteFile;
		//cMFWriteFile.MFCreateAndWriteFile(L"C:\\projet\\media\\rgb24\\image.yv12", pbOutputData, dwOutputLength);

		hr = pOutputBuffer->Unlock();
		pbOutputData = NULL;

		// Check if multi buffering is needed, here only one buffer before output
		IF_FAILED_THROW(hr = m_pSampleOut->AddBuffer(pOutputBuffer));
	}
	catch(HRESULT){}

	if(FAILED(hr)){
		ResetOutSample();
	}

	if(pOutputBuffer && pbOutputData){
		LOG_HRESULT(pOutputBuffer->Unlock());
	}

	SAFE_RELEASE(pOutputBuffer);

	return hr;
}

HRESULT CMFTVideoMpeg2::Mpeg2ProcessOutput(IMFSample* pSample, DWORD* pdwStatus){

	//TRACE((L"Mpeg2ProcessOutput"));

	assert(m_pSampleOut);

	// No need, already verify by the caller
	//if(pSample == NULL){
			//return E_POINTER;
	//}

	HRESULT hr = S_OK;
	IMFMediaBuffer* pOutput = NULL;

	// Get the output buffer.
	IF_FAILED_RETURN(hr = pSample->GetBufferByIndex(0, &pOutput));

	DWORD cbLenght = 0;

	IF_FAILED_RETURN(hr = pOutput->GetMaxLength(&cbLenght));

	IF_FAILED_RETURN(hr = (cbLenght < m_dwSampleSize2 ? E_INVALIDARG : S_OK));

	IMFMediaBuffer* pCurrentOutputBuf = NULL;
	BYTE* pCurOutputData = NULL;
	DWORD dwCurOutputLength = 0;

	BYTE* pOutputData = NULL;
	DWORD dwOutputLength = 0;

	try{

		IF_FAILED_THROW(hr = m_pSampleOut->GetBufferByIndex(0, &pCurrentOutputBuf));

		//SAFE_RELEASE(m_pOuputLastBuffer);
		//m_pOuputLastBuffer = pCurrentOutputBuf;
		//m_pOuputLastBuffer->AddRef();

		IF_FAILED_THROW(hr = pCurrentOutputBuf->Lock(&pCurOutputData, NULL, &dwCurOutputLength));
		IF_FAILED_THROW(hr = pOutput->Lock(&pOutputData, NULL, &dwOutputLength));

		assert(dwOutputLength <= dwCurOutputLength);

		//memcpy(pOutputData, pCurOutputData, m_dwSampleSize);
		//m_pBufferToFile.WriteBufferToFile(pCurOutputData, m_dwSampleSize);
		Mpeg2CopyMmx(pOutputData, pCurOutputData, m_dwSampleSize2);

		LOG_HRESULT(hr = pCurrentOutputBuf->Unlock());
		LOG_HRESULT(hr = pOutput->Unlock());

		pCurOutputData = NULL;
		pOutputData = NULL;

		IF_FAILED_THROW(hr = pOutput->SetCurrentLength(m_dwSampleSize2));

		// Clean point / key frame
		IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));

		IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtFrame));
		//IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtFrameTest));
		IF_FAILED_THROW(hr = pSample->SetSampleDuration(m_rtAvgPerFrame));

		//static int iFrame = 0;
		//TRACE_TIME((L"%d:%I64d:%I64d", iFrame, m_rtFrame, m_rtAvgPerFrame));
		//iFrame++;

		/*if(m_bDiscontinuity){

				//m_bDiscontinuity = FALSE;
				IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE));
		}*/

		/*if(m_bSetDiscontinuity){

				m_bSetDiscontinuity = FALSE;
				IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE));
		}*/

		/*static DWORD dwIndex = 0;
		TRACE((L"%d - SampleTime : %I64d - AvgPerFrame : %I64d", dwIndex, m_rtFrame, m_rtAvgPerFrame));
		dwIndex++;*/

		//m_rtFrameTest += m_rtAvgPerFrame;
		m_rtFrame += m_rtAvgPerFrame;
	}
	catch(HRESULT){ LOG_HRESULT(hr); }

	if(pCurrentOutputBuf && pCurOutputData){
		LOG_HRESULT(pCurrentOutputBuf->Unlock());
	}

	if(pOutput && pOutputData){
		LOG_HRESULT(pOutput->Unlock());
	}

	SAFE_RELEASE(pCurrentOutputBuf);
	SAFE_RELEASE(pOutput);

	if(SUCCEEDED(hr)){

		IF_FAILED_RETURN(hr = m_pSampleOut->RemoveBufferByIndex(0));

		IF_FAILED_RETURN(hr = m_pSampleOut->GetBufferCount(&cbLenght));

		if(cbLenght == 0){
			ResetOutSample();
			*pdwStatus = 0;
		}
		else{
			*pdwStatus |= MFT_OUTPUT_DATA_BUFFER_INCOMPLETE;
		}
	}
	//else{

			// What to do if failed ?
	//}

	return hr;
}