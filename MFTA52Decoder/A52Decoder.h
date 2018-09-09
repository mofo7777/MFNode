//----------------------------------------------------------------------------------------------
// A52Decoder.h
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
#ifndef A52DECODER_H
#define A52DECODER_H

class CA52Decoder : BaseObject, public IMFTransform{

public:

	// A52Decoder.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// A52Decoder_Transform.cpp
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

	// A52Decoder.cpp
	CA52Decoder();
	virtual ~CA52Decoder();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;

	IMFMediaType* m_pInputType;
	IMFMediaType* m_pOutputType;

	IMFSample* m_pInputSample;

	a52_state_t* m_A52State;

	BOOL m_bDraining;

	UINT32 m_uiChannels;
	UINT32 m_uiSamplePerSec;
	REFERENCE_TIME m_rtSampleTime;
	BOOL m_bValidTime;

	// A52Decoder.cpp
	void OnFlush();
	HRESULT OnStartStreaming();

	// A52Decoder_Types.cpp
	HRESULT GetOutputType(IMFMediaType**);
	HRESULT OnCheckInputType(IMFMediaType*);
	HRESULT OnSetInputType(IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);

	// A52Decoder_Transform.cpp
	HRESULT DecodeA52(IMFSample*, BYTE*, const DWORD);
	HRESULT CA52Decoder::AddOutputBuffer(IMFSample*, const FLOAT);
	HRESULT ConvertOutput(IMFSample*);

	// Inline
	UINT16 GetAC52Channels(const BYTE);
};

inline UINT16 CA52Decoder::GetAC52Channels(const BYTE bAcmode){

	UINT16 uiChannels;

	switch(bAcmode){

	case 0x00: uiChannels = 2; break;
	case 0x01: uiChannels = 1; break;
	case 0x02: uiChannels = 2; break;
	case 0x03: uiChannels = 3; break;
	case 0x04: uiChannels = 3; break;
	case 0x05: uiChannels = 4; break;
	case 0x06: uiChannels = 4; break;
	case 0x07: uiChannels = 5; break;
	default: uiChannels = 0;
	}

	return uiChannels;
}

#endif