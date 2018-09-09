//----------------------------------------------------------------------------------------------
// MFNodeForm.cpp
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
#include "Stdafx.h"

CMFNodeForm* CMFNodeForm::m_pCMFNodeForm = NULL;

LRESULT CALLBACK MFNodeMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

		return CMFNodeForm::GetMFNodeForm()->MFNodeFormProc(hWnd, msg, wParam, lParam);
}

CMFNodeForm::CMFNodeForm()
		: m_hInst(NULL),
		  m_hWnd(NULL),
				m_hBackGround(NULL),
				m_pPlayer(NULL),
				m_bRepaintVideo(FALSE),
				m_bWaiting(FALSE),
				m_bSetCursor(FALSE)
{
		m_pCMFNodeForm = this;
		m_wszFile1[0] = '\0';
}

CMFNodeForm::~CMFNodeForm(){

		if(IsWindow(m_hWnd)){
				m_cMFControlManager.Close(m_hWnd);
				DestroyWindow(m_hWnd);
		}

		if(m_hBackGround)
				DeleteObject(m_hBackGround);

		UnregisterClass(MFNODE_CLASS, m_hInst);
}

HRESULT CMFNodeForm::InitForm(const HINSTANCE hInst){

		HRESULT hr = E_FAIL;

		InitCommonControls();

		m_hInst = hInst;
		m_hBackGround = CreateSolidBrush(RGB(0x80, 0x80, 0x80));

		WNDCLASSEX WndClassEx;

		WndClassEx.cbSize        = sizeof(WNDCLASSEX);
		WndClassEx.style         = CS_HREDRAW | CS_VREDRAW;
		WndClassEx.lpfnWndProc   = MFNodeMsgProc;
		WndClassEx.cbClsExtra    = 0L;
		WndClassEx.cbWndExtra    = 0L;
		WndClassEx.hInstance     = hInst;
		WndClassEx.hIcon         = NULL;//LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		WndClassEx.hCursor       = LoadCursor(NULL, IDC_ARROW);
		WndClassEx.hbrBackground = m_hBackGround;
		WndClassEx.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
		WndClassEx.lpszClassName = MFNODE_CLASS;
		WndClassEx.hIconSm       = NULL;//LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));

		if(!RegisterClassEx(&WndClassEx)){
				return hr;
		}

  int iWndL = 1024 + GetSystemMetrics(SM_CXSIZEFRAME) * 2,
      iWndH = 768 + GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

  int iXWnd = (GetSystemMetrics(SM_CXSCREEN) - iWndL) / 2,
      iYWnd = (GetSystemMetrics(SM_CYSCREEN) - iWndH) / 2;

  if((m_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, MFNODE_CLASS, MFNODE_CLASS, WS_OVERLAPPEDWINDOW, iXWnd, iYWnd,
					                       iWndL, iWndH, GetDesktopWindow(), NULL, m_hInst, NULL)) == NULL){
				return hr;
		}

		IF_FAILED_RETURN(hr = m_cMFControlManager.InitChilds(m_hInst, m_hWnd));

		EnableSessionControls(TRUE);

		ShowWindow(m_hWnd, SW_SHOWNORMAL);
  UpdateWindow(m_hWnd);

		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

		MFNodeCheck();

  return hr;
}

HRESULT CMFNodeForm::OpenWaveMixer(){

		HRESULT hr = S_OK;

		if(m_pPlayer)
				return hr;

		WCHAR szFile1[MAX_PATH];
		WCHAR szFile2[MAX_PATH];

		LRESULT lCount = SendMessage(m_cMFControlManager.GetChild(CHILD_EDIT_WAVE1), WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile1);

		if(lCount == 0)
				return hr;

		lCount = SendMessage(m_cMFControlManager.GetChild(CHILD_EDIT_WAVE2), WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile2);

		if(lCount == 0)
				return hr;

		if(CheckWaveFile(szFile1, szFile2) == FALSE){

				MessageBox(m_hWnd, L"Wave files must be :\r\n- same channels number\r\n- same bits per sample\r\n- same bitrate", L"Error", MB_OK);
				return hr;
		}

		if(FAILED(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE))){

				// MF_E_BAD_STARTUP_VERSION
				LOG_HRESULT(hr);
				return hr;
		}

		m_pPlayer = new (std::nothrow) CMFNodePlayer(m_hWnd);
		
		if(m_pPlayer == NULL){

				LOG_HRESULT(MFShutdown());
				hr = E_OUTOFMEMORY;
				return hr;
		}

		hr = m_pPlayer->OpenWaveMixer(szFile1, szFile2);

		if(FAILED(hr)){
				LOG_HRESULT(MFShutdown());
		}

		return hr;
}

