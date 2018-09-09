//----------------------------------------------------------------------------------------------
// MFTMpegAudioDecoder.h
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
#ifndef CMFTMPEGAUDIODECODER_H
#define CMFTMPEGAUDIODECODER_H

class CMFTMpegAudioDecoder : BaseObject, public IMFTransform{

		public:

				// MFTMpegAudioDecoder.cpp
				static HRESULT CreateInstance(IUnknown*, REFIID, void**);

    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

				// IMFTransform - MFTMpegAudioDecoder_Transform.cpp
				STDMETHODIMP GetStreamLimits(DWORD*, DWORD*, DWORD*, DWORD*);
				STDMETHODIMP GetStreamCount(DWORD*, DWORD*);
				STDMETHODIMP GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*){ TRACE_TRANSFORM((L"Transform::GetStreamIDs")); return E_NOTIMPL; }
    STDMETHODIMP GetInputStreamInfo(DWORD, MFT_INPUT_STREAM_INFO*);
    STDMETHODIMP GetOutputStreamInfo(DWORD, MFT_OUTPUT_STREAM_INFO*);
    STDMETHODIMP GetAttributes(IMFAttributes**){ TRACE_TRANSFORM((L"Transform::GetAttributes")); return E_NOTIMPL; }
    STDMETHODIMP GetInputStreamAttributes(DWORD, IMFAttributes**){ TRACE_TRANSFORM((L"Transform::GetInputStreamAttributes")); return E_NOTIMPL; }
    STDMETHODIMP GetOutputStreamAttributes(DWORD, IMFAttributes**){ TRACE_TRANSFORM((L"Transform::GetOutputStreamAttributes")); return E_NOTIMPL; }
    STDMETHODIMP DeleteInputStream(DWORD){ TRACE_TRANSFORM((L"Transform::DeleteInputStream")); return E_NOTIMPL; }
    STDMETHODIMP AddInputStreams(DWORD, DWORD*){ TRACE_TRANSFORM((L"Transform::AddInputStreams")); return E_NOTIMPL; }
    STDMETHODIMP GetInputAvailableType(DWORD, DWORD, IMFMediaType**);
    STDMETHODIMP GetOutputAvailableType(DWORD, DWORD, IMFMediaType**);
				STDMETHODIMP SetInputType(DWORD, IMFMediaType*, DWORD);
    STDMETHODIMP SetOutputType(DWORD, IMFMediaType*, DWORD);
    STDMETHODIMP GetInputCurrentType(DWORD, IMFMediaType**);
    STDMETHODIMP GetOutputCurrentType(DWORD, IMFMediaType**);
    STDMETHODIMP GetInputStatus(DWORD, DWORD*);
    STDMETHODIMP GetOutputStatus(DWORD*);
    STDMETHODIMP SetOutputBounds(LONGLONG, LONGLONG){ TRACE_TRANSFORM((L"Transform::SetOutputBounds")); return E_NOTIMPL; }
    STDMETHODIMP ProcessEvent(DWORD, IMFMediaEvent*){ TRACE_TRANSFORM((L"Transform::ProcessEvent")); return E_NOTIMPL; }
    STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE, ULONG_PTR);
    STDMETHODIMP ProcessInput(DWORD, IMFSample*, DWORD);
				STDMETHODIMP ProcessOutput(DWORD, DWORD, MFT_OUTPUT_DATA_BUFFER*, DWORD*);

		private:

				// MFTMpegAudioDecoder.cpp
				CMFTMpegAudioDecoder();
				virtual ~CMFTMpegAudioDecoder();

				volatile long m_nRefCount;
    CriticSection m_CriticSection;

				CMpeg12Decoder m_cMpeg12Decoder;

    IMFMediaType* m_pInputType;
				IMFMediaType* m_pOutputType;

				IMFSample* m_pAudioSample;
				IMFMediaBuffer* m_pInputBuffer;
				IMFMediaBuffer* m_pLastInputBuffer;
				IMFMediaBuffer* m_pOutputBuffer;
				BOOL m_HasAudio;

				// 2304 is enough, 4608 is for float version
				short m_sAudioBuffer[MAX_AUDIO_BUFFER_SIZE];
				
				UINT32 m_uiAvgRate;
    REFERENCE_TIME m_rtTime;
				BOOL m_bValidTime;

				// MFTMpegAudioDecoder.cpp
				HRESULT AllocateStreamingResources();
				void FreeStreamingResources();
				void OnDiscontinuity();
				void OnFlush();

				// MFTMpegAudioDecoder_Transform.cpp
				HRESULT BufferKeepLastByte(const BYTE*, const DWORD);
				HRESULT AddOutputBuffer(IMFSample*, const UINT);
				HRESULT GetFullBuffer();
				HRESULT ConvertOutput(IMFSample*);

				// MFTMpegAudioDecoder_MediaType.cpp
				HRESULT GetInputType(IMFMediaType**);
				HRESULT GetOutputType(IMFMediaType**);
				HRESULT OnCheckInputType(IMFMediaType*);
				HRESULT OnCheckOutputType(IMFMediaType*);
				HRESULT ValidateMpegAudioType(IMFMediaType*);
				HRESULT ValidatePcmAudioType(IMFMediaType*);

				// Inline
				void ReleaseBufferSample();
};

inline void CMFTMpegAudioDecoder::ReleaseBufferSample(){

		if(m_pAudioSample == NULL)
				return;

		IMFMediaBuffer* pBuffer = NULL;
		DWORD dwBuffers = 0;

		HRESULT hr = m_pAudioSample->GetBufferCount(&dwBuffers);

		LOG_HRESULT(hr);

		//TRACE((L"Release Count = %d", dwBuffers));

		if(SUCCEEDED(hr)){
				
				for(DWORD i = 0; i < dwBuffers; i++){
						
						LOG_HRESULT(hr = m_pAudioSample->GetBufferByIndex(i, &pBuffer));
						SAFE_RELEASE(pBuffer);
				}
		}
}

#endif