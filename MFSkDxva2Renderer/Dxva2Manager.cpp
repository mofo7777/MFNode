//----------------------------------------------------------------------------------------------
// Dxva2Manager.cpp
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

DWORD WINAPI CDxva2Manager::StaticThreadProc(LPVOID lpParameter){

	CDxva2Manager* pDxva2Manager = reinterpret_cast<CDxva2Manager*>(lpParameter);

	BOOL bWindow = pDxva2Manager->InitWindow(pDxva2Manager->m_uiWidth, pDxva2Manager->m_uiHeight);

	if(bWindow == FALSE){
		SetEvent(pDxva2Manager->m_hThreadReadyEvent);
		return 1;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	SetEvent(pDxva2Manager->m_hThreadReadyEvent);

	while(GetMessage(&msg, NULL, 0, 0)){

		if(msg.message == WM_USER){
			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<DWORD>(msg.wParam);
}

LRESULT CALLBACK WndMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

	switch(msg){

	case WM_ERASEBKGND:
		// Suppress window erasing, to reduce flickering while the video is playing.
		return 1L;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	//return 0L;
}

CDxva2Manager::CDxva2Manager()
	: m_pDXVAHD(NULL),
	m_pDXVAVP(NULL),
	m_pD3D9(NULL),
	m_pDevice9(NULL),
	m_pSurface9(NULL),
	m_hWnd(NULL),
	m_uiFrameIndex(0),
	m_uiWidth(0),
	m_uiHeight(0),
	m_uiStride(0),
	m_uiYuvHeight(0),
	m_VideoFmt(D3DFMT_UNKNOWN),
	m_hRendererThread(NULL),
	m_hThreadReadyEvent(NULL),
	m_dwThreadID(0)
{
	ZeroMemory(m_ProcAmp, sizeof(m_ProcAmp));

	m_ProcAmp[0].Filter = DXVAHD_FILTER_BRIGHTNESS;
	m_ProcAmp[0].Stream = DXVAHD_STREAM_STATE_FILTER_BRIGHTNESS;
	m_ProcAmp[1].Filter = DXVAHD_FILTER_CONTRAST;
	m_ProcAmp[1].Stream = DXVAHD_STREAM_STATE_FILTER_CONTRAST;
	m_ProcAmp[2].Filter = DXVAHD_FILTER_HUE;
	m_ProcAmp[2].Stream = DXVAHD_STREAM_STATE_FILTER_HUE;
	m_ProcAmp[3].Filter = DXVAHD_FILTER_SATURATION;
	m_ProcAmp[3].Stream = DXVAHD_STREAM_STATE_FILTER_SATURATION;
	m_ProcAmp[4].Filter = DXVAHD_FILTER_NOISE_REDUCTION;
	m_ProcAmp[4].Stream = DXVAHD_STREAM_STATE_FILTER_NOISE_REDUCTION;
	m_ProcAmp[5].Filter = DXVAHD_FILTER_EDGE_ENHANCEMENT;
	m_ProcAmp[5].Stream = DXVAHD_STREAM_STATE_FILTER_EDGE_ENHANCEMENT;
	m_ProcAmp[6].Filter = DXVAHD_FILTER_ANAMORPHIC_SCALING;
	m_ProcAmp[6].Stream = DXVAHD_STREAM_STATE_FILTER_ANAMORPHIC_SCALING;
}

HRESULT CDxva2Manager::InitDxva2(const UINT32 uiWidth, const UINT32 uiHeight, const UINT32 uiStride, const D3DFORMAT VideoFmt){

	assert(m_hWnd == NULL);
	assert(m_pDXVAHD == NULL);
	assert(m_pDXVAVP == NULL);
	assert(m_pDevice9 == NULL);
	assert(m_pD3D9 == NULL);
	assert(m_pSurface9 == NULL);

	m_uiWidth = uiWidth;
	m_uiHeight = uiHeight;
	m_uiStride = uiStride;
	m_uiYuvHeight = m_uiHeight + (m_uiHeight / 2);
	m_VideoFmt = VideoFmt;
	m_uiFrameIndex = 0;

	HRESULT hr;
	DWORD dwID = 0;

	try{

		m_hThreadReadyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		IF_FAILED_THROW_LAST_ERROR(m_hThreadReadyEvent, E_POINTER);

		m_hRendererThread = CreateThread(NULL, 0, StaticThreadProc, (LPVOID)this, 0, &dwID);
		IF_FAILED_THROW_LAST_ERROR(m_hRendererThread, E_POINTER);

		HANDLE hObjects[] = {m_hThreadReadyEvent, m_hRendererThread};
		DWORD dwWait = 0;

		dwWait = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);

		if(WAIT_OBJECT_0 != dwWait){

			CLOSE_EVENT_IF(m_hRendererThread);
			IF_FAILED_THROW(E_UNEXPECTED);
		}

		m_dwThreadID = dwID;
	}
	catch(HRESULT hError){ hr = hError; }

	CLOSE_EVENT_IF(m_hThreadReadyEvent);

	IF_FAILED_RETURN(hr = (m_hWnd == NULL ? E_UNEXPECTED : S_OK));

	try{

		IF_FAILED_THROW(hr = InitDevice9());
		IF_FAILED_THROW(hr = InitVideoProcessor());
	}
	catch(HRESULT){}

	if(FAILED(hr)){
		ReleaseDxva2();
	}

	return hr;
}

void CDxva2Manager::ReleaseDxva2(){

	SAFE_RELEASE(m_pDXVAVP);
	SAFE_RELEASE(m_pDXVAHD);

	SAFE_RELEASE(m_pSurface9);
	SAFE_RELEASE(m_pDevice9);

	if(m_pD3D9){

		ULONG ulCount = m_pD3D9->Release();
		assert(ulCount == 0);
		m_pD3D9 = NULL;
	}

	if(IsWindow(m_hWnd)){

		PostThreadMessage(m_dwThreadID, WM_USER, 0, 0);

		WaitForSingleObject(m_hRendererThread, INFINITE);

		CLOSE_EVENT_IF(m_hRendererThread);

		m_dwThreadID = 0;

		DestroyWindow(m_hWnd);
		m_hWnd = NULL;

		const WCHAR wszClass[] = L"MFSkDxva2Renderer";
		UnregisterClass(wszClass, g_hModule);
	}

	m_uiWidth = 0;
	m_uiHeight = 0;
	m_uiStride = 0;
	m_uiYuvHeight = 0;
	m_VideoFmt = D3DFMT_UNKNOWN;
	m_uiFrameIndex = 0;
}

BOOL CDxva2Manager::InitWindow(const UINT32 uiWidth, const UINT32 uiHeight){

	BOOL bInit = FALSE;
	const WCHAR wszClass[] = L"MFSkDxva2Renderer";

	WNDCLASSEX WndClassEx;

	WndClassEx.cbSize = sizeof(WNDCLASSEX);
	WndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	WndClassEx.lpfnWndProc = WndMsgProc;
	WndClassEx.cbClsExtra = 0L;
	WndClassEx.cbWndExtra = 0L;
	WndClassEx.hInstance = g_hModule;
	WndClassEx.hIcon = NULL;
	WndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WndClassEx.lpszMenuName = NULL;
	WndClassEx.lpszClassName = wszClass;
	WndClassEx.hIconSm = NULL;

	if(!RegisterClassEx(&WndClassEx)){
		return bInit;
	}

	int iWndL = uiWidth + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		iWndH = uiHeight + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

	int iXWnd = (GetSystemMetrics(SM_CXSCREEN) - iWndL) / 2,
		iYWnd = (GetSystemMetrics(SM_CYSCREEN) - iWndH) / 2;

	if((m_hWnd = CreateWindow(wszClass, wszClass, WS_OVERLAPPED, iXWnd, iYWnd,
		iWndL, iWndH, GetDesktopWindow(), NULL, g_hModule, NULL)) == NULL){
		return bInit;
	}

	bInit = TRUE;

	ShowWindow(m_hWnd, SW_SHOWNORMAL);
	UpdateWindow(m_hWnd);

	return bInit;
}

HRESULT CDxva2Manager::OnFrameMove(IMFSample* pSample){

	HRESULT hr = S_OK;

	IMFMediaBuffer* pBuffer = NULL;
	BYTE* pData = NULL;
	DWORD dwLength = 0;

	D3DLOCKED_RECT d3dRect;

	try{

		IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pBuffer));
		IF_FAILED_THROW(hr = pBuffer->GetMaxLength(&dwLength));

		// Todo check dwLength

		IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, &dwLength));

		IF_FAILED_THROW(hr = m_pSurface9->LockRect(&d3dRect, NULL, 0));

		IF_FAILED_THROW(hr = MFCopyImage((BYTE*)d3dRect.pBits, d3dRect.Pitch, pData, m_uiStride, m_uiWidth, m_uiYuvHeight));

		IF_FAILED_THROW(hr = m_pSurface9->UnlockRect());
	}
	catch(HRESULT){}

	if(pBuffer && pData){
		LOG_HRESULT(pBuffer->Unlock());
	}

	SAFE_RELEASE(pBuffer);

	return hr;
}

