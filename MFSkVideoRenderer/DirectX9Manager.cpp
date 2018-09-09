//----------------------------------------------------------------------------------------------
// DirectX9Manager.cpp
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
#include "Stdafx.h"

DWORD WINAPI CDirectX9Manager::StaticThreadProc(LPVOID lpParameter){

	CDirectX9Manager* pDxManager = reinterpret_cast<CDirectX9Manager*>(lpParameter);

	BOOL bWindow = pDxManager->InitWindow(pDxManager->m_uiWidth, pDxManager->m_uiHeight);

	if(bWindow == FALSE){
		SetEvent(pDxManager->m_hThreadReadyEvent);
		return 1;
	}

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	SetEvent(pDxManager->m_hThreadReadyEvent);

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

CDirectX9Manager::CDirectX9Manager()
	: m_pD3D9(NULL),
	m_pDevice9(NULL),
	m_pVideoTextureY(NULL),
	m_pVideoTextureU(NULL),
	m_pVideoTextureV(NULL),
	m_pVideoEffect(NULL),
	m_hWnd(NULL),
	m_uiWidth(0),
	m_uiHeight(0),
	m_hRendererThread(NULL),
	m_hThreadReadyEvent(NULL),
	m_dwThreadID(0),
	m_bReverseX(FALSE),
	m_bSetbReverseX(FALSE),
	m_bReverseY(FALSE),
	m_bSetbReverseY(FALSE),
	m_bNegatif(FALSE),
	m_bSetNegatif(FALSE),
	m_bGray(FALSE),
	m_bSetGray(FALSE),
	m_bGrayScale(FALSE),
	m_bSetGrayScale(FALSE),
	m_uiColorMode(0),
	m_bSetColorMode(FALSE),
	m_fContrast(1.0f),
	m_bSetContrast(FALSE),
	m_fSaturation(1.0f),
	m_bSetSaturation(FALSE)
{
}

HRESULT CDirectX9Manager::InitDirectX9(const UINT32 uiWidth, const UINT32 uiHeight){

	assert(m_hWnd == NULL);
	assert(m_pVideoTextureY == NULL);
	assert(m_pVideoTextureU == NULL);
	assert(m_pVideoTextureV == NULL);
	assert(m_pVideoEffect == NULL);
	assert(m_pDevice9 == NULL);
	assert(m_pD3D9 == NULL);

	HRESULT hr;
	UINT uiAdapter = D3DADAPTER_DEFAULT;
	D3DDISPLAYMODE dm;
	D3DPRESENT_PARAMETERS pp;

	m_uiWidth = uiWidth;
	m_uiHeight = uiHeight;

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

	m_pD3D9 = Direct3DCreate9(D3D_SDK_VERSION);

	IF_FAILED_RETURN(hr = (m_pD3D9 == NULL ? E_FAIL : S_OK));

	IF_FAILED_RETURN(hr = m_pD3D9->GetAdapterDisplayMode(uiAdapter, &dm));

	ZeroMemory(&pp, sizeof(pp));

	pp.Windowed = TRUE;
	pp.hDeviceWindow = m_hWnd;
	pp.SwapEffect = D3DSWAPEFFECT_COPY;
	pp.BackBufferFormat = dm.Format;
	pp.BackBufferWidth = uiWidth;
	pp.BackBufferHeight = uiHeight;
	pp.Flags = D3DPRESENTFLAG_VIDEO;
	pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	IF_FAILED_RETURN(hr = m_pD3D9->CreateDevice(uiAdapter, D3DDEVTYPE_HAL, m_hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
		&pp, &m_pDevice9));

	IF_FAILED_RETURN(hr = m_pDevice9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE));
	IF_FAILED_RETURN(hr = m_pDevice9->SetRenderState(D3DRS_ZENABLE, FALSE));
	IF_FAILED_RETURN(hr = m_pDevice9->SetRenderState(D3DRS_LIGHTING, FALSE));

	m_cVideoFrame.OnCreate(uiWidth, uiHeight);
	IF_FAILED_RETURN(hr = m_cVideoFrame.OnRestore(m_pDevice9));

	IF_FAILED_RETURN(hr = m_pDevice9->CreateTexture(uiWidth, uiHeight, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &m_pVideoTextureY, NULL));
	IF_FAILED_RETURN(hr = m_pDevice9->CreateTexture(uiWidth / 2, uiHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &m_pVideoTextureU, NULL));
	IF_FAILED_RETURN(hr = m_pDevice9->CreateTexture(uiWidth / 2, uiHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_MANAGED, &m_pVideoTextureV, NULL));

	IF_FAILED_RETURN(hr = FillTextureForBlack());

	m_pVideoTextureY->PreLoad();
	m_pVideoTextureU->PreLoad();
	m_pVideoTextureV->PreLoad();

	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER | D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESHADERSTATE;

	IF_FAILED_RETURN(hr = D3DXCreateEffectFromResource(m_pDevice9, g_hModule, MAKEINTRESOURCE(IDR_EFFECT), NULL, NULL, dwShaderFlags, NULL, &m_pVideoEffect, NULL));

	IF_FAILED_RETURN(hr = m_pVideoEffect->SetTexture("g_TextureY", m_pVideoTextureY));
	IF_FAILED_RETURN(hr = m_pVideoEffect->SetTexture("g_TextureU", m_pVideoTextureU));
	IF_FAILED_RETURN(hr = m_pVideoEffect->SetTexture("g_TextureV", m_pVideoTextureV));

	IF_FAILED_RETURN(hr = InitWorldViewProj(uiWidth, uiHeight));

	return hr;
}

