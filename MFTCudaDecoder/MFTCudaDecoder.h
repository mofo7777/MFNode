//----------------------------------------------------------------------------------------------
// MFTCudaDecoder.h
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
#ifndef MFTCUDADEOCDER_H
#define MFTCUDADEOCDER_H

class CMFTCudaDecoder : public IMFTransform{

public:

	// MFTCudaDecoder.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - MFTCudaDecoder.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFTransform - MFTCudaDecoder_Transform.cpp
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

	// MFTCudaDecoder.cpp
	CMFTCudaDecoder();
	virtual ~CMFTCudaDecoder();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	IMFMediaType* m_pInputType;
	IMFMediaType* m_pOutputType;

	queue<IMFSample*> m_qSampleOut;

	CCudaManager m_cCudaManager;
	CCudaDecoder m_cCudaDecoder;
	CCudaFrame m_cCudaFrame;
	CMFBuffer m_InputBuffer;
	CMFBuffer m_SliceBuffer;

	CUVIDPICPARAMS m_VideoParam;
	DWORD m_dwTemporalReference;
	DWORD m_dwCudaFrameSize;

	DWORD m_dwSampleSize;
	UINT32 m_uiFrameWidth;
	UINT32 m_uiFrameHeight;
	MFRatio m_FrameRate;
	MFRatio m_Pixel;
	UINT64 m_rtAvgPerFrame;
	REFERENCE_TIME m_rtTime;

	unsigned int* m_pSliceOffset;
	int m_iCurIDX;
	int m_iLastPictureP;
	int m_iLastForward;

	BOOL m_bSlice;
	BOOL m_bMpeg1;
	BOOL m_bProgressive;
	BOOL m_bMpegHeaderEx;
	BOOL m_bNV12;
	BOOL m_bDraining;
	BOOL m_bIsDiscontinuity;
	BOOL m_bFirstPictureI;

	// MFTCudaVideo.cpp
	HRESULT BeginStreaming();
	void EndStreaming();
	HRESULT OnFlush();
	HRESULT OnDrain();
	void InitQuanta();

	// MFTCudaVideo_Types.cpp
	HRESULT GetInputType(IMFMediaType**){ return E_NOTIMPL; }
	HRESULT GetOutputType(IMFMediaType**, const DWORD);
	HRESULT OnCheckInputType(IMFMediaType*);
	HRESULT OnSetInputType(IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);

	// MFTCudaVideo_Process.cpp
	HRESULT Slice(BYTE*, const DWORD);
	HRESULT FindNextStartCode(BYTE*, const DWORD, DWORD*);
	HRESULT PictureCode(const BYTE*);
	HRESULT SequenceCode(const BYTE*);
	HRESULT SequenceExCode(const BYTE*);
	HRESULT SliceInit(const BYTE*);
	HRESULT CudaDecodePicture();
	HRESULT DecodePicture(const SCUDAFRAME*);

	// inline
	BOOL HaveSampleOut(){ return m_qSampleOut.empty() ? FALSE : TRUE; }

	// Test
#ifdef TRACE_PICTURE_PARAM
	void TracePictureParam();
#endif

#ifdef TRACE_SLICE
	void TraceSlice();
#endif

#ifdef TRACE_PICTURE_OUTPUT
	void TracePicture(const int iCurIDX, const int iPictureType, const DWORD dwTemporalReference, const REFERENCE_TIME);
#endif
};

#endif