HRESULT CDxva2Manager::OnFrameRender(){

	HRESULT hr = S_OK;
	IDirect3DSurface9 *pRT = NULL;
	DXVAHD_STREAM_DATA stream_data;
	ZeroMemory(&stream_data, sizeof(DXVAHD_STREAM_DATA));

	try{

		stream_data.Enable = TRUE;
		stream_data.OutputIndex = 0;
		stream_data.InputFrameOrField = m_uiFrameIndex;
		stream_data.pInputSurface = m_pSurface9;

		IF_FAILED_THROW(hr = m_pDevice9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pRT));

		IF_FAILED_THROW(hr = m_pDXVAVP->VideoProcessBltHD(pRT, m_uiFrameIndex, 1, &stream_data));

		IF_FAILED_THROW(hr = m_pDevice9->Present(NULL, NULL, NULL, NULL));

		m_uiFrameIndex++;
	}
	catch(HRESULT){}

	SAFE_RELEASE(pRT);

	return hr;
}

HRESULT CDxva2Manager::SetProcAmp(const DXVAHD_FILTER Filter, const BOOL bEnable, const INT iValue){

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pDXVAVP == NULL ? MF_E_NOT_INITIALIZED : S_OK));

	for(int i = 0; i < 7; i++){

		if(m_ProcAmp[i].Filter == Filter){

			if(m_ProcAmp[i].bSupported){

				if(iValue < m_ProcAmp[i].Range.Minimum || iValue > m_ProcAmp[i].Range.Maximum){
					hr = E_INVALIDARG;
				}
				else{

					// Todo use m_ProcAmp[i].Range.Multiplier : on my gpu all multiplier are 1.0f
					m_ProcAmp[i].FilterState.Enable = bEnable;
					m_ProcAmp[i].FilterState.Level = iValue;

					IF_FAILED_RETURN(hr = m_pDXVAVP->SetVideoProcessStreamState(0, m_ProcAmp[i].Stream, sizeof(m_ProcAmp[i].FilterState), &m_ProcAmp[i].FilterState));
				}
			}
			else{
				hr = MF_E_NET_UNSUPPORTED_CONFIGURATION;
			}
			break;
		}
	}

	return hr;
}

