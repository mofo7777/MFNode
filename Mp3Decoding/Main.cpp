//----------------------------------------------------------------------------------------------
// Main.cpp
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
#include "Stdafx.h"

// Change this !!!
#define MP3_INPUT_FILE     L"C:\\projet\\media\\Wav\\Sound.mp3"
#define WAVE_OUTPUT_FILE   L"C:\\projet\\media\\Wav\\SoundFromMp3.wav"

void main(){

		TRACE_INIT();

		HRESULT hr = S_OK;

	 IMFTransform* pDecoder = NULL;
	 IMFMediaType* pInputType = NULL;
	 IMFMediaType* pOutputType = NULL;
	 IMFSourceReader* pReader = NULL;
		IMFSample* pOutputSample = NULL;
		IMFMediaBuffer* pOutputBuffer = NULL;

		MFT_OUTPUT_STREAM_INFO OutputStreamInfo;
		MFT_OUTPUT_DATA_BUFFER OutputDataBuffer[1];

		WAVEFORM waveData;
		GUID guid;

		try{

				IF_FAILED_THROW(hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED));
	   IF_FAILED_THROW(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE));
	   IF_FAILED_THROW(hr = CoCreateInstance(CLSID_CMP3DecMediaObject, 0, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pDecoder)));
		  
	   IF_FAILED_THROW(hr = MFCreateSourceReaderFromURL(MP3_INPUT_FILE, 0, & pReader));
		  IF_FAILED_THROW(hr = pReader->GetNativeMediaType(0, 0, &pInputType));

				//LogMediaType(pInputType);

				IF_FAILED_THROW(hr = pInputType->GetGUID(MF_MT_SUBTYPE, &guid));

				IF_FAILED_THROW(hr = (guid != MFAudioFormat_MP3 ? E_UNEXPECTED : S_OK));

				IF_FAILED_THROW(hr = pReader->SetCurrentMediaType(0, NULL, pInputType));
		  
		  IF_FAILED_THROW(hr = pDecoder->SetInputType(0, pInputType, 0));
				SAFE_RELEASE(pInputType);

				DWORD dwIndex = 0;
				UINT32 uiTmp;

				while((hr = pDecoder->GetOutputAvailableType(0, dwIndex, &pOutputType)) == S_OK){

						//LogMediaType(pOutputType);

				  IF_FAILED_THROW(hr = pOutputType->GetGUID(MF_MT_SUBTYPE, &guid));
						IF_FAILED_THROW(hr = pOutputType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uiTmp));

						if(guid == MFAudioFormat_PCM && uiTmp == 2){

								waveData.nChannels = (UINT16)uiTmp;

				    IF_FAILED_THROW(hr = pOutputType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &uiTmp));
								waveData.nSamplesPerSec = uiTmp;

				    IF_FAILED_THROW(hr = pOutputType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, &uiTmp));
								waveData.nAvgBytesPerSec = uiTmp;

				    IF_FAILED_THROW(hr = pOutputType->GetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, &uiTmp));
								waveData.nBlockAlign = (UINT16)uiTmp;

				    IF_FAILED_THROW(hr = pOutputType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &uiTmp));
								waveData.wBitsPerSample = (UINT16)uiTmp;

								SAFE_RELEASE(pOutputType);
								break;
						}

						SAFE_RELEASE(pOutputType);
						dwIndex++;
				}

				IF_FAILED_THROW(hr = pDecoder->GetOutputAvailableType(0, 1, &pOutputType));

		  IF_FAILED_THROW(hr = pDecoder->SetOutputType(0, pOutputType, 0));
				SAFE_RELEASE(pOutputType);

				//IF_FAILED_THROW(hr = pDecoder->GetOutputCurrentType(0, &pOutputType));
				// LogMediaType(pOutputType);
				//SAFE_RELEASE(pOutputType);

		  IF_FAILED_THROW(hr = pDecoder->GetOutputStreamInfo(0, &OutputStreamInfo));
		  IF_FAILED_THROW(hr = MFCreateSample(&pOutputSample));
		  IF_FAILED_THROW(hr = MFCreateMemoryBuffer(OutputStreamInfo.cbSize, &pOutputBuffer));
		  IF_FAILED_THROW(hr = pOutputBuffer->SetCurrentLength(0));
		  IF_FAILED_THROW(hr = pOutputSample->AddBuffer(pOutputBuffer));
				OutputDataBuffer[0].dwStreamID = 0;
		  OutputDataBuffer[0].pSample = pOutputSample;
		  
	   IF_FAILED_THROW(hr = pDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
		}
		catch(HRESULT){}

		// I know a simple goto...
		if(FAILED(hr)){

				SAFE_RELEASE(pOutputBuffer);
				SAFE_RELEASE(pOutputSample);
		  SAFE_RELEASE(pInputType);
		  SAFE_RELEASE(pOutputType);
		  SAFE_RELEASE(pDecoder);
		  SAFE_RELEASE(pReader);

				LOG_HRESULT(MFShutdown());
				CoUninitialize();

				TRACE_CLOSE();

				return;
		}

		CMFWaveWriter cWaveWriter;

		BOOL bWaveInit = cWaveWriter.Initialize(WAVE_OUTPUT_FILE);

		// I know again a simple goto...
		if(bWaveInit == FALSE){

				SAFE_RELEASE(pOutputBuffer);
				SAFE_RELEASE(pOutputSample);
		  SAFE_RELEASE(pInputType);
		  SAFE_RELEASE(pOutputType);
		  SAFE_RELEASE(pDecoder);
		  SAFE_RELEASE(pReader);

				LOG_HRESULT(MFShutdown());
				CoUninitialize();

				TRACE_CLOSE();

				return;
		}

  IMFSample* pInputSample = NULL;

		DWORD dwStreamIndex;
		LONGLONG llTimeStamp;

		IMFMediaBuffer* pResultBuffer = NULL;
		BYTE* pAudioData = NULL;
		DWORD dwBufferSize;
  
		DWORD dwStatus;
  DWORD dwLength;
		UINT32 uiFileLength = 0;
		DWORD dwFlags;

		// Here errors should be better checked.
	 do{
		
				LOG_HRESULT(hr = pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &dwStreamIndex, &dwFlags, &llTimeStamp, &pInputSample));
		
				if(dwFlags != 0){

						LOG_HRESULT(hr = pDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0));
						LOG_HRESULT(hr = pDecoder->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN , 0));
						// We should check ProcessOutput... perhaps there is audio frame.
			   break;
				}

		  LOG_HRESULT(hr = pDecoder->ProcessInput(0, pInputSample, 0));
				
				// CLSID_CMP3DecMediaObject does not implement this...
				/*if(SUCCEEDED(hr = pDecoder->GetOutputStatus(&dwFlags))){

						if(dwFlags != MFT_OUTPUT_STATUS_SAMPLE_READY)
								continue;
				}*/
		  
		  OutputDataBuffer[0].dwStatus   = 0;
		  OutputDataBuffer[0].pEvents    = 0;
		  
		  dwStatus = 0;

		  do{

						hr = pDecoder->ProcessOutput(0, 1, OutputDataBuffer, &dwStatus);

						assert(hr != MF_E_TRANSFORM_STREAM_CHANGE);

						// Mp3 decoder seems not to tell us there is more data for output...
				  assert(OutputDataBuffer[0].dwStatus != MFT_OUTPUT_DATA_BUFFER_INCOMPLETE);

				  if(hr == S_OK){

						  LOG_HRESULT(hr = pOutputBuffer->GetCurrentLength(&dwLength));

		      LOG_HRESULT(hr = pOutputSample->ConvertToContiguousBuffer(&pResultBuffer));
		      LOG_HRESULT(hr = pResultBuffer->Lock(&pAudioData, NULL, &dwBufferSize));

						  bWaveInit = cWaveWriter.WriteWaveData(pAudioData, dwLength);
								assert(bWaveInit);

						  uiFileLength += dwLength;

		      LOG_HRESULT(hr = pResultBuffer->Unlock());
						}

						LOG_HRESULT(pOutputBuffer->SetCurrentLength(0));

						SAFE_RELEASE(pResultBuffer);
				}
				while(hr == S_OK);

				LOG_HRESULT(pOutputBuffer->SetCurrentLength(0));

				SAFE_RELEASE(pResultBuffer);
				SAFE_RELEASE(pInputSample);
	 }
	 while (dwFlags == 0);

		// Todo check hr fatal error...
		bWaveInit = cWaveWriter.FinalizeHeader(waveData, uiFileLength);

		assert(bWaveInit);
		
		SAFE_RELEASE(pResultBuffer);
		SAFE_RELEASE(pInputSample);
		
		SAFE_RELEASE(pOutputBuffer);
		SAFE_RELEASE(pOutputSample);
		SAFE_RELEASE(pInputType);
		SAFE_RELEASE(pOutputType);
		SAFE_RELEASE(pDecoder);
		SAFE_RELEASE(pReader);

  LOG_HRESULT(MFShutdown());
  CoUninitialize();

		TRACE_CLOSE();
}