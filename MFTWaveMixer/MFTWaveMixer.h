//----------------------------------------------------------------------------------------------
// MFTWaveMixer.h
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
#ifndef CMFTWAVEMIXER_H
#define CMFTWAVEMIXER_H

class CMFTWaveMixer : RefCountedObject, public IMFTransform{

public:

	// MFTWaveMixer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
	STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

	// IMFTransform - MFTWaveMixer_Transform.cpp
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

	// MFTWaveMixer.cpp
	CMFTWaveMixer();
	virtual ~CMFTWaveMixer();

	CriticSection     m_CriticSection;

	// Same for Input0/Output.
	IMFMediaType*     m_pAudioType;
	// Just to check input type 0 equal input type 1
	IMFMediaType*     m_pInput1AudioType;

	// Input samples : 0 for StreamId 0 and 1 for StreamId 1
	IMFSample* m_pSample0;
	IMFSample* m_pSample1;

	// Input buffers
	IMFMediaBuffer*   m_pInput0Buffer;
	BYTE*             m_pbInput0Data;
	DWORD             m_dwInput0Length;

	IMFMediaBuffer*   m_pInput1Buffer;
	BYTE*             m_pbInput1Data;
	DWORD             m_dwInput1Length;

	// Timestamps (first source will be used)
	REFERENCE_TIME    m_rtTimestamp;
	BOOL              m_bValidTime;

	BOOL              m_bInput0TypeSet;
	BOOL              m_bInput1TypeSet;
	BOOL              m_bOutputTypeSet;

	enum StreamDirection{ InputStream, OutputStream };

	// MFTWaveMixer.cpp
	HRESULT OnBeginStream();
	void OnFlushOrDrain(){ ReleaseInputBuffer(); m_rtTimestamp = 0; m_bValidTime = FALSE; }

	HRESULT GetMinimalType(IMFMediaType**);
	HRESULT OnCheckAudioType(IMFMediaType*, const DWORD);
	HRESULT OnSetMediaType(IMFMediaType*, StreamDirection, const DWORD);
	HRESULT ValidatePCMAudioType(IMFMediaType*);

	// MFTWaveMixer_Mixer.cpp
	void MixAudio(const BYTE*, const BYTE*, BYTE*, const DWORD);

	// Inline
	BOOL IsValidInputStream(DWORD dwInputStreamID) const{ return (dwInputStreamID == 0 || dwInputStreamID == 1); }

	UINT32 BlockAlign() const { return MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_BLOCK_ALIGNMENT, 0); }
	UINT32 NumChannels() const { return MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_NUM_CHANNELS, 0); }
	UINT32 BitsPerSample() const { return MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_BITS_PER_SAMPLE, 0); }
	UINT32 AvgBytesPerSec() const { return MFGetAttributeUINT32(m_pAudioType, MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 0); }

	void ReleaseInputBuffer();
};

inline void CMFTWaveMixer::ReleaseInputBuffer(){

	if(m_pbInput0Data){

		m_pInput0Buffer->Unlock();
		m_pbInput0Data = NULL;
		m_dwInput0Length = 0;
	}

	if(m_pbInput1Data){

		m_pInput1Buffer->Unlock();
		m_pbInput1Data = NULL;
		m_dwInput1Length = 0;
	}

	SAFE_RELEASE(m_pInput0Buffer);
	SAFE_RELEASE(m_pSample0);

	SAFE_RELEASE(m_pInput1Buffer);
	SAFE_RELEASE(m_pSample1);
}

#endif
