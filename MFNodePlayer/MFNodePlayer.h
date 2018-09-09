//----------------------------------------------------------------------------------------------
// MFNodePlayer.h
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
#ifndef MFNODEPLAYER_H
#define MFNODEPLAYER_H

class CMFNodePlayer : public IMFAsyncCallback{

public:

	CMFNodePlayer(const HWND);
	virtual ~CMFNodePlayer();

	// IUnknown - MFNodePlayer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFAsyncCallback - MFNodePlayer.cpp
	STDMETHODIMP GetParameters(DWORD*, DWORD*){ TRACE_PLAYER((L"Player::GetParameters")); return E_NOTIMPL; }
	STDMETHODIMP Invoke(IMFAsyncResult*);

	// MFNodePlayer.cpp
	HRESULT OpenWaveMixer(PCWSTR, PCWSTR);
	HRESULT OpenScreenShot();
	HRESULT OpenHttpStreamer();
	HRESULT OpenFlvFile(PCWSTR);
	HRESULT OpenFileRenderer(PCWSTR, const CUDA_DECODER);
	HRESULT OpenSequencer(PCWSTR, PCWSTR);
	HRESULT OpenAVCapture();
	HRESULT OpenFileDxva2(PCWSTR);
	HRESULT Play();
	HRESULT Pause();
	HRESULT Stop();
	HRESULT CloseSession();
	HRESULT RedrawVideo();
	HRESULT ResizeVideo(const WORD, const WORD);
	const SessionState GetState(){ TRACE_PLAYER((L"Player::GetState")); AutoLock lock(m_CriticSection); return m_state; }
	const CURRENT_SESSION GetSession(){ TRACE_PLAYER((L"Player::GetSession")); AutoLock lock(m_CriticSection); return m_CurrentSession; }
	const MFTIME GetDuration() const{ return m_llDuration; }

#ifdef TEST_CUDA_DECODER
	HRESULT OpenCudaRenderer(PCWSTR);
#elif defined TEST_DXVA2_DECODER
	HRESULT OpenDxva2Decoder(PCWSTR);
#endif

	// Inline
	void SetVideoReverseX(const BOOL);
	void SetVideoReverseY(const BOOL);
	void SetVideoNegatif(const BOOL);
	void SetVideoGray(const BOOL);
	void SetVideoGrayScale(const BOOL);
	void SetVideoColor(const SHADER_COLOR);
	void SetVideoContrast(const float);
	void SetVideoSaturation(const float);

	// Inline
	void GetBrightnessRange(BOOL*, INT*, INT*, INT*);
	void SetBrightness(const BOOL, const INT);
	void GetHueRange(BOOL*, INT*, INT*, INT*);
	void SetHue(const BOOL, const INT);
	void GetContrastRange(BOOL*, INT*, INT*, INT*);
	void SetContrast(const BOOL, const INT);
	void GetSaturationRange(BOOL*, INT*, INT*, INT*);
	void SetSaturation(const BOOL, const INT);
	void GetNoiseRange(BOOL*, INT*, INT*, INT*);
	void SetNoise(const BOOL, const INT);
	void GetEdgeRange(BOOL*, INT*, INT*, INT*);
	void SetEdge(const BOOL, const INT);
	void GetAnamorphicRange(BOOL*, INT*, INT*, INT*);
	void SetAnamorphic(const BOOL, const INT);

private:

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	HWND m_hWindow;

	IMFMediaSession* m_pSession;
	IMFSequencerSource* m_pSequencerSource;
	IMFMediaSource* m_pSource;
	IMFMediaSource* m_pSource2;
	IMFMediaSink* m_pSink;
	IMFVideoDisplayControl* m_pVideoDisplay;
	IMFVideoShaderEffect* m_pVideoShaderEffect;
	IMFDxva2RendererSettings* m_pDxva2RendererSettings;
	SessionState m_state;
	HANDLE m_closeCompleteEvent;
	MFTIME m_llDuration;

	CURRENT_SESSION m_CurrentSession;

	MFSequencerElementId m_dwSegmentId1;
	MFSequencerElementId m_dwSegmentId2;

#ifdef MF_TRACE_PLAYER_EVENT
	DWORD m_dwBegin;
	DWORD m_dwEnd;
#endif

	// MFNodePlayer.cpp
	HRESULT GetDescriptor(IMFMediaEvent*, IMFPresentationDescriptor**);