HRESULT CMFNodeForm::OpenSession(const CURRENT_SESSION Session){

		HRESULT hr = S_OK;

		// Should not happen, because menu are disabled when playing
		if(m_pPlayer)
				return hr;

		if(FAILED(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE))){

				// MF_E_BAD_STARTUP_VERSION
				LOG_HRESULT(hr);
				return hr;
		}

		m_pPlayer = new (std::nothrow) CMFNodePlayer(m_hWnd);
		
		if(m_pPlayer == NULL){

				LOG_HRESULT(MFShutdown());
				hr = E_OUTOFMEMORY;
				return hr;
		}

		if(Session == SESSION_SCREENSHOT){

				hr = m_pPlayer->OpenScreenShot();
		}
		else if(Session == SESSION_HTTP){
				hr = m_pPlayer->OpenHttpStreamer();
		}
		else if(Session == SESSION_AVCAPTURE){
				hr = m_pPlayer->OpenAVCapture();
		}
		else if(Session == SESSION_FLV || Session == SESSION_RENDERER || Session == SESSION_DXVA2){

				WCHAR szFile[MAX_PATH];

				HWND hWnd;

				if(Session == SESSION_FLV)
						hWnd = m_cMFControlManager.GetChild(CHILD_EDIT_FLV);
				else if(Session == SESSION_RENDERER)
						hWnd = m_cMFControlManager.GetChild(CHILD_EDIT_RENDERER);
				else
						hWnd = m_cMFControlManager.GetChild(CHILD_EDIT_MPEG);

				LRESULT lCount = SendMessage(hWnd, WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile);

				if(lCount == 0){
						hr = E_FAIL;
				}
				else{

						if(Session == SESSION_FLV){
								hr = m_pPlayer->OpenFlvFile(szFile);
						}
						else if(Session == SESSION_DXVA2){
								hr = m_pPlayer->OpenFileDxva2(szFile);
						}
						else{

								CUDA_DECODER decoder = CUDA_DECODER_NONE;

								if(SendMessage(m_cMFControlManager.GetChild(CHILD_RADIO_CUDA), BM_GETCHECK, 0, 0)){
										decoder = CUDA_DECODER_CUDA;
								}

								hr = m_pPlayer->OpenFileRenderer(szFile, decoder);
						}
				}
		}

		if(FAILED(hr)){
				hr = m_pPlayer->CloseSession();
		}

		return hr;
}

HRESULT CMFNodeForm::OpenSequencer(){

		HRESULT hr = S_OK;

		if(m_pPlayer)
				return hr;

		WCHAR szFile1[MAX_PATH];
		WCHAR szFile2[MAX_PATH];

		LRESULT lCount = SendMessage(m_cMFControlManager.GetChild(CHILD_EDIT_VIDEO1), WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile1);

		if(lCount == 0)
				return hr;

		lCount = SendMessage(m_cMFControlManager.GetChild(CHILD_EDIT_VIDEO2), WM_GETTEXT, (WPARAM)MAX_PATH, (LPARAM)szFile2);

		if(lCount == 0)
				return hr;

		if(FAILED(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE))){

				// MF_E_BAD_STARTUP_VERSION
				LOG_HRESULT(hr);
				return hr;
		}

		m_pPlayer = new (std::nothrow) CMFNodePlayer(m_hWnd);
		
		if(m_pPlayer == NULL){

				LOG_HRESULT(MFShutdown());
				hr = E_OUTOFMEMORY;
				return hr;
		}

		hr = m_pPlayer->OpenSequencer(szFile1, szFile2);

		if(FAILED(hr)){
				LOG_HRESULT(MFShutdown());
		}

		return hr;
}

