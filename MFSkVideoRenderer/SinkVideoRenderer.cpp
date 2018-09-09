//----------------------------------------------------------------------------------------------
// SinkVideoRenderer.cpp
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

CSinkVideoRenderer::CSinkVideoRenderer(HRESULT& hr)
		: m_nRefCount(1),
		  m_bShutdown(FALSE),
		  m_pStreamVideoRenderer(NULL),
				m_pClock(NULL),
				m_bPreroll(FALSE),
				m_PerFrame_1_4th(0),
    m_hRendererThread(NULL),
    m_hThreadReadyEvent(NULL),
				m_dwThreadID(0)
{
		TRACE_SINK((L"SinkRenderer::CTOR"));

		CStreamVideoRenderer* pStream = NULL;
		hr = CStreamVideoRenderer::CreateInstance(this, &pStream, hr);

		if(SUCCEEDED(hr)){

				m_pStreamVideoRenderer = pStream;
				m_pStreamVideoRenderer->AddRef();
		}
		else{
				m_bShutdown = TRUE;
		}

		SAFE_RELEASE(pStream);
}

CSinkVideoRenderer::~CSinkVideoRenderer(){
		
		TRACE_SINK((L"SinkRenderer::DTOR"));
		Shutdown();
}

HRESULT CSinkVideoRenderer::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		TRACE_SINK((L"SinkRenderer::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CSinkVideoRenderer* pMFSk = new (std::nothrow)CSinkVideoRenderer(hr);

		IF_FAILED_RETURN(pMFSk == NULL ? E_OUTOFMEMORY : S_OK);
		IF_FAILED_RETURN(FAILED(hr) ? hr : S_OK);

		LOG_HRESULT(hr = pMFSk->QueryInterface(iid, ppv));

		SAFE_RELEASE(pMFSk);

		return hr;
}

HRESULT CSinkVideoRenderer::QueryInterface(REFIID riid, void** ppv){

		TRACE_SINK((L"SinkRenderer::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CSinkVideoRenderer, IMFMediaSink),
				QITABENT(CSinkVideoRenderer, IMFClockStateSink),
				QITABENT(CSinkVideoRenderer, IMFVideoDisplayControl),
				QITABENT(CSinkVideoRenderer, IMFMediaSinkPreroll),
				QITABENT(CSinkVideoRenderer, IMFVideoShaderEffect),
				{0}
		};

		return QISearch(this, qit, riid, ppv);
}

ULONG CSinkVideoRenderer::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"SinkRenderer::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CSinkVideoRenderer::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"SinkRenderer::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CSinkVideoRenderer::NotifyPreroll(MFTIME){

		TRACE_SINK((L"SinkRenderer::NotifyPreroll"));
		
		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		m_bPreroll = TRUE;

		IF_FAILED_RETURN(hr = m_pStreamVideoRenderer->RequestSample());
		IF_FAILED_RETURN(hr = m_pStreamVideoRenderer->RequestSample());
		IF_FAILED_RETURN(hr = m_pStreamVideoRenderer->RequestSample());

		return hr;
}

HRESULT CSinkVideoRenderer::ProcessSample(IMFSample* pSample){

		TRACE_SINK((L"SinkRenderer::ProcessSample"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_POINTER: S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cMFSafeQueue.PushSample(pSample);

		if(m_bPreroll  && m_cMFSafeQueue.GetSize() >= 3){

				LOG_HRESULT(hr = m_pStreamVideoRenderer->Preroll());
    m_bPreroll = FALSE;
		}

		return hr;
}

HRESULT CSinkVideoRenderer::GetVideoAttribute(UINT32* uiWidth, UINT32* uiHeight){

		TRACE_SINK((L"SinkRenderer::GetVideoAttribute"));

		HRESULT hr = S_OK;
		IMFMediaType* pMediaType = NULL;
		MFRatio fps;
		UINT64 AvgTimePerFrame = 0;

		try{

				IF_FAILED_THROW(hr = m_pStreamVideoRenderer->GetCurrentMediaType(&pMediaType));
				IF_FAILED_THROW(hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, uiWidth, uiHeight));
				IF_FAILED_THROW(hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, (UINT32*)&fps.Numerator, (UINT32*)&fps.Denominator));
				IF_FAILED_THROW(hr = MFFrameRateToAverageTimePerFrame(fps.Numerator, fps.Denominator, &AvgTimePerFrame));

				m_PerFrame_1_4th = AvgTimePerFrame / 4;
		}
		catch(HRESULT){}

		SAFE_RELEASE(pMediaType);

		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderReverseX(BOOL bReverse){

		TRACE_SINK((L"SinkRenderer::SetShaderReverseX"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetShaderReverseX(bReverse);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderReverseY(BOOL bReverse){

		TRACE_SINK((L"SinkRenderer::SetShaderReverseY"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetShaderReverseY(bReverse);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderNegatif(BOOL bNegatif){

		TRACE_SINK((L"SinkRenderer::SetShaderNegatif"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetShaderNegatif(bNegatif);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderGray(BOOL bGray){

		TRACE_SINK((L"SinkRenderer::SetShaderGray"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetShaderGray(bGray);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderGrayScale(BOOL bGray){

		TRACE_SINK((L"SinkRenderer::SetShaderGrayScale"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetShaderGrayScale(bGray);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderColor(UINT uiColorMode){

		TRACE_SINK((L"SinkRenderer::SetVideoColor"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetVideoColor(uiColorMode);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderContrast(FLOAT fContrast){

		TRACE_SINK((L"SinkRenderer::SetShaderContrast"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetVideoContrast(fContrast);
		
		return hr;
}

HRESULT CSinkVideoRenderer::SetShaderSaturation(FLOAT fSaturation){

		TRACE_SINK((L"SinkRenderer::SetShaderSaturation"));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		m_cDirectX9Manager.SetVideoSaturation(fSaturation);
		
		return hr;
}