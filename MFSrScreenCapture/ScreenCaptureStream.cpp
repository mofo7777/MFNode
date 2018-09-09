//----------------------------------------------------------------------------------------------
// ScreenCaptureStream.cpp
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
#include "StdAfx.h"

CScreenCaptureStream::CScreenCaptureStream(CScreenCapture* pSource, IMFStreamDescriptor* pSD, const DWORD dwSize, HRESULT& hr)
		: m_nRefCount(1),
				m_pEventQueue(NULL),
    m_State(SourceStopped),
				m_pMediaBuffer(NULL),
				m_pVideoBuffer(NULL),
				m_dwVideoSize(dwSize),
    m_rtCurrentPosition(0){

		TRACE_STREAMSOURCE((L"Stream::CTOR"));
		
		assert(pSource != NULL);
  assert(pSD != NULL);

		m_pScreenCapture = pSource;
		m_pScreenCapture->AddRef();

		m_pStreamDescriptor = pSD;
		m_pStreamDescriptor->AddRef();

		try{

				IF_FAILED_THROW(hr = MFCreateEventQueue(&m_pEventQueue));

				IF_FAILED_THROW(hr = MFCreateAlignedMemoryBuffer(m_dwVideoSize, MF_16_BYTE_ALIGNMENT, &m_pMediaBuffer));
				IF_FAILED_THROW(hr = m_pMediaBuffer->SetCurrentLength(m_dwVideoSize));

				#ifdef REVERSE_GDI_IMAGE
				  IF_FAILED_THROW(hr = MFCreateAlignedMemoryBuffer(m_dwVideoSize, MF_16_BYTE_ALIGNMENT, &m_pMediaGdiBuffer));
						IF_FAILED_THROW(hr = m_pMediaGdiBuffer->SetCurrentLength(m_dwVideoSize));
    #endif
		}
		catch(HRESULT){}
}

HRESULT CScreenCaptureStream::CreateInstance(CScreenCaptureStream** ppStream, CScreenCapture* pSource, IMFStreamDescriptor* pSD, const DWORD dwSize, HRESULT& hr){
		
		TRACE_STREAMSOURCE((L"Stream::CreateInstance"));

		IF_FAILED_RETURN(hr = (ppStream == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pSource == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pSD == NULL ? E_POINTER : S_OK));

		CScreenCaptureStream* pStream = new (std::nothrow)CScreenCaptureStream(pSource, pSD, dwSize, hr);
		
		IF_FAILED_RETURN(pStream == NULL ? E_OUTOFMEMORY : S_OK);

		*ppStream = pStream;
		(*ppStream)->AddRef();

		SAFE_RELEASE(pStream);
		
		return hr;
}

HRESULT CScreenCaptureStream::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_STREAMSOURCE((L"Stream::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CScreenCaptureStream, IMFMediaEventGenerator),
				QITABENT(CScreenCaptureStream, IMFMediaStream),
				{ 0 }
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CScreenCaptureStream::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Stream::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CScreenCaptureStream::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Stream::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CScreenCaptureStream::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState){

		TRACE_STREAMSOURCE((L"Stream::BeginGetEvent"));

		HRESULT hr;

  AutoLock lock(m_CriticSection);
		
		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->BeginGetEvent(pCallback, punkState));

		return hr;
}

HRESULT CScreenCaptureStream::EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent){

		TRACE_STREAMSOURCE((L"Stream::EndGetEvent"));

		HRESULT hr;

  AutoLock lock(m_CriticSection);
		
		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->EndGetEvent(pResult, ppEvent));

		return hr;
}

HRESULT CScreenCaptureStream::GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent){
		
		TRACE_STREAMSOURCE((L"Stream::GetEvent"));

		HRESULT hr;

		IMFMediaEventQueue* pQueue = NULL;

		{
				AutoLock lock(m_CriticSection);

				LOG_HRESULT(hr = CheckShutdown());

				if(SUCCEEDED(hr)){
						pQueue = m_pEventQueue;
						pQueue->AddRef();
				}
		}

		if(SUCCEEDED(hr)){
				LOG_HRESULT(hr = pQueue->GetEvent(dwFlags, ppEvent));
		}

		SAFE_RELEASE(pQueue);

		return hr;
}

