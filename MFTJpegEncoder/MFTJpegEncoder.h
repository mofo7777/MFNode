//----------------------------------------------------------------------------------------------
// MFTJpegEncoder.h
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
#ifndef MFTJPEGENCODER_H
#define MFTJPEGENCODER_H

class CMFTJpegEncoder : BaseObject, public IMFTransform{

		public:

				// MFTJpegEncoder.cpp
				static HRESULT CreateInstance(IUnknown*, REFIID, void**);

				// IUnknown - MFTJpegEncoder.cpp
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

				// IMFTransform - MFTJpegEncoder_Transform.cpp
				STDMETHODIMP GetStreamLimits(DWORD*, DWORD*, DWORD*, DWORD*);
				STDMETHODIMP GetStreamCount(DWORD*, DWORD*);
				STDMETHODIMP GetStreamIDs(DWORD, DWORD*, DWORD, DWORD*);
    STDMETHODIMP GetInputStreamInfo(DWORD, MFT_INPUT_STREAM_INFO*);
    STDMETHODIMP GetOutputStreamInfo(DWORD, MFT_OUTPUT_STREAM_INFO*);
    STDMETHODIMP GetAttributes(IMFAttributes**);
    STDMETHODIMP GetInputStreamAttributes(DWORD, IMFAttributes**);
    STDMETHODIMP GetOutputStreamAttributes(DWORD, IMFAttributes**);
    STDMETHODIMP DeleteInputStream(DWORD);
    STDMETHODIMP AddInputStreams(DWORD, DWORD*);
    STDMETHODIMP GetInputAvailableType(DWORD, DWORD, IMFMediaType**);
    STDMETHODIMP GetOutputAvailableType(DWORD, DWORD, IMFMediaType**);
				STDMETHODIMP SetInputType(DWORD, IMFMediaType*, DWORD);
    STDMETHODIMP SetOutputType(DWORD, IMFMediaType*, DWORD);
    STDMETHODIMP GetInputCurrentType(DWORD, IMFMediaType**);
    STDMETHODIMP GetOutputCurrentType(DWORD, IMFMediaType**);
    STDMETHODIMP GetInputStatus(DWORD, DWORD*);
    STDMETHODIMP GetOutputStatus(DWORD*);
    STDMETHODIMP SetOutputBounds(LONGLONG, LONGLONG);
    STDMETHODIMP ProcessEvent(DWORD, IMFMediaEvent*);
    STDMETHODIMP ProcessMessage(MFT_MESSAGE_TYPE, ULONG_PTR);
    STDMETHODIMP ProcessInput(DWORD, IMFSample*, DWORD);
				STDMETHODIMP ProcessOutput(DWORD, DWORD, MFT_OUTPUT_DATA_BUFFER*, DWORD*);

		private:

				// MFTJpegEncoder.cpp
				CMFTJpegEncoder();
				virtual ~CMFTJpegEncoder();

    CriticSection m_CriticSection;
				volatile long m_nRefCount;

				IMFMediaType* m_pInputType;
    IMFMediaType* m_pOutputType;

				// Later if provide sample
				IMFSample* m_pSampleOut;
				REFERENCE_TIME m_rtFrame;
    UINT64 m_ui64AvgPerFrame;

				ULONG_PTR m_GdiToken;
				CJpegStream* m_cJpegStream;
				IStream* m_iJpegStream;
				Bitmap* m_pBmpImageFile;
				BOOL m_bHaveImage;

				// Between 0 and 100 (100 = best)
				int m_iJpegQuality;
				CLSID m_JpgClsid;

				// MFTJpegEncoder.cpp
				HRESULT BeginStreaming();
				void EndStreaming();
				HRESULT GetEncoderClsid(const WCHAR*, CLSID*);

				// MFTJpegEncoder_Types.cpp
				HRESULT GetOutputType(IMFMediaType**);
				HRESULT OnCheckInputType(IMFMediaType*);
				HRESULT OnCheckOutputType(IMFMediaType*);
};

#endif