HRESULT CDxva2Manager::GetProcAmp(const DXVAHD_FILTER Filter, BOOL* pSupported, INT* pDefault, INT* pMin, INT* pMax){

	HRESULT hr;

	IF_FAILED_RETURN(hr = (m_pDXVAVP == NULL ? MF_E_NOT_INITIALIZED : S_OK));

	for(int i = 0; i < 7; i++){

		if(m_ProcAmp[i].Filter == Filter){

			*pSupported = m_ProcAmp[i].bSupported;
			*pDefault = m_ProcAmp[i].Range.Default;
			*pMin = m_ProcAmp[i].Range.Minimum;
			*pMax = m_ProcAmp[i].Range.Maximum;

			break;
		}
	}

	return hr;
}

HRESULT CDxva2Manager::InitDevice9(){

	HRESULT hr;
	D3DPRESENT_PARAMETERS d3dpp;

	IF_FAILED_RETURN(hr = Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3D9));

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.BackBufferWidth = 0;
	d3dpp.BackBufferHeight = 0;
	d3dpp.Windowed = true;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	IF_FAILED_RETURN(hr = m_pD3D9->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
		D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, NULL, &m_pDevice9));

	return hr;
}

HRESULT CDxva2Manager::InitVideoProcessor(){

	HRESULT hr;

	D3DFORMAT* pFormats = NULL;
	DXVAHD_VPCAPS* pVPCaps = NULL;
	UINT uiIndex;

	DXVAHD_CONTENT_DESC desc;

	desc.InputFrameFormat = DXVAHD_FRAME_FORMAT_PROGRESSIVE;
	desc.InputFrameRate.Numerator = 30;
	desc.InputFrameRate.Denominator = 1;
	desc.InputWidth = m_uiWidth;
	desc.InputHeight = m_uiHeight;
	desc.OutputFrameRate.Numerator = 30;
	desc.OutputFrameRate.Denominator = 1;
	desc.OutputWidth = m_uiWidth;
	desc.OutputHeight = m_uiHeight;

	DXVAHD_VPDEVCAPS caps;
	ZeroMemory(&caps, sizeof(caps));

	try{

		IF_FAILED_THROW(hr = DXVAHD_CreateDevice(m_pDevice9, &desc, DXVAHD_DEVICE_USAGE_PLAYBACK_NORMAL, NULL, &m_pDXVAHD));

		IF_FAILED_THROW(hr = m_pDXVAHD->GetVideoProcessorDeviceCaps(&caps));

		IF_FAILED_THROW(hr = (caps.MaxInputStreams < 1 ? E_FAIL : S_OK));

		pFormats = new (std::nothrow)D3DFORMAT[caps.OutputFormatCount];

		IF_FAILED_THROW(hr = (pFormats == NULL ? E_OUTOFMEMORY : S_OK));

		IF_FAILED_THROW(hr = m_pDXVAHD->GetVideoProcessorOutputFormats(caps.OutputFormatCount, pFormats));

		for(uiIndex = 0; uiIndex < caps.OutputFormatCount; uiIndex++){

			//LogFormat(pFormats[uiIndex]);
			if(pFormats[uiIndex] == D3DFMT_X8R8G8B8){
				break;
			}
		}

		IF_FAILED_THROW(hr = (uiIndex == caps.OutputFormatCount ? E_FAIL : S_OK));

		SAFE_DELETE_ARRAY(pFormats);

		pFormats = new (std::nothrow)D3DFORMAT[caps.InputFormatCount];

		IF_FAILED_THROW(hr = (pFormats == NULL ? E_OUTOFMEMORY : S_OK));

		IF_FAILED_THROW(hr = m_pDXVAHD->GetVideoProcessorInputFormats(caps.InputFormatCount, pFormats));

		for(uiIndex = 0; uiIndex < caps.InputFormatCount; uiIndex++){

			//LogFormat(pFormats[uiIndex]);
			if(pFormats[uiIndex] == m_VideoFmt){
				break;
			}
		}

		IF_FAILED_THROW(hr = (uiIndex == caps.InputFormatCount ? E_FAIL : S_OK));

		pVPCaps = new (std::nothrow)DXVAHD_VPCAPS[caps.VideoProcessorCount];

		IF_FAILED_THROW(hr = (pVPCaps == NULL ? E_OUTOFMEMORY : S_OK));

		IF_FAILED_THROW(hr = m_pDXVAHD->GetVideoProcessorCaps(caps.VideoProcessorCount, pVPCaps));

		IF_FAILED_THROW(hr = m_pDXVAHD->CreateVideoProcessor(&pVPCaps[0].VPGuid, &m_pDXVAVP));

		IF_FAILED_THROW(hr = m_pDXVAHD->CreateVideoSurface(m_uiWidth, m_uiHeight, m_VideoFmt, caps.InputPool, 0,
			DXVAHD_SURFACE_TYPE_VIDEO_INPUT, 1, &m_pSurface9, NULL));

		//IF_FAILED_THROW(hr = FillTextureWithBlack());
		IF_FAILED_THROW(hr = m_pDevice9->ColorFill(m_pSurface9, NULL, D3DCOLOR_XYUV(0, 128, 128)));
		IF_FAILED_THROW(hr = m_pDevice9->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 0), 1.0f, 0));

		GetProcAmpData(caps);
		IF_FAILED_THROW(hr = ConfigureVideoProcessor());
	}
	catch(HRESULT){}

	SAFE_DELETE_ARRAY(pFormats);
	SAFE_DELETE_ARRAY(pVPCaps);

	return hr;
}