HRESULT CScreenCaptureStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue){

		TRACE_STREAMSOURCE((L"Stream::QueueEvent : %s", MFEventString(met)));

		HRESULT hr;

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());
		
		LOG_HRESULT(hr = m_pEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue));

		return hr;
}

HRESULT CScreenCaptureStream::GetMediaSource(IMFMediaSource** ppMediaSource){

		TRACE_STREAMSOURCE((L"Stream::GetMediaSource"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppMediaSource == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		// If called after shutdown, then m_pSource is NULL. Otherwise, m_pSource should not be NULL.
		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_pScreenCapture == NULL ? E_UNEXPECTED : S_OK));

		LOG_HRESULT(hr = m_pScreenCapture->QueryInterface(IID_PPV_ARGS(ppMediaSource)));

		return hr;
}

HRESULT CScreenCaptureStream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor){

		TRACE_STREAMSOURCE((L"Stream::GetStreamDescriptor"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppStreamDescriptor == NULL ? E_POINTER : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_pStreamDescriptor == NULL ? E_UNEXPECTED : S_OK));

		IF_FAILED_RETURN(hr = CheckShutdown());

		*ppStreamDescriptor = m_pStreamDescriptor;
		(*ppStreamDescriptor)->AddRef();

		return hr;
}

HRESULT CScreenCaptureStream::RequestSample(IUnknown* pToken){

		TRACE_STREAMSOURCE((L"Stream::RequestSample"));

		HRESULT hr;
		
		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = (m_pScreenCapture == NULL ? E_UNEXPECTED : S_OK));

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = (m_pScreenCapture->GetState() == SourceStopped ? MF_E_INVALIDREQUEST : S_OK));

		if(m_pScreenCapture->GetState() == SourcePaused){
				return S_OK;
		}

		IMFSample* pSample = NULL;

		try{
				
				IF_FAILED_THROW(hr = CreateScreenVideoSample(&pSample));

				if(pToken){
						IF_FAILED_THROW(hr = pSample->SetUnknown(MFSampleExtension_Token, pToken));
				}

				IF_FAILED_THROW(hr = m_pEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, hr, pSample));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSample);

		return hr;
}

HRESULT CScreenCaptureStream::Shutdown(){

		TRACE_STREAMSOURCE((L"Stream::Shutdown"));

		AutoLock lock(m_CriticSection);

		HRESULT hr;
		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_pEventQueue){
				LOG_HRESULT(m_pEventQueue->Shutdown());
		}

		SAFE_RELEASE(m_pEventQueue);
		SAFE_RELEASE(m_pScreenCapture);
		SAFE_RELEASE(m_pStreamDescriptor);

		if(m_pMediaBuffer && m_pVideoBuffer){
				LOG_HRESULT(hr = m_pMediaBuffer->Unlock());
				m_pVideoBuffer = NULL;
		}

		SAFE_RELEASE(m_pMediaBuffer);
		m_dwVideoSize = 0;

		#ifdef REVERSE_GDI_IMAGE
  		if(m_pMediaGdiBuffer && m_pVideoGdiBuffer){
						LOG_HRESULT(hr = m_pMediaGdiBuffer->Unlock());
						m_pVideoGdiBuffer = NULL;
				}

				SAFE_RELEASE(m_pMediaGdiBuffer);
  #endif

		m_State = SourceShutdown;

		return S_OK;
}

