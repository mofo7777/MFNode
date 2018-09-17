//----------------------------------------------------------------------------------------------
// Dxva2Decoder.h
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
#ifndef MFTDXVA2DEOCDER_H
#define MFTDXVA2DEOCDER_H

class CDxva2Decoder : BaseObject, public IMFTransform, public IMFAsyncCallback{

public:

	// Dxva2Decoder.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - Dxva2Decoder.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFAsyncCallback - Dxva2Decoder.cpp
	STDMETHODIMP GetParameters(DWORD*, DWORD*){ TRACE_TRANSFORM((L"MFTDxva2::GetParameters")); return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult*);

	// IMFTransform - Dxva2Decoder_Transform.cpp
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

	// Dxva2Decoder.cpp
	CDxva2Decoder();
	virtual ~CDxva2Decoder();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	IMFMediaType* m_pInputType;
	IMFMediaType* m_pOutputType;

	IDirect3DDeviceManager9* m_pDeviceManager;
	IDirectXVideoDecoderService* m_pDecoderService;
	IDirectXVideoDecoder* m_pVideoDecoder;
	IDirect3DSurface9* m_pSurface9[NUM_DXVA2_SURFACE];
	HANDLE m_hD3d9Device;
	DXVA2_ConfigPictureDecode* m_pConfigs;
	GUID m_gMpegVld;

	DXVA2_DecodeExecuteParams m_ExecuteParams;
	DXVA2_DecodeBufferDesc m_BufferDesc[4];
	DXVA_PictureParameters m_PictureParams;
	DXVA_SliceInfo m_SliceInfo[MAX_SLICE];
	DXVA_QmatrixData m_QuantaMatrix;
	DXVA2_VideoDesc m_Dxva2Desc;

	CMFBuffer m_InputBuffer;
	CMFBuffer m_SliceBuffer;
	unsigned int* m_pSliceOffset;

	VIDEO_PARAMS m_VideoParam;
	CTSScheduler m_cTSScheduler;
	DWORD m_dwTemporalReference;
	DWORD m_iCurSurfaceIndex;
	BOOL m_bSlice;
	BOOL m_bMpegHeaderEx;
	BOOL m_bIsMpeg1;
	BOOL m_bProgressive;
	BOOL m_bFirstPictureI;
	int m_iLastForward;
	int m_iLastPictureP;
	BOOL m_bFreeSurface[NUM_DXVA2_SURFACE];

	DWORD m_dwSampleSize;
	BOOL m_bHaveOuput;
	UINT64  m_rtAvgPerFrameInput;
	REFERENCE_TIME m_rtTime;
	UINT32  m_uiWidthInPixels;
	UINT32  m_uiHeightInPixels;
	MFRatio m_FrameRate;
	MFRatio m_PixelRatio;
	BOOL m_bIsDiscontinuity;
	BOOL m_bDraining;

	// Dxva2Decoder.cpp
	HRESULT BeginStreaming();
	HRESULT Flush();
	void ReleaseInput();
	void InitDxva2Buffers();

	// Dxva2Decoder_Dxva2.cpp
	HRESULT GetVideoDecoder(IDirect3DDeviceManager9*);
	HRESULT ConfigureDecoder();
	HRESULT SchedulePicture();
	HRESULT Dxva2DecodePicture();
	WORD GetScaleCode(const BYTE*);
	void InitPictureParams();
	void InitSliceParams();
	void InitQuantaMatrixParams();

	// Dxva2Decoder_Slice.cpp
	HRESULT Slice(BYTE*, const DWORD);
	HRESULT FindNextStartCode(BYTE*, const DWORD, DWORD*);
	HRESULT PictureCode(const BYTE*);
	HRESULT SequenceCode(const BYTE*);
	HRESULT SequenceExCode(const BYTE*);
	HRESULT SliceInit(const BYTE*);
	void InitQuanta();

	// Dxva2Decoder_Types.cpp
	HRESULT GetOutputType(IMFMediaType**);
	HRESULT OnCheckInputType(IMFMediaType*);
	HRESULT OnSetInputType(IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);

	// Dxva2Decoder_Trace.cpp
#ifdef TRACE_DXVA2
	void TraceConfigPictureDecode();
	void TracePictureParams();
	void TraceQuantaMatrix();
	void TraceSlices();
	void TraceCompBuffers();
#endif

	// Inline
	MFOffset MakeOffset(float);
};

inline MFOffset CDxva2Decoder::MakeOffset(float v){

	TRACE_TRANSFORM((L"MFTDxva2::MakeOffset"));

	MFOffset offset;
	offset.value = short(v);
	offset.fract = WORD(65536 * (v - offset.value));
	return offset;
}

#endif