HRESULT CMFNodeForm::OpenFreeSession(const WCHAR* wszFile){

		HRESULT hr = S_OK;
		
		if(m_pPlayer != NULL){
				
				//m_wszFile = wszFile;
				memcpy(m_wszFile1, wszFile, MAX_PATH * sizeof(WCHAR));
				m_pPlayer->CloseSession();
				return hr;
  }

		if(FAILED(hr = MFStartup(MF_VERSION, MFSTARTUP_LITE))){

				// MF_E_BAD_STARTUP_VERSION
				LOG_HRESULT(hr);
				return hr;
		}

		m_pPlayer = new (std::nothrow) CMFNodePlayer(m_hWnd);
		
		if(m_pPlayer == NULL){

				LOG_HRESULT(MFShutdown());
				hr = E_OUTOFMEMORY;
				return hr;
		}

		// We can use OpenFlvFile
		hr = m_pPlayer->OpenFlvFile(wszFile);

		if(FAILED(hr)){
				LOG_HRESULT(MFShutdown());
		}

		return hr;
}

HRESULT CMFNodeForm::CloseSession(){

		HRESULT hr = S_OK;

		if(m_pPlayer){
				
				//SAFE_RELEASE(m_pPlayer);

				// To be sure that MediaSession does not reference the Player.
				ULONG ulPlayer = m_pPlayer->Release();				
				m_pPlayer = NULL;
				assert(ulPlayer == 0);

				LOG_HRESULT(MFShutdown());
		}

		SetWindowText(m_hWnd, L"MFNode Player : closed");
		InvalidateRect(m_hWnd, NULL, TRUE);

		EnableSessionControls(TRUE);
		EnableControls(FALSE);

		return hr;
}

void CMFNodeForm::AddMediaFileToEdit(const int iChildEdit){

		OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  TCHAR szBuffer[MAX_PATH];
  szBuffer[0] = NULL;

  static const TCHAR szWavFilter[] = TEXT("Wave file (.wav)\0*.WAV\0") \
                                     TEXT("all files (*.*)\0*.*\0\0");

		static const TCHAR szFlvFilter[] = TEXT("Flv file (.flv)\0*.FLV\0") \
                                     TEXT("all files (*.*)\0*.*\0\0");

		static const TCHAR szMp4Filter[] = TEXT("Mp4 file (.mp4)\0*.MP4\0") \
                                     TEXT("all files (*.*)\0*.*\0\0");

		static const TCHAR szMpgFilter[] = TEXT("Mpg file (.mpg)\0*.MPG\0") \
				                                 TEXT("Mpeg2 file (.mpeg)\0*.MPEG\0") \
                                     TEXT("all files (*.*)\0*.*\0\0");

  ofn.lStructSize         = sizeof(OPENFILENAME);
  ofn.hwndOwner           = m_hWnd;
  //ofn.lpstrFilter         = szFilter;
  ofn.nFilterIndex        = 1;
  ofn.lpstrFile           = szBuffer;
  ofn.nMaxFile            = MAX_PATH;
  //ofn.lpstrTitle          = TEXT("Select wave file");
  ofn.Flags               = OFN_HIDEREADONLY;
  //ofn.lpstrDefExt         = TEXT("WAV");

		switch(iChildEdit){

		  case CHILD_EDIT_WAVE1:
				case CHILD_EDIT_WAVE2:
						ofn.lpstrFilter = szWavFilter;
						ofn.lpstrTitle = TEXT("Select wave file");
      ofn.lpstrDefExt = TEXT("WAV");
						break;

				case CHILD_EDIT_FLV:
						ofn.lpstrFilter = szFlvFilter;
						ofn.lpstrTitle = TEXT("Select flv file");
      ofn.lpstrDefExt = TEXT("FLV");
						break;

				case CHILD_EDIT_VIDEO1:
				case CHILD_EDIT_VIDEO2:
						ofn.lpstrFilter = szMp4Filter;
						ofn.lpstrTitle = TEXT("Select mp4 file");
      ofn.lpstrDefExt = TEXT("MP4");
						break;

				default:
						ofn.lpstrFilter = szMpgFilter;
						ofn.lpstrTitle = TEXT("Select mpg file");
      ofn.lpstrDefExt = TEXT("MPG");
						break;
		}
    
  if(GetOpenFileName(&ofn)){

				HWND hEdit = m_cMFControlManager.GetChild(iChildEdit);    
				SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)szBuffer);
  }
}