void CDxva2Manager::GetProcAmpData(const DXVAHD_VPDEVCAPS& caps){

	HRESULT hr;

	for(DWORD i = 0; i < 7; i++){

		if(caps.FilterCaps & (1 << i)){

			hr = m_pDXVAHD->GetVideoProcessorFilterRange(m_ProcAmp[i].Filter, &m_ProcAmp[i].Range);

			if(SUCCEEDED(hr)){
				m_ProcAmp[i].bSupported = TRUE;
				m_ProcAmp[i].FilterState.Level = m_ProcAmp[i].Range.Default;
			}
			else{
				m_ProcAmp[i].bSupported = FALSE;
			}
		}
		else{
			m_ProcAmp[i].bSupported = FALSE;
		}
	}
}

HRESULT CDxva2Manager::ConfigureVideoProcessor(){

	HRESULT hr = S_OK;

	const RECT FrameRect = {0, 0, m_uiWidth, m_uiHeight};

	DXVAHD_STREAM_STATE_FRAME_FORMAT_DATA FrameFormat = {DXVAHD_FRAME_FORMAT_PROGRESSIVE};
	DXVAHD_STREAM_STATE_LUMA_KEY_DATA luma = {TRUE, 0.9f, 1.0f};
	DXVAHD_STREAM_STATE_ALPHA_DATA alpha = {TRUE, float(0xFF) / 0xFF};
	DXVAHD_STREAM_STATE_SOURCE_RECT_DATA src = {TRUE, FrameRect};
	DXVAHD_STREAM_STATE_DESTINATION_RECT_DATA dest = {TRUE, FrameRect};
	DXVAHD_BLT_STATE_TARGET_RECT_DATA tr = {TRUE, FrameRect};

	try{

		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(0, DXVAHD_STREAM_STATE_D3DFORMAT, sizeof(m_VideoFmt), &m_VideoFmt));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(0, DXVAHD_STREAM_STATE_FRAME_FORMAT, sizeof(FrameFormat), &FrameFormat));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(1, DXVAHD_STREAM_STATE_LUMA_KEY, sizeof(luma), &luma));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(0, DXVAHD_STREAM_STATE_ALPHA, sizeof(alpha), &alpha));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(0, DXVAHD_STREAM_STATE_SOURCE_RECT, sizeof(src), &src));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessStreamState(0, DXVAHD_STREAM_STATE_DESTINATION_RECT, sizeof(dest), &dest));
		IF_FAILED_THROW(hr = m_pDXVAVP->SetVideoProcessBltState(DXVAHD_BLT_STATE_TARGET_RECT, sizeof(tr), &tr));

		// Extended color info.
		//hr = AdjustExtendedColor(0);
		// Background color.
		//hr = AdjustBackgroundColor(0);
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CDxva2Manager::FillTextureWithBlack(){

	HRESULT hr = S_OK;

	DWORD dwUvHeight = m_uiHeight / 2;

	D3DLOCKED_RECT lr;
	BYTE y = 0x00;
	BYTE uv = 0x80;
	BYTE* pData;

	try{

		IF_FAILED_THROW(hr = m_pSurface9->LockRect(&lr, NULL, 0));

		IF_FAILED_THROW(hr = (lr.Pitch < (INT)m_uiWidth ? E_UNEXPECTED : S_OK));

		pData = (BYTE*)lr.pBits;

		for(UINT ui = 0; ui < m_uiHeight; ui++){

			memset(pData, y, m_uiWidth);
			pData += lr.Pitch;
		}

		for(UINT ui = 0; ui < dwUvHeight; ui++){

			memset(pData, uv, m_uiWidth);
			pData += lr.Pitch;
		}

		IF_FAILED_THROW(hr = m_pSurface9->UnlockRect());
	}
	catch(HRESULT){}

	return hr;
}