HRESULT CScreenCaptureStream::CreateScreenVideoSample(IMFSample** ppSample){
		
		TRACE_STREAMSOURCE((L"Stream::video"));

		HRESULT hr;

		IF_FAILED_RETURN(hr = (m_pMediaBuffer == NULL ? E_POINTER : S_OK));

		#ifdef REVERSE_GDI_IMAGE
		  IF_FAILED_RETURN(hr = (m_pMediaGdiBuffer == NULL ? E_POINTER : S_OK));
  #endif

		IMFSample* pSample = NULL;
		LONGLONG duration = VIDEO_DURATION;

		try{

				IF_FAILED_THROW(hr = m_pMediaBuffer->Lock(&m_pVideoBuffer, NULL, NULL));
				IF_FAILED_THROW(hr = CaptureGDIScreen());

				IF_FAILED_THROW(hr = m_pMediaBuffer->Unlock());
				m_pVideoBuffer = NULL;

				IF_FAILED_THROW(hr = MFCreateSample(&pSample));
				IF_FAILED_THROW(hr = pSample->AddBuffer(m_pMediaBuffer));
				IF_FAILED_THROW(hr = pSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE));
				IF_FAILED_THROW(hr = pSample->SetSampleTime(m_rtCurrentPosition));
				IF_FAILED_THROW(hr = pSample->SetSampleDuration(duration));

				m_rtCurrentPosition += duration;

				*ppSample = pSample;
				(*ppSample)->AddRef();
		}
		catch(HRESULT){}

		if(m_pMediaBuffer && m_pVideoBuffer){
				LOG_HRESULT(hr = m_pMediaBuffer->Unlock());
				m_pVideoBuffer = NULL;
		}

		SAFE_RELEASE(pSample);

		return hr;
}

HRESULT CScreenCaptureStream::CaptureGDIScreen(){

		HRESULT hr = S_OK;
		HDC hScreenDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hbm = NULL;

		try{

				hScreenDC = ::GetDC(NULL);
				hMemDC = ::CreateCompatibleDC(hScreenDC);
				hbm = ::CreateCompatibleBitmap(hScreenDC, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

				SelectObject(hMemDC, hbm);

    if(!StretchBlt(hMemDC, 0, 0, VIDEO_WIDTH, VIDEO_HEIGHT, hScreenDC,  0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SRCCOPY))
						throw E_FAIL;

				BITMAPINFO bmi;
				ZeroMemory(&bmi, sizeof(BITMAPINFO));
				bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = VIDEO_WIDTH;
    bmi.bmiHeader.biHeight = VIDEO_HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
				bmi.bmiHeader.biBitCount = VIDEO_BYTE_RGB;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = VIDEO_WIDTH * VIDEO_HEIGHT * VIDEO_OCTET_RGB;
    //bmi.bmiHeader.biXPelsPerMeter = 0;
    //bmi.bmiHeader.biYPelsPerMeter = 0;
    //bmi.bmiHeader.biClrUsed = 0;
    //bmi.bmiHeader.biClrImportant = 0;

				#ifdef REVERSE_GDI_IMAGE
				  IF_FAILED_THROW(hr = m_pMediaGdiBuffer->Lock(&m_pVideoGdiBuffer, NULL, NULL));

						if(GetDIBits(hMemDC, hbm, 0, bmi.bmiHeader.biHeight, m_pVideoGdiBuffer, &bmi, DIB_RGB_COLORS) == 0)
								throw E_FAIL;

						const int iWidthSize = VIDEO_WIDTH * VIDEO_OCTET_RGB;

						for(int i = 0, j = VIDEO_HEIGHT - 1; i < VIDEO_HEIGHT; i++, j--)
								for(int k = 0; k < iWidthSize; k++)
										m_pVideoBuffer[(i * iWidthSize) + k] = m_pVideoGdiBuffer[(j * iWidthSize) + k];
    #else
				  if(GetDIBits(hMemDC, hbm, 0, bmi.bmiHeader.biHeight, m_pVideoBuffer, &bmi, DIB_RGB_COLORS) == 0)
								throw E_FAIL;
    #endif
		}
		catch(HRESULT){}

		#ifdef REVERSE_GDI_IMAGE
		  if(m_pMediaGdiBuffer && m_pVideoGdiBuffer){
				  LOG_HRESULT(hr = m_pMediaGdiBuffer->Unlock());
				  m_pVideoGdiBuffer = NULL;
		  }
  #endif

		if(hbm)
				DeleteObject(hbm);

		if(hMemDC)
				DeleteDC(hMemDC);

		if(hScreenDC)
				ReleaseDC(NULL, hScreenDC);

    return hr;
}