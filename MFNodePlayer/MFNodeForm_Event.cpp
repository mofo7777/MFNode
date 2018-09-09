//----------------------------------------------------------------------------------------------
// MFNodeForm_Event.cpp
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

LRESULT CALLBACK CMFNodeForm::MFNodeFormProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){

		switch(msg){

		  case WM_PAINT:
						if(m_bRepaintVideo && m_pPlayer){
				    m_pPlayer->RedrawVideo();
								return DefWindowProc(hWnd, msg, wParam, lParam);
		    }
		    else{
								OnPaint(hWnd);
						}
						break;

				case WM_SIZE:
						ResizeVideo(LOWORD(lParam), HIWORD(lParam));
						break;

				case WM_ERASEBKGND:
						// Suppress window erasing, to reduce flickering while the video is playing.
						return 1;

				// WM_NCHITTEST
				// Todo : handle caret on editbox.
				// On HTCLIENT, GetClientRect on editBox, if mouse is in, setcaret.
				case WM_SETCURSOR:

						switch(LOWORD(lParam)){

						  case HTTOP:
								case HTBOTTOM:
										SetCursor(LoadCursor(0, IDC_SIZENS));
										m_bSetCursor = TRUE;
										return 0;

						  case HTLEFT:
								case HTRIGHT:
										SetCursor(LoadCursor(0, IDC_SIZEWE));
										m_bSetCursor = TRUE;
										return 0;

						  case HTTOPLEFT:
								case HTBOTTOMRIGHT:
										SetCursor(LoadCursor(0, IDC_SIZENWSE));
										m_bSetCursor = TRUE;
										return 0;

						  case HTTOPRIGHT:
								case HTBOTTOMLEFT:
										SetCursor(LoadCursor(0, IDC_SIZENESW));
										m_bSetCursor = TRUE;
										return 0;

								default:
										if(m_bSetCursor == FALSE){
												return 1;
										}
										else{
												SetCursor(LoadCursor(0, MAKEINTRESOURCE(m_bWaiting ? IDC_WAIT : IDC_ARROW)));
												m_bSetCursor = FALSE;
												return 0;
										}
						}

				case WM_APP_PLAYER_EVENT:
						OnPlayerEvent(wParam, lParam);
						break;

				case WM_COMMAND:

						if(OnCommand(LOWORD(wParam))){
								return DefWindowProc(hWnd, msg, wParam, lParam);
						}
						break;

    case WM_HSCROLL:
						if((LOWORD(wParam) == TB_THUMBTRACK || LOWORD(wParam) == TB_PAGEUP || LOWORD(wParam) == TB_PAGEDOWN) && m_pPlayer != NULL){

								OnHorizontalScroll((HWND)lParam);
						}
						break;

				case WM_KEYDOWN:

	     switch(wParam){

								case VK_ESCAPE:
										OnExit();
										PostQuitMessage(0);
										break;

								case VK_SPACE:
										OnSpaceBar();
										break;

								default:
										return DefWindowProc(hWnd, msg, wParam, lParam);
						}
						break;

				case WM_DROPFILES:
						OnDrop(wParam);
						break;

    case WM_CTLCOLORBTN:
						if(m_cMFControlManager.IsChildButton((const HWND)lParam)){

								SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)m_hBackGround;
      }
      break;

    case WM_CTLCOLORSTATIC:
						if(m_cMFControlManager.IsChildColor((const HWND)lParam)){

								SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)m_hBackGround;
      }
      break;

		  case WM_DESTROY:
						OnExit();
						PostQuitMessage(0);
						break;

				case WM_QUERYENDSESSION:
						OnExit();
						return DefWindowProc(hWnd, msg, TRUE, lParam);

				/*case WM_ENDSESSION:
						break;*/

				default:
						return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		
		return 0L;
}

