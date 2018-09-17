//----------------------------------------------------------------------------------------------
// Vp6Decoder.h
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
#ifndef VP6DECODER_H
#define VP6DECODER_H

class CVp6Decoder : BaseObject, public IMFTransform{

public:

	// Vp6Decoder.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// Vp6Decoder_Transform.cpp
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

	// Vp6Decoder.cpp
	CVp6Decoder();
	virtual ~CVp6Decoder();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	VP62 m_pDecoder;

	IMFMediaType* m_pInputType;
	IMFMediaType* m_pOutputType;

	IMFSample* m_pInputSample;

	DWORD   m_dwSampleSize;
	UINT32  m_uiWidthInPixels;
	UINT32  m_uiHeightInPixels;

	BOOL m_bDraining;

	// Vp6Decoder.cpp
	void OnFlush();

	// Vp6Decoder_Types.cpp
	HRESULT GetOutputType(IMFMediaType**);
	HRESULT OnCheckInputType(IMFMediaType*);
	HRESULT OnSetInputType(IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);

	// Vp6Decoder_Mmx.cpp
	void Vp6CopyMmx(void*, const void*, size_t);
	void Vp6CopyFrameYV12Stride(const unsigned int, const UINT32, const UINT32, BYTE*, const BYTE*, const BYTE*, const BYTE*);
};

#endif