	// MFNodePlayer_Topology.cpp
	HRESULT CreateMediaSource(IMFMediaSource**, PCWSTR, const DWORD);
	HRESULT CreateScreenShotTopology(IMFTopology**);
	HRESULT CreateHttpTopology(IMFTopology**);
	HRESULT CreateTopology(IMFTopology**, IMFMediaSource*);
	HRESULT CreateRendererTopology(IMFTopology**, IMFMediaSource*, const CLSID, const CLSID);
	HRESULT BuildScreenShotTopology(IMFTopology*, IMFPresentationDescriptor*, DWORD);
	HRESULT BuildHttpTopology(IMFTopology*, IMFPresentationDescriptor*, DWORD);
	HRESULT BuildTopology(IMFTopology*, IMFPresentationDescriptor*, IMFMediaSource*);
	HRESULT BuildRendererTopology(IMFTopology*, IMFMediaSource*, IMFPresentationDescriptor*, const CLSID, const CLSID);
	HRESULT CreateSourceStreamNode(IMFMediaSource*, IMFPresentationDescriptor*, IMFStreamDescriptor*, IMFTopologyNode**);
	HRESULT CreateTransformNode(const CLSID&, IMFTopologyNode**);
	HRESULT CreateClsidOutputNode(const CLSID&, IMFTopologyNode**);
	HRESULT CreateVideoOutputNode(IMFStreamDescriptor*, IMFTopologyNode**);
	HRESULT CreateOutputNode(IMFStreamDescriptor*, IMFTopologyNode**);

#ifdef TEST_CUDA_DECODER
	HRESULT CreateCudaTopology(IMFTopology**, IMFMediaSource*);
	HRESULT BuildCudaTopology(IMFTopology*, IMFPresentationDescriptor*, IMFMediaSource*);
#elif defined TEST_DXVA2_DECODER
	HRESULT CreateDxva2Topology(IMFTopology**, IMFMediaSource*);
	HRESULT BuildDxva2Topology(IMFTopology*, IMFPresentationDescriptor*, IMFMediaSource*);
	HRESULT ConnectD3DManager(IMFTopologyNode*, IMFTopologyNode*);
#endif

	// MFNodePlayer_Capture.cpp
	HRESULT CreateCaptureSource(IMFMediaSource**);
	HRESULT CreateAVCaptureSource(IMFMediaSource**, const GUID&, const wstring&, const UINT32);
	HRESULT SelectMediaType(IMFMediaSource*, const BOOL);

	// Inline
	const BOOL IsVideoSession() const;
};

inline const BOOL CMFNodePlayer::IsVideoSession() const{

	return m_CurrentSession == SESSION_SCREENSHOT ||
		m_CurrentSession == SESSION_SEQUENCER ||
		m_CurrentSession == SESSION_FLV ||
		m_CurrentSession == SESSION_AVCAPTURE;
}

// Shader functions : we could refactor to just call one function with a second parameter...
inline void CMFNodePlayer::SetVideoReverseX(const BOOL bReverse){

	TRACE_PLAYER((L"Player::SetVideoReverseX"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderReverseX(bReverse));
}

inline void CMFNodePlayer::SetVideoReverseY(const BOOL bReverse){

	TRACE_PLAYER((L"Player::SetVideoReverseY"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderReverseY(bReverse));
}

inline void CMFNodePlayer::SetVideoNegatif(const BOOL bNegatif){

	TRACE_PLAYER((L"Player::SetVideoNegatif"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderNegatif(bNegatif));
}

inline void CMFNodePlayer::SetVideoGray(const BOOL bGray){

	TRACE_PLAYER((L"Player::SetVideoGray"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderGray(bGray));
}

inline void CMFNodePlayer::SetVideoGrayScale(const BOOL bGray){

	TRACE_PLAYER((L"Player::SetVideoGrayScale"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderGrayScale(bGray));
}

inline void CMFNodePlayer::SetVideoColor(const SHADER_COLOR color){

	TRACE_PLAYER((L"Player::SetVideoNegatif"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderColor(color));
}

inline void CMFNodePlayer::SetVideoContrast(const float fContrast){

	TRACE_PLAYER((L"Player::SetVideoContrast"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderContrast(fContrast));
}

inline void CMFNodePlayer::SetVideoSaturation(const float fSaturation){

	TRACE_PLAYER((L"Player::SetVideoSaturation"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pVideoShaderEffect == NULL)
		return;

	LOG_HRESULT(m_pVideoShaderEffect->SetShaderSaturation(fSaturation));
}

inline void CMFNodePlayer::GetBrightnessRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetBrightnessRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetBrightness(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetBrightness(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetBrightness"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetBrightness(bEnable, iVal));
}

inline void CMFNodePlayer::GetHueRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetHueRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetHue(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetHue(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetHue"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetHue(bEnable, iVal));
}

inline void CMFNodePlayer::GetContrastRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetContrastRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetContrast(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetContrast(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetContrast"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetContrast(bEnable, iVal));
}

inline void CMFNodePlayer::GetSaturationRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetSaturationRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetSaturation(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetSaturation(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetSaturation"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetSaturation(bEnable, iVal));
}

inline void CMFNodePlayer::GetNoiseRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetNoiseRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetNoiseReduction(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetNoise(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetNoise"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetNoiseReduction(bEnable, iVal));
}

inline void CMFNodePlayer::GetEdgeRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetEdgeRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetEdgeEnhancement(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetEdge(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetEdge"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetEdgeEnhancement(bEnable, iVal));
}

inline void CMFNodePlayer::GetAnamorphicRange(BOOL* pSupported, INT* pVal, INT* pMin, INT* pMax){

	TRACE_PLAYER((L"Player::GetAnamorphicRange"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->GetAnamorphicScaling(pSupported, pVal, pMin, pMax));
}

inline void CMFNodePlayer::SetAnamorphic(const BOOL bEnable, const INT iVal){

	TRACE_PLAYER((L"Player::SetAnamorphic"));

	AutoLock lock(m_CriticSection);

	if(m_state == SessionClosing || m_pDxva2RendererSettings == NULL)
		return;

	LOG_HRESULT(m_pDxva2RendererSettings->SetAnamorphicScaling(bEnable, iVal));
}

#endif