void CMFNodeForm::EnableSessionControls(const BOOL bEnable){

		EnableMenuItem(GetMenu(m_hWnd), ID_CLOSE_SESSION, bEnable);
		EnableMenuItem(GetMenu(m_hWnd), ID_CONTROL_PLAY, bEnable);
		EnableMenuItem(GetMenu(m_hWnd), ID_CONTROL_PAUSE, bEnable);
		EnableMenuItem(GetMenu(m_hWnd), ID_CONTROL_STOP, bEnable);
}

void CMFNodeForm::EnableControls(const BOOL bEnable){

		EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_SCREEN, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_WAVEMIXER, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_HTTP, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_FLV, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_RENDERER, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_SEQUENCER, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_AVCAPTURE, bEnable);
  EnableMenuItem(GetMenu(m_hWnd), ID_OPEN_DXVA2, bEnable);
}

BOOL CMFNodeForm::CheckWaveFile(const WCHAR* szWaveFile1, const WCHAR* szWaveFile2){

		BOOL bCheck = FALSE;

		CMFFile WaveFile1;
		CMFFile WaveFile2;

		if(WaveFile1.MFOpenFile(szWaveFile1) == FALSE)
				return bCheck;

		if(WaveFile2.MFOpenFile(szWaveFile2) == FALSE)
				return bCheck;

		const int iMinWaveHeader = 36;

		if(WaveFile1.GetSizeFile() < iMinWaveHeader || WaveFile2.GetSizeFile() < iMinWaveHeader)
				return bCheck;

		BYTE szWaveHeader1[iMinWaveHeader];
		BYTE szWaveHeader2[iMinWaveHeader];

		BYTE* pFile1 = szWaveHeader1;
		BYTE* pFile2 = szWaveHeader2;

		memcpy(pFile1, WaveFile1.GetFileBuffer(), iMinWaveHeader);
		memcpy(pFile2, WaveFile2.GetFileBuffer(), iMinWaveHeader);

		const char Riff[] = {'R', 'I', 'F', 'F', '\0'};

		if(memcmp(pFile1, Riff, 4) != 0)
				return bCheck;

		if(memcmp(pFile2, Riff, 4) != 0)
				return bCheck;

		pFile1 += 8;
		pFile2 += 8;

		const char WavFmt[] = {'W', 'A', 'V', 'E', 'f', 'm', 't', ' ', '\0'};

		if(memcmp(pFile1, WavFmt, 8) != 0)
				return bCheck;

		if(memcmp(pFile2, WavFmt, 8) != 0)
				return bCheck;

		pFile1 += 12;
		pFile2 += 12;

		WAVEFORMATEX fmt1;
		WAVEFORMATEX fmt2;

		const int iWaveFmt = sizeof(WAVEFORMATEX);

		memcpy(&fmt1, pFile1, iWaveFmt);
		memcpy(&fmt2, pFile2, iWaveFmt);

		if(fmt1.wFormatTag == fmt2.wFormatTag && fmt1.nChannels == fmt2.nChannels && fmt1.wBitsPerSample == fmt2.wBitsPerSample &&
				fmt1.nSamplesPerSec == fmt2.nSamplesPerSec)
				bCheck = TRUE;

		return bCheck;
}