void CDirectX9Manager::ReleaseDirectX9(){

	m_uiWidth = 0;
	m_uiHeight = 0;

	m_cVideoFrame.OnDelete();

	SAFE_RELEASE(m_pVideoTextureY);
	SAFE_RELEASE(m_pVideoTextureU);
	SAFE_RELEASE(m_pVideoTextureV);

	SAFE_RELEASE(m_pVideoEffect);

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

		const WCHAR wszClass[] = L"MFSkVideoRenderer";
		UnregisterClass(wszClass, g_hModule);
	}
}

BOOL CDirectX9Manager::InitWindow(const UINT32 uiWidth, const UINT32 uiHeight){

	BOOL bInit = FALSE;
	const WCHAR wszClass[] = L"MFSkVideoRenderer";

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

	int iWndL = uiWidth + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
		iWndH = uiHeight + GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

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

HRESULT CDirectX9Manager::InitWorldViewProj(const UINT32 uiWidth, const UINT32 uiHeight){

	HRESULT hr;

	D3DXVECTOR3 vEye(0.0f, 0.0f, -700.0f);
	D3DXVECTOR3 vAt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f);

	D3DXMATRIX MatrixWorld;
	D3DXMATRIX MatrixView;
	D3DXMATRIX MatrixProj;

	D3DXMatrixIdentity(&MatrixWorld);

	D3DXMatrixLookAtLH(&MatrixView, &vEye, &vAt, &vUp);

	D3DXMatrixPerspectiveFovLH(&MatrixProj, D3DX_PI / 4.0f, static_cast<float>(uiWidth) / static_cast<float>(uiHeight), 0.1f, 10000.0f);

	D3DXMATRIX m = MatrixWorld * MatrixView * MatrixProj;
	IF_FAILED_RETURN(hr = m_pVideoEffect->SetMatrix("g_mWorldViewProjection", &m));

	return hr;
}

HRESULT CDirectX9Manager::OnFrameMove(IMFSample* pSample){

	HRESULT hr = S_OK;

	IMFMediaBuffer* pInputBuffer = NULL;
	BYTE* pInputData = NULL;
	DWORD dwInputLength = 0;
	DWORD dwYFrame = m_uiWidth * m_uiHeight;
	DWORD dwUVFrame = static_cast<DWORD>((m_uiWidth / 2.0f) * (m_uiHeight / 2.0f));

	D3DLOCKED_RECT lr;

	try{

		if(m_bSetbReverseX){

			m_bSetbReverseX = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetBool("g_bReverseX", m_bReverseX));
		}

		if(m_bSetbReverseY){

			m_bSetbReverseY = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetBool("g_bReverseY", m_bReverseY));
		}

		if(m_bSetNegatif){

			m_bSetNegatif = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetBool("g_bNegatif", m_bNegatif));
		}

		if(m_bSetGray){

			m_bSetGray = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetBool("g_bGray", m_bGray));
		}

		if(m_bSetGrayScale){

			m_bSetGrayScale = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetBool("g_bGrayScale", m_bGrayScale));
		}

		if(m_bSetColorMode){

			m_bSetColorMode = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetInt("g_iColorMode", m_uiColorMode));
		}

		if(m_bSetContrast){

			m_bSetContrast = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetFloat("g_fContrast", m_fContrast));
		}

		if(m_bSetSaturation){

			m_bSetSaturation = FALSE;
			IF_FAILED_THROW(hr = m_pVideoEffect->SetFloat("g_fSaturation", m_fSaturation));
		}

		IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pInputBuffer));
		IF_FAILED_THROW(hr = pInputBuffer->GetMaxLength(&dwInputLength));

		assert(dwInputLength > dwYFrame);

		IF_FAILED_THROW(hr = pInputBuffer->Lock(&pInputData, NULL, &dwInputLength));

		IF_FAILED_THROW(hr = m_pVideoTextureY->LockRect(0, &lr, NULL, 0));
		memcpy(lr.pBits, pInputData, dwYFrame);
		IF_FAILED_THROW(hr = m_pVideoTextureY->UnlockRect(0));

		IF_FAILED_THROW(hr = m_pVideoTextureU->LockRect(0, &lr, NULL, 0));
		memcpy(lr.pBits, pInputData + dwYFrame, dwUVFrame);
		IF_FAILED_THROW(hr = m_pVideoTextureU->UnlockRect(0));

		IF_FAILED_THROW(hr = m_pVideoTextureV->LockRect(0, &lr, NULL, 0));
		memcpy(lr.pBits, pInputData + dwYFrame + dwUVFrame, dwUVFrame);
		IF_FAILED_THROW(hr = m_pVideoTextureV->UnlockRect(0));
	}
	catch(HRESULT){}

	if(pInputBuffer && pInputData){
		LOG_HRESULT(pInputBuffer->Unlock());
	}

	SAFE_RELEASE(pInputBuffer);

	return hr;
}

