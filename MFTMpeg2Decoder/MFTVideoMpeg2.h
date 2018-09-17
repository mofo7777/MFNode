//----------------------------------------------------------------------------------------------
// MFTVideoMpeg2.h
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
#ifndef MFTVIDEOMPEG2_H
#define MFTVIDEOMPEG2_H

class CMFTVideoMpeg2 : BaseObject, RefCountedObject, public IMFTransform{

public:

	// MFTVideoMpeg2.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
	STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

	// IMFTransform - MFTVideoMpeg2_Transform.cpp
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

	// MFTVideoMpeg2.cpp
	CMFTVideoMpeg2();
	virtual ~CMFTVideoMpeg2();

	CriticSection     m_CriticSection;

	CMpeg2Decoder*    m_pMpeg2Decoder;

	IMFMediaType*     m_pInputType;
	IMFMediaType*     m_pOutputType;

	IMFMediaBuffer*   m_pInputBuffer;
	//IMFMediaBuffer*   m_pOuputLastBuffer;
	BYTE*             m_pbInputData;
	DWORD             m_dwInputLength;

	IMFSample*        m_pSampleOut;
	//BOOL              m_bDiscontinuity;
	//BOOL              m_bSetDiscontinuity;

	// For Test
	DWORD m_dwNumVideoFrame;

	//  Timestamps
	REFERENCE_TIME    m_rtFrame;
	//REFERENCE_TIME    m_rtFrameTest;
	REFERENCE_TIME    m_rtAvgPerFrame;
	UINT64            m_rtAvgPerFrameInput;

	// Fomat information
	UINT32            m_uiWidthInPixels2;
	UINT32            m_uiHeightInPixels2;
	MFRatio           m_FrameRate;
	DWORD             m_dwSampleSize2;

	// MFTVideoMpeg2.cpp
	HRESULT AllocateStreamingResources();
	void FreeStreamingResources();
	HRESULT GetOutputType(IMFMediaType**);
	HRESULT OnCheckInputType(IMFMediaType*);
	HRESULT OnSetInputType(IMFMediaType*);
	HRESULT OnCheckOutputType(IMFMediaType*);
	HRESULT OnSetOutType(IMFMediaType*);
	HRESULT OnDrain();
	HRESULT OnFlush();

	// MFTVideoMpeg2_Process.cpp
	HRESULT Mpeg2ProcessInput();
	HRESULT Mpeg2SetImage();
	HRESULT Mpeg2ProcessOutput(IMFSample*, DWORD*);

	// MFTVideoMpeg2_Mmx.cpp
	void Mpeg2CopyMmx(void*, const void*, size_t);
	void Mpeg2CopyFrameYV12(const UINT32, const UINT32, BYTE*, const BYTE*, const BYTE*, const BYTE*);
	void Mpeg2CopyFrameYV12Stride(const unsigned int, const UINT32, const UINT32, BYTE*, const BYTE*, const BYTE*, const BYTE*);
	void Mpeg2CopyFrameI420(const UINT32, const UINT32, BYTE*, const BYTE*, const BYTE*, const BYTE*);

	// inline
	void ReleaseInputBuffer();
	void ResetOutSample();
	BOOL HaveSampleOut();
};

inline void CMFTVideoMpeg2::ReleaseInputBuffer(){

	if(m_pInputBuffer && m_pbInputData)
		m_pInputBuffer->Unlock();

	SAFE_RELEASE(m_pInputBuffer);
	m_pbInputData = NULL;
	m_dwInputLength = 0;
}

inline void CMFTVideoMpeg2::ResetOutSample(){

	if(m_pSampleOut == NULL)
		return;

	HRESULT hr;
	IMFMediaBuffer* pBuffer = NULL;
	DWORD cBuffers = 0;

	hr = m_pSampleOut->GetBufferCount(&cBuffers);

	if(SUCCEEDED(hr)){

		for(DWORD i = 0; i < cBuffers; i++){

			hr = m_pSampleOut->GetBufferByIndex(i, &pBuffer);

			SAFE_RELEASE(pBuffer);
			assert(hr == S_OK);
		}

		hr = m_pSampleOut->RemoveAllBuffers();
		assert(hr == S_OK);
	}
}

inline BOOL CMFTVideoMpeg2::HaveSampleOut(){

	if(m_pSampleOut){

		DWORD dwNumBuf = 0;

		if(SUCCEEDED(m_pSampleOut->GetBufferCount(&dwNumBuf))){

			if(dwNumBuf > 0)
				return TRUE;
		}
	}
	return FALSE;
}

#endif