void CMFNodeForm::MFNodeCheck(){

		if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))){

				IUnknown* pMFNode = NULL;
				IMFMediaSink* pSink = NULL;

				wstring wszInfo = L"This MFNode is not registered on your system ! Check ";
				wstring wszDll;
				const WCHAR wszPlayer[] = L"MFNodePlayer";

				if(FAILED(CoCreateInstance(CLSID_MFTWaveMixer, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFTWaveMixer.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFSrScreenCapture, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFSrScreenCapture.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFTJpegEncoder, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFTJpegEncoder.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFTFlvSplitter, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFSrFlvSplitter.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFSrMpeg2Splitter, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFSrMpeg2Splitter.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFTCudaDecoder, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFTCudaDecoder.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFTDxva2Decoder, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFTDxva2Decoder.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFTVp6Decoder, NULL, CLSCTX_INPROC, __uuidof(IUnknown), reinterpret_cast<void**>(&pMFNode)))){
						wszDll = wszInfo + L"MFTVp6Decoder.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				SAFE_RELEASE(pMFNode);

				if(FAILED(CoCreateInstance(CLSID_MFSkJpegHttpStreamer, NULL, CLSCTX_INPROC, __uuidof(IMFMediaSink), reinterpret_cast<void**>(&pSink)))){
						wszDll = wszInfo + L"MFSkJpegHttpStreamer.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				if(pSink){

						// Because of circular reference in MFSkJpegHttpStreamer, we need to call Shutdown.
						pSink->Shutdown();
				  SAFE_RELEASE(pSink);
				}

				if(FAILED(CoCreateInstance(CLSID_MFSkImageWriter, NULL, CLSCTX_INPROC, __uuidof(IMFMediaSink), reinterpret_cast<void**>(&pSink)))){
						wszDll = wszInfo + L"MFSkImageWriter.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				if(pSink){

						// Because of circular reference in MFSkImageWriter, we need to call Shutdown.
						pSink->Shutdown();
				  SAFE_RELEASE(pSink);
				}

				if(FAILED(CoCreateInstance(CLSID_MFSkVideoRenderer, NULL, CLSCTX_INPROC, __uuidof(IMFMediaSink), reinterpret_cast<void**>(&pSink)))){
						wszDll = wszInfo + L"MFSkVideoRenderer.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				if(pSink){

						// Because of circular reference in MFSkVideoRenderer, we need to call Shutdown.
						pSink->Shutdown();
				  SAFE_RELEASE(pSink);
				}

				if(FAILED(CoCreateInstance(CLSID_MFSkDxva2Renderer, NULL, CLSCTX_INPROC, __uuidof(IMFMediaSink), reinterpret_cast<void**>(&pSink)))){
						wszDll = wszInfo + L"MFSkDxva2Renderer.dll";
						MessageBox(NULL, wszDll.c_str(), wszPlayer, MB_OK | MB_ICONWARNING);
				}

				if(pSink){

						// Because of circular reference in MFSkDxva2Renderer, we need to call Shutdown.
						pSink->Shutdown();
				  SAFE_RELEASE(pSink);
				}

				CoUninitialize();
		}
}

void CMFNodeForm::UpdateDxva2Bar(){

		assert(m_pPlayer);

		BOOL bSupported = FALSE;
		int iVal;
		int iMin;
		int iMax;

		HWND hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_BRIGHTNESS);

		m_pPlayer->GetBrightnessRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_HUE);

		m_pPlayer->GetHueRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_CONTRAST);

		m_pPlayer->GetContrastRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_SATURATION);

		m_pPlayer->GetSaturationRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_NOISE);

		m_pPlayer->GetNoiseRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_EDGE);

		m_pPlayer->GetEdgeRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}

		hChild = m_cMFControlManager.GetChild(CHILD_TRACKBAR_DXVA2_ANAMORPHIC);

		m_pPlayer->GetAnamorphicRange(&bSupported, &iVal, &iMin, &iMax);

		EnableWindow(hChild, bSupported);

		if(bSupported){

				SendMessage(hChild, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)iMin);
				SendMessage(hChild, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)iMax);
				SendMessage(hChild, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)iVal);
		}
}