HRESULT CDirectX9Manager::OnFrameRender(){

	HRESULT hr;

	IF_FAILED_RETURN(hr = m_pDevice9->Clear(0L, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0L));

	if(SUCCEEDED(hr = m_pDevice9->BeginScene())){

		UINT uiPasses;

		LOG_HRESULT(hr = m_pVideoEffect->Begin(&uiPasses, 0));

		LOG_HRESULT(hr = m_pVideoEffect->BeginPass(0));
		LOG_HRESULT(hr = m_cVideoFrame.OnRender(m_pDevice9));
		LOG_HRESULT(hr = m_pVideoEffect->EndPass());

		LOG_HRESULT(hr = m_pVideoEffect->End());

		IF_FAILED_RETURN(hr = m_pDevice9->EndScene());

		IF_FAILED_RETURN(hr = m_pDevice9->Present(NULL, NULL, NULL, NULL));
	}
	else{

		LOG_HRESULT(hr);
	}

	return hr;
}

HRESULT CDirectX9Manager::FillTextureForBlack(){

	HRESULT hr = S_OK;

	DWORD dwYFrame = m_uiWidth * m_uiHeight;
	DWORD dwUVFrame = static_cast<DWORD>((m_uiWidth / 2.0f) * (m_uiHeight / 2.0f));

	D3DLOCKED_RECT lr;
	BYTE y = 0x00;
	BYTE uv = 0x80;
	BYTE* pData;

	try{

		IF_FAILED_THROW(hr = m_pVideoTextureY->LockRect(0, &lr, NULL, 0));

		pData = (BYTE*)lr.pBits;

		for(UINT ui = 0; ui < dwYFrame; ui++)
			*pData++ = y;

		IF_FAILED_THROW(hr = m_pVideoTextureY->UnlockRect(0));

		IF_FAILED_THROW(hr = m_pVideoTextureU->LockRect(0, &lr, NULL, 0));

		pData = (BYTE*)lr.pBits;

		for(UINT ui = 0; ui < dwUVFrame; ui++)
			*pData++ = uv;

		IF_FAILED_THROW(hr = m_pVideoTextureU->UnlockRect(0));

		IF_FAILED_THROW(hr = m_pVideoTextureV->LockRect(0, &lr, NULL, 0));

		pData = (BYTE*)lr.pBits;

		for(UINT ui = 0; ui < dwUVFrame; ui++)
			*pData++ = uv;

		IF_FAILED_THROW(hr = m_pVideoTextureV->UnlockRect(0));
	}
	catch(HRESULT){}

	return hr;
}

/*
static inline void ApplyLetterBoxing(D2D1_RECT_F& rendertTargetArea, D2D1_SIZE_F& frameArea)
{
	const float aspectRatio = frameArea.width / frameArea.height;

	const float targetW = fabs(rendertTargetArea.right - rendertTargetArea.left);
	const float targetH = fabs(rendertTargetArea.bottom - rendertTargetArea.top);

	float tempH = targetW / aspectRatio;

	if(tempH <= targetH)
	// desired frame height is smaller than display
	// height so fill black on top and bottom of display
	{
		float deltaH = fabs(tempH - targetH) / 2;
		rendertTargetArea.top += deltaH;
		rendertTargetArea.bottom -= deltaH;
	}
	else
	//desired frame height is bigger than display
	// height so fill black on left and right of display
	{
		float tempW = targetH * aspectRatio;
		float deltaW = fabs(tempW - targetW) / 2;

		rendertTargetArea.left += deltaW;
		rendertTargetArea.right -= deltaW;
	}
}
*/