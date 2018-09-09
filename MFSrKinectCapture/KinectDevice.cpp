//----------------------------------------------------------------------------------------------
// KinectDevice.cpp
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

CKinectDevice::CKinectDevice()
	: m_pNuiSensor(NULL),
	m_hNextKinectFrameEvent(INVALID_HANDLE_VALUE),
	m_hKinectStreamHandle(INVALID_HANDLE_VALUE),
	m_pAudioStream(NULL),
	m_uiWidthInPixels(KINECT_VIDEO_WIDTH),
	m_uiHeightInPixels(KINECT_VIDEO_HEIGHT){

	m_FrameRate.Numerator = KINECT_VIDEO_NUMERATOR;
	m_FrameRate.Denominator = KINECT_VIDEO_DENOMINATOR;
}

void CKinectDevice::ReleaseKinect(){

	if(m_pNuiSensor){
		m_pNuiSensor->NuiShutdown();
	}

	if(m_hNextKinectFrameEvent != INVALID_HANDLE_VALUE){
		CLOSE_HANDLE_IF(m_hNextKinectFrameEvent);
	}

	SAFE_RELEASE(m_pNuiSensor);
	SAFE_RELEASE(m_pAudioStream);
}

HRESULT CKinectDevice::Initialize(){

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pNuiSensor != NULL ? MF_E_UNEXPECTED : S_OK));

	INuiSensor* pNuiSensor;
	int iSensorCount = 0;

	try{

		IF_FAILED_THROW(hr = NuiGetSensorCount(&iSensorCount));

		// Look at each Kinect sensor
		for(int i = 0; i < iSensorCount; ++i){

			// Create the sensor so we can check status, if we can't create it, move on to the next
			hr = NuiCreateSensorByIndex(i, &pNuiSensor);

			if(FAILED(hr)){
				continue;
			}

			// Get the status of the sensor, and if connected, then we can initialize it
			hr = pNuiSensor->NuiStatus();

			if(hr == S_OK){

				m_pNuiSensor = pNuiSensor;
				break;
			}

			// This sensor wasn't OK, so release it since we're not using it
			pNuiSensor->Release();
		}

		IF_FAILED_THROW(hr = (m_pNuiSensor == NULL ? E_FAIL : S_OK));

		// Initialize the Kinect and specify that we'll be using color and audio
		IF_FAILED_THROW(hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_AUDIO));

		m_hNextKinectFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		IF_FAILED_THROW(hr = (m_hNextKinectFrameEvent == NULL ? E_POINTER : S_OK));

		// Open a color image stream to receive color frames
		IF_FAILED_THROW(hr = m_pNuiSensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, KINECT_IMAGE_RESOLUTION, 0, 2, m_hNextKinectFrameEvent, &m_hKinectStreamHandle));
	}
	catch(HRESULT){}

	if(SUCCEEDED(hr))
		LOG_HRESULT(hr = InitializeAudioSource());

	if(m_pNuiSensor != NULL && FAILED(hr)){

		ReleaseKinect();
	}

	return hr;
}

HRESULT CKinectDevice::InitializeAudioSource(){

	HRESULT hr = S_OK;
	INuiAudioBeam* pNuiAudioSource = NULL;
	IPropertyStore* pPropertyStore = NULL;
	PROPVARIANT pvSysMode;
	DMO_MEDIA_TYPE mt = {0};

	try{

		PropVariantInit(&pvSysMode);
		MoInitMediaType(&mt, sizeof(WAVEFORMATEX));

		IF_FAILED_THROW(hr = m_pNuiSensor->NuiGetAudioSource(&pNuiAudioSource));

		IF_FAILED_THROW(hr = pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**)&m_pAudioStream));

		IF_FAILED_THROW(hr = pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**)&pPropertyStore));

		//PropVariantInit(&pvSysMode);
		pvSysMode.vt = VT_I4;
		//   SINGLE_CHANNEL_AEC     = 0
					//   ADAPTIVE_ARRAY_ONLY    = 1
		//   OPTIBEAM_ARRAY_ONLY    = 2
					//   ADAPTIVE_ARRAY_AND_AEC = 3
		//   OPTIBEAM_ARRAY_AND_AEC = 4
		//   SINGLE_CHANNEL_NSAGC   = 5
					//   MODE_NOT_SET           = 6
		pvSysMode.lVal = (LONG)(5);
		pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);

		// Set the number of iterations on echo cancellations
		/*PROPVARIANT pvSysMode;
		PropVariantInit(&pvSysMode);
		pvSysMode.vt = VT_I4;
		// index is 0, 1 We need 0,2
		// AES_OFF = 0 ??
		// AES_ON = 1 ??
		pvSysMode.lVal = 0;
		pPropertyStore->SetValue(MFPKEY_WMAAECMA_FEATR_AES, pvSysMode);
		PropVariantClear(&pvSysMode);*/

		// Set DMO output format
					// AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample
		WAVEFORMATEX wfxOut = {WAVE_FORMAT_PCM, 1, 16000, 32000, 2, 16, 0};

		mt.majortype = MEDIATYPE_Audio;
		mt.subtype = MEDIASUBTYPE_PCM;
		mt.lSampleSize = 0;
		mt.bFixedSizeSamples = TRUE;
		mt.bTemporalCompression = FALSE;
		mt.formattype = FORMAT_WaveFormatEx;
		memcpy(mt.pbFormat, &wfxOut, sizeof(WAVEFORMATEX));

		IF_FAILED_THROW(hr = m_pAudioStream->SetOutputType(0, &mt, 0));
	}
	catch(HRESULT){}

	PropVariantClear(&pvSysMode);
	MoFreeMediaType(&mt);

	SAFE_RELEASE(pNuiAudioSource);
	SAFE_RELEASE(pPropertyStore);

	return hr;
}