void CMFNodeForm::OnHorizontalScroll(const HWND hWnd){

		int iTrackBar = 0;

		const HWND hChild = m_cMFControlManager.GetChildTrackBar(hWnd, iTrackBar);

		if(hChild == NULL)
				return;

		UINT uiValue;

  if((uiValue = (UINT)SendMessage(hChild, TBM_GETPOS, 0, 0)) == CB_ERR)
				return;

		switch(iTrackBar){

		  case CHILD_TRACKBAR_CONTRAST:
						m_pPlayer->SetVideoContrast((float)uiValue / 50.0f);
				  break;

		  case CHILD_TRACKBAR_SATURATION:
						m_pPlayer->SetVideoSaturation((float)uiValue / 50.0f);
				  break;

		  case CHILD_TRACKBAR_DXVA2_BRIGHTNESS:
						m_pPlayer->SetBrightness(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_HUE:
						m_pPlayer->SetHue(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_CONTRAST:
						m_pPlayer->SetContrast(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_SATURATION:
						m_pPlayer->SetSaturation(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_NOISE:
						m_pPlayer->SetNoise(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_EDGE:
						m_pPlayer->SetEdge(TRUE, (INT)uiValue);
				  break;

		  case CHILD_TRACKBAR_DXVA2_ANAMORPHIC:
						m_pPlayer->SetAnamorphic(TRUE, (INT)uiValue);
				  break;
		}
}

void CMFNodeForm::OnExit(){

		if(m_pPlayer != NULL){

				m_pPlayer->CloseSession();
				// We will never receive SessionClosed event because we are in the message loop.
				// So before calling CloseSession, let some time to the MediaSession to finish.
				// Because we leave the program, it is not a big deal. We should try to use another
				// message loop to handle this correctly.
				Sleep(200);
				CloseSession();
  }

		m_cMFControlManager.Close(m_hWnd);
}

void CMFNodeForm::ResizeVideo(const WORD wWidth, const WORD wHeight){

		if(m_pPlayer == NULL || m_bRepaintVideo == FALSE)
				return;

		m_pPlayer->ResizeVideo(wWidth, wHeight);
}

void CMFNodeForm::OnSpaceBar(){
		
		if(m_pPlayer == NULL)
				return;

		if(m_pPlayer->GetState() == SessionStarted){
				m_pPlayer->Pause();
		}
		else if(m_pPlayer->GetState() == SessionPaused){
				m_pPlayer->Play();
		}
}

void CMFNodeForm::OnPlayerEvent(WPARAM wParam, LPARAM lParam){
		
		BOOL bWaiting = FALSE;
		BOOL bPlayback = FALSE;

		switch(wParam){
		
		  case SessionOpenPending:
						SetWindowText(m_hWnd, L"MFNode Player : opening");
						bWaiting = TRUE;
						break;
		
		  case SessionReady:
						SetWindowText(m_hWnd, L"MFNode Player : ready");						
						if(m_pPlayer != NULL){
								// For future use
								//MFTIME duration = m_pPlayer->GetDuration();
								m_pPlayer->Play();
						}
						EnableSessionControls(FALSE);
						EnableControls(TRUE);
						break;

				case SessionStarted:
						SetWindowText(m_hWnd, L"MFNode Player : started");
						bPlayback = TRUE;

						if(m_pPlayer->GetSession() == SESSION_DXVA2){
								// Sleep because it seems that session can be started, but streamsink not initialized...
								// Have to found a better way to handle this.
								Sleep(1000);
						  UpdateDxva2Bar();
					 }
						// DEBUG
						/*if(m_pPlayer != NULL){
								Sleep(1000);
    				m_pPlayer->CloseSession();
    		}*/
						// DEBUG
						break;

				case SessionPaused:
						SetWindowText(m_hWnd, L"MFNode Player : paused");
						bPlayback = TRUE;
						break;

				case SessionStopped:
						SetWindowText(m_hWnd, L"MFNode Player : stopped");
						bPlayback = TRUE;
						break;

				case SessionClosed:
						SetWindowText(m_hWnd, L"MFNode Player : closed");
						break;

				case SessionAbort:
						SetWindowText(m_hWnd, L"MFNode Player : abort");
						break;
		}

		HCURSOR hCursor = LoadCursor(0, MAKEINTRESOURCE(bWaiting ? IDC_WAIT : IDC_ARROW));
		SetCursor(hCursor);
		m_bWaiting = bWaiting;

		if(wParam == SessionAbort){

				if(m_pPlayer != NULL){
						m_pPlayer->CloseSession();
    }
		}
		else{

				if(bPlayback && lParam){
				  m_bRepaintVideo = TRUE;
		  }
		  else{

				  m_bRepaintVideo = FALSE;

				  if(wParam == SessionClosed){

						  CloseSession();

								// In case where you drag and drop a file while playing another media
								if(m_wszFile1[0] != '\0'){
										
										#ifdef TEST_CUDA_DECODER
										  OpenCudaTestSession(m_wszFile1);
										#elif defined TEST_DXVA2_DECODER
												OpenDxva2TestSession(m_wszFile1);
								  #else
										  OpenFreeSession(m_wszFile1);
										#endif

										m_wszFile1[0] = '\0';
								}

								// DEBUG
								//OpenSession(SESSION_AVCAPTURE);
								// DEBUG
				  }
				}
		}
}

void CMFNodeForm::OnPaint(const HWND hWnd){

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hWnd, &ps);
  
  RECT rc;
  GetClientRect(hWnd, &rc);
  FillRect(hdc, &rc, m_hBackGround);
  
  RECT rcWindow = {50, 10, 200, 50};
  
  SetBkMode(hdc, TRANSPARENT);
  
  DrawText(hdc, L"Wave Session :", 14, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 110;
  rcWindow.bottom += 110;
  DrawText(hdc, L"Flv Session :", 13, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 80;
  rcWindow.bottom += 80;
  DrawText(hdc, L"Renderer Session :", 18, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 135;
  rcWindow.bottom += 135;
  rcWindow.right  += 175;
  rcWindow.left  += 175;
  DrawText(hdc, L"Contrast", 8, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 50;
  rcWindow.bottom += 50;
  DrawText(hdc, L"Saturation", 10, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 105;
  rcWindow.bottom += 105;
  rcWindow.right  -= 175;
  rcWindow.left  -= 175;
  DrawText(hdc, L"Sequencer Session :", 19, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 105;
  rcWindow.bottom += 105;
  DrawText(hdc, L"Dxva2 Session :", 15, &rcWindow, DT_VCENTER | DT_LEFT);
  
  rcWindow.top    += 55;
  rcWindow.bottom += 55;
  rcWindow.left  = 0;
  rcWindow.right  = 100;
  DrawText(hdc, L"Brightness :", 12, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    += 25;
  rcWindow.bottom += 25;
  DrawText(hdc, L"Hue :", 5, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    += 25;
  rcWindow.bottom += 25;
  DrawText(hdc, L"Contrast :", 10, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    += 25;
  rcWindow.bottom += 25;
  DrawText(hdc, L"Saturation :", 12, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    -= 75;
  rcWindow.bottom -= 75;
  rcWindow.left  = 420;
  rcWindow.right  = 570;
  DrawText(hdc, L"NoiseReduction :", 16, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    += 25;
  rcWindow.bottom += 25;
  DrawText(hdc, L"Edge Enhancement :", 18, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  rcWindow.top    += 25;
  rcWindow.bottom += 25;
  DrawText(hdc, L"Anamorphic Scaling :", 20, &rcWindow, DT_VCENTER | DT_RIGHT);
  
  MoveToEx(hdc, 20, 110, NULL);
  LineTo(hdc, rc.right - 20, 110);
  
  MoveToEx(hdc, 20, 190, NULL);
  LineTo(hdc, rc.right - 20, 190);
  
  MoveToEx(hdc, 20, 480, NULL);
  LineTo(hdc, rc.right - 20, 480);
  
  MoveToEx(hdc, 20, 590, NULL);
  LineTo(hdc, rc.right - 20, 590);
  
  EndPaint(hWnd, &ps);
}

BOOL CMFNodeForm::OnCommand(const DWORD dwCmd){

		BOOL bNoCmd = FALSE;

		switch(dwCmd){

				case ID_BUTTON1:
      AddMediaFileToEdit(CHILD_EDIT_WAVE1);
						break;

    case ID_BUTTON2:
    		AddMediaFileToEdit(CHILD_EDIT_WAVE2);
    		break;
    
    case ID_BUTTON3:
    		AddMediaFileToEdit(CHILD_EDIT_FLV);
    		break;
    
    case ID_BUTTON4:
    		AddMediaFileToEdit(CHILD_EDIT_RENDERER);
    		break;
    
    case ID_BUTTON5:
    		AddMediaFileToEdit(CHILD_EDIT_VIDEO1);
    		break;
    
    case ID_BUTTON6:
    		AddMediaFileToEdit(CHILD_EDIT_VIDEO2);
    		break;
    
    case ID_BUTTON7:
    		AddMediaFileToEdit(CHILD_EDIT_MPEG);
    		break;
    
    case ID_FILE_EXIT:
    		OnExit();
    		PostQuitMessage(0);
    		break;

    case ID_OPEN_WAVEMIXER:
    		OpenWaveMixer();
    		break;
    
    case ID_OPEN_SCREEN:
    		OpenSession(SESSION_SCREENSHOT);
    		break;
    
    case ID_OPEN_HTTP:
    		OpenSession(SESSION_HTTP);
    		break;
    
    case ID_OPEN_FLV:
    		OpenSession(SESSION_FLV);
    		break;
    
    case ID_OPEN_RENDERER:
    		OpenSession(SESSION_RENDERER);
    		break;
    
    case ID_OPEN_SEQUENCER:
    		OpenSequencer();
    		break;
    
    case ID_OPEN_AVCAPTURE:
    		OpenSession(SESSION_AVCAPTURE);
    		break;
    
    case ID_OPEN_DXVA2:
    		OpenSession(SESSION_DXVA2);
    		break;
    
    case ID_CLOSE_SESSION:
    		if(m_pPlayer != NULL){
    				m_pPlayer->CloseSession();
    		}
    		break;

    case ID_CONTROL_PLAY:
    		if(m_pPlayer != NULL){
    				m_pPlayer->Play();
    		}
    		break;
    
    case ID_CONTROL_PAUSE:
    		if(m_pPlayer != NULL){
    				m_pPlayer->Pause();
    		}
    		break;
    
    case ID_CONTROL_STOP:
    		if(m_pPlayer != NULL){
    				m_pPlayer->Stop();
    		}
    		break;
    
    case ID_CHECKBOX_REVERSEX:
    		if(m_pPlayer != NULL){
    				m_pPlayer->SetVideoReverseX(SendMessage(m_cMFControlManager.GetChild(CHILD_CHECK_REVERSE_X), BM_GETCHECK, 0, 0));
    		}
    		break;
    
    case ID_CHECKBOX_REVERSEY:
    		if(m_pPlayer != NULL){
    				m_pPlayer->SetVideoReverseY(SendMessage(m_cMFControlManager.GetChild(CHILD_CHECK_REVERSE_Y), BM_GETCHECK, 0, 0));
    		}
    		break;
    
    case ID_CHECKBOX_NEGATIF:
    		if(m_pPlayer != NULL){
    				m_pPlayer->SetVideoNegatif(SendMessage(m_cMFControlManager.GetChild(CHILD_CHECK_NEGATIF), BM_GETCHECK, 0, 0));
    		}
    		break;
    
    case ID_CHECKBOX_GRAY:
    		if(m_pPlayer != NULL){
    				m_pPlayer->SetVideoGray(SendMessage(m_cMFControlManager.GetChild(CHILD_CHECK_GRAY), BM_GETCHECK, 0, 0));
    		}
    		break;

    case ID_CHECKBOX_GRAYSCALE:
    		if(m_pPlayer != NULL){
    				m_pPlayer->SetVideoGrayScale(SendMessage(m_cMFControlManager.GetChild(CHILD_CHECK_GRAYSCALE), BM_GETCHECK, 0, 0));
    		}
    		break;
    
    case ID_RADIO_ORIGINAL:
    		if(m_pPlayer != NULL){
    
    				if(SendMessage(m_cMFControlManager.GetChild(CHILD_RADIO_COLOR), BM_GETCHECK, 0, 0))
    						m_pPlayer->SetVideoColor(SHADER_COLOR_ORIGINAL);
    		}
    		break;
    
    case ID_RADIO_RED:
    		if(m_pPlayer != NULL){
    
    				if(SendMessage(m_cMFControlManager.GetChild(CHILD_RADIO_RED), BM_GETCHECK, 0, 0))
    						m_pPlayer->SetVideoColor(SHADER_COLOR_RED);
    		}
    		break;
    
    case ID_RADIO_GREEN:
    		if(m_pPlayer != NULL){
    
    				if(SendMessage(m_cMFControlManager.GetChild(CHILD_RADIO_GREEN), BM_GETCHECK, 0, 0))
    						m_pPlayer->SetVideoColor(SHADER_COLOR_GREEN);
    		}
    		break;
    
    case ID_RADIO_BLUE:
    		if(m_pPlayer != NULL){
    
    				if(SendMessage(m_cMFControlManager.GetChild(CHILD_RADIO_BLUE), BM_GETCHECK, 0, 0))
    						m_pPlayer->SetVideoColor(SHADER_COLOR_BLUE);
    		}
    		break;

				default:
						bNoCmd = TRUE;
    		break;
  }

		return bNoCmd;
}

void CMFNodeForm::OnDrop(WPARAM wParam){

		HDROP hDrop = (HDROP)wParam;
		UINT uiFile = 0;

		uiFile = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);

		if(uiFile != 1){
				
				// Just one file at a time
				DragFinish(hDrop);
				return;
		}

		UINT uiCount = DragQueryFile(hDrop, 0, NULL, NULL);
		WCHAR* pwszFileName = NULL;

		if(uiCount != 0 && uiCount < MAX_PATH){

				UINT uiSize = MAX_PATH * sizeof(WCHAR);

				pwszFileName = new WCHAR[uiSize];

				if(DragQueryFile(hDrop, 0, pwszFileName, uiSize)){

						#ifdef TEST_CUDA_DECODER
						  OpenCudaTestSession(pwszFileName);
						#elif defined		TEST_DXVA2_DECODER
						  OpenDxva2TestSession(pwszFileName);
      #else
						  OpenFreeSession(pwszFileName);
						#endif
				}
		}

		DragFinish(hDrop);

		if(pwszFileName){
				delete[] pwszFileName;
				pwszFileName = NULL;
		}
}

#ifdef TEST_CUDA_DECODER
HRESULT CMFNodeForm::OpenCudaTestSession(const WCHAR* wszFile){

		HRESULT hr = S_OK;
		
		if(m_pPlayer != NULL){
				
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

		hr = m_pPlayer->OpenCudaRenderer(wszFile);

		if(FAILED(hr)){
				LOG_HRESULT(MFShutdown());
		}

		return hr;
}
#elif defined TEST_DXVA2_DECODER
HRESULT CMFNodeForm::OpenDxva2TestSession(const WCHAR* wszFile){

		HRESULT hr = S_OK;
		
		if(m_pPlayer != NULL){
				
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

		hr = m_pPlayer->OpenDxva2Decoder(wszFile);

		if(FAILED(hr)){
				LOG_HRESULT(MFShutdown());
		}

		return hr;
}
#endif