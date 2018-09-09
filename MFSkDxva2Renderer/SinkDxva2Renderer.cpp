//----------------------------------------------------------------------------------------------
// SinkDxva2Renderer.cpp
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
#include "StdAfx.h"

CSinkDxva2Renderer::CSinkDxva2Renderer(HRESULT& hr)
	: m_nRefCount(1),
	m_bShutdown(FALSE),
	m_pStreamDxva2Renderer(NULL),
	m_pClock(NULL),
	m_bPreroll(FALSE),
	m_PerFrame_1_4th(0),
	m_hRendererThread(NULL),
	m_hThreadReadyEvent(NULL),
	m_dwThreadID(0)
{
	TRACE_SINK((L"SinkRenderer::CTOR"));

	CStreamDxva2Renderer* pStream = NULL;
	hr = CStreamDxva2Renderer::CreateInstance(this, &pStream, hr);

	if(SUCCEEDED(hr)){

		m_pStreamDxva2Renderer = pStream;
		m_pStreamDxva2Renderer->AddRef();
	}
	else{
		m_bShutdown = TRUE;
	}

	SAFE_RELEASE(pStream);
}

CSinkDxva2Renderer::~CSinkDxva2Renderer(){

	TRACE_SINK((L"SinkRenderer::DTOR"));
	Shutdown();
}

HRESULT CSinkDxva2Renderer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_SINK((L"SinkRenderer::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CSinkDxva2Renderer* pMFSk = new (std::nothrow)CSinkDxva2Renderer(hr);

	IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
	IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

	LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFSk);

	return hr;
}

HRESULT CSinkDxva2Renderer::QueryInterface(REFIID riid, void** ppv){

	TRACE_SINK((L"SinkRenderer::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CSinkDxva2Renderer, IMFMediaSink),
			QITABENT(CSinkDxva2Renderer, IMFClockStateSink),
			QITABENT(CSinkDxva2Renderer, IMFDxva2RendererSettings),
			QITABENT(CSinkDxva2Renderer, IMFMediaSinkPreroll),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

ULONG CSinkDxva2Renderer::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CSinkDxva2Renderer::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"SinkRenderer::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CSinkDxva2Renderer::NotifyPreroll(MFTIME){

	TRACE_SINK((L"SinkRenderer::NotifyPreroll"));

	AutoLock lock(m_CriticSection);

	HRESULT hr;
	IF_FAILED_RETURN(hr = CheckShutdown());

	m_bPreroll = TRUE;

	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->RequestSample());
	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->RequestSample());
	IF_FAILED_RETURN(hr = m_pStreamDxva2Renderer->RequestSample());

	return hr;
}

HRESULT CSinkDxva2Renderer::ProcessSample(IMFSample* pSample){

	TRACE_SINK((L"SinkRenderer::ProcessSample"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER : S_OK));

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	m_cMFSafeQueue.PushSample(pSample);

	if(m_bPreroll  && m_cMFSafeQueue.GetSize() >= 3){

		LOG_HRESULT(hr = m_pStreamDxva2Renderer->Preroll());
		m_bPreroll = FALSE;
	}

	return hr;
}

HRESULT CSinkDxva2Renderer::GetVideoAttribute(UINT32* uiWidth, UINT32* uiHeight, UINT32* uiStride, D3DFORMAT* VideoFmt){

	TRACE_SINK((L"SinkRenderer::GetVideoAttribute"));

	HRESULT hr = S_OK;
	IMFMediaType* pMediaType = NULL;
	MFRatio fps;
	UINT64 AvgTimePerFrame = 0;
	GUID SubType;

	try{

		// D3DFMT_NV12

		IF_FAILED_THROW(hr = m_pStreamDxva2Renderer->GetCurrentMediaType(&pMediaType));
		IF_FAILED_RETURN(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &SubType));
		IF_FAILED_THROW(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, uiWidth, uiHeight));
		IF_FAILED_THROW(hr = pMediaType->GetUINT32(MF_MT_DEFAULT_STRIDE, uiStride));
		IF_FAILED_THROW(hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator));
		IF_FAILED_THROW(hr = MFFrameRateToAverageTimePerFrame(fps.Numerator, fps.Denominator, &AvgTimePerFrame));

		IF_FAILED_THROW(hr = ((SubType != MFVideoFormat_YV12 && SubType != MFVideoFormat_NV12) ? MF_E_INVALIDTYPE : S_OK));

		*VideoFmt = (SubType == MFVideoFormat_YV12 ? (D3DFORMAT)D3DFMT_YV12 : (D3DFORMAT)D3DFMT_NV12);
		m_PerFrame_1_4th = AvgTimePerFrame / 4;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pMediaType);

	return hr;
}