HRESULT CKinectDevice::GetVideoKinectType(IMFMediaType** ppType){

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
		IF_FAILED_THROW(hr = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
		IF_FAILED_THROW(hr = pOutputType->SetUINT32(MF_MT_SAMPLE_SIZE, m_uiWidthInPixels * m_uiHeightInPixels * 4));

		IF_FAILED_THROW(hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, m_uiWidthInPixels, m_uiHeightInPixels));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, m_FrameRate.Numerator, m_FrameRate.Denominator));
		IF_FAILED_THROW(hr = MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 4, 3));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CKinectDevice::GetAudioKinectType(IMFMediaType** ppType){

	HRESULT hr = S_OK;
	IMFMediaType* pOutputType = NULL;

	try{

		IF_FAILED_THROW(hr = MFCreateMediaType(&pOutputType));

		WAVEFORMATEX WaveFormat;
		WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
		WaveFormat.nChannels = 1;
		WaveFormat.nSamplesPerSec = 16000;
		WaveFormat.nAvgBytesPerSec = 32000;
		WaveFormat.wBitsPerSample = 16;
		WaveFormat.nBlockAlign = 2;
		WaveFormat.cbSize = 0;

		IF_FAILED_THROW(hr = MFInitMediaTypeFromWaveFormatEx(pOutputType, &WaveFormat, sizeof(WaveFormat)));

		*ppType = pOutputType;
		(*ppType)->AddRef();
	}
	catch(HRESULT){}

	SAFE_RELEASE(pOutputType);
	return hr;
}

HRESULT CKinectDevice::GetNextVideoFrame(BYTE* pBuffer, DWORD dwSize, LONGLONG& duration){

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pNuiSensor == NULL ? E_POINTER : S_OK));

	//static UINT32 uiTest = 0;

	/*while(WaitForSingleObject(m_hNextKinectFrameEvent, 30) != WAIT_OBJECT_0);{

			TRACE((L"KinectGetNextFrame : WaitForSingleObject humm : %d", ++uiTest));
	}*/

	if(WaitForSingleObject(m_hNextKinectFrameEvent, 0) != WAIT_OBJECT_0){

		//TRACE((L"KinectGetNextFrame : WaitForSingleObject humm"));
		//return MF_E_INVALID_STREAM_STATE; -> Set Discontinuity
		return S_OK;
	}
	NUI_IMAGE_FRAME imageFrame;

	// Attempt to get the color frame
	IF_FAILED_RETURN(hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_hKinectStreamHandle, 0, &imageFrame));

	INuiFrameTexture* pTexture = imageFrame.pFrameTexture;
	NUI_LOCKED_RECT LockedRect;

	// Lock the frame data so the Kinect knows not to modify it while we're reading it
	pTexture->LockRect(0, &LockedRect, NULL, 0);

	// Make sure we've received valid data
	if(LockedRect.Pitch != 0){

		// Draw the data with Direct2D
		//m_pImageRenderer->Draw(static_cast<BYTE *>(LockedRect.pBits), LockedRect.size);
		assert((DWORD)LockedRect.size == dwSize);

		memcpy(pBuffer, LockedRect.pBits, dwSize);

		// We're done with the texture so unlock it
		pTexture->UnlockRect(0);

		duration = imageFrame.liTimeStamp.QuadPart;

		// Release the frame
		LOG_HRESULT(hr = m_pNuiSensor->NuiImageStreamReleaseFrame(m_hKinectStreamHandle, &imageFrame));
	}

	return hr;
}

HRESULT CKinectDevice::GetNextAudioFrame(BYTE* pBuffer, DWORD& dwSize){

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pNuiSensor == NULL ? E_POINTER : S_OK));

	ULONG cbProduced = 0;
	BYTE* pProduced = NULL;
	DWORD dwStatus = 0;
	DMO_OUTPUT_DATA_BUFFER outputBuffer = {0};
	outputBuffer.pBuffer = &m_AudioBuffer;
	DWORD dwCurSize = 0;
	BYTE* pCurBuffer = pBuffer;

	do{

		m_AudioBuffer.Init(0);
		outputBuffer.dwStatus = 0;

		LOG_HRESULT(hr = m_pAudioStream->ProcessOutput(0, 1, &outputBuffer, &dwStatus));

		// duration ?? ToDo
		//duration = 0;

		if(FAILED(hr)){
			break;
		}

		if(hr == S_FALSE){
			cbProduced = 0;
		}
		else{

			m_AudioBuffer.GetBufferAndLength(&pProduced, &cbProduced);

			dwCurSize += cbProduced;

			if(dwCurSize > dwSize){

				// For debug purpose.
				// TRACE((L"GetNextAudioFrame : dwCurSize > 32000"));
				break;
			}

			memcpy(pCurBuffer, pProduced, cbProduced);

			pCurBuffer += cbProduced;
		}
	} while(outputBuffer.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

	//TRACE((L"dwCurSize %d", dwCurSize));

	if(dwCurSize != 0)
		dwSize = dwCurSize;

	return hr;
}