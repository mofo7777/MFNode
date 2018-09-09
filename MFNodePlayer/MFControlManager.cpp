//----------------------------------------------------------------------------------------------
// MFControlManager.cpp
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

CMFControlManager::CMFControlManager(){

	for(int i = 0; i < CHILD_END; i++){
		m_hChilds[i] = NULL;
	}
}

HRESULT CMFControlManager::InitChilds(const HINSTANCE hInst, const HWND hWnd){

	HRESULT hr;

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_WAVE1], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 40, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_WAVE2], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 70, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_FLV], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 150, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_RENDERER], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 230, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_VIDEO1], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 520, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_VIDEO2], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 550, 800, 20, hWnd, NULL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_MPEG], WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 620, 800, 20, hWnd, NULL, hInst));

	// If you don't wan't to spend your time to select file, use this (and change path/filename)
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_WAVE1], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\Wav\\file1.wav", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 40, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_WAVE2], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\Wav\\file2.wav", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 70, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_FLV], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\\Flv\\video.flv", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 150, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_RENDERER], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\\Mpeg2\\video.mpeg", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 230, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_VIDEO1], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\Mp4\\video1.mp4", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 520, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_VIDEO2], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\project\\media\\Mp4\\video2.mp4", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 550, 800, 20, hWnd, NULL, hInst));
	//IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_EDIT_MPEG], WS_EX_CLIENTEDGE, L"EDIT", L"C:\\projet\\media\\Mpeg2\\video.mpg", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 50, 620, 800, 20, hWnd, NULL, hInst));

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_WAVE1], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 38, 50, 24, hWnd, ID_BUTTON1, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_WAVE2], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 68, 50, 24, hWnd, ID_BUTTON2, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_FLV], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 148, 50, 24, hWnd, ID_BUTTON3, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_RENDERER], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 228, 50, 24, hWnd, ID_BUTTON4, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_VIDEO1], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 518, 50, 24, hWnd, ID_BUTTON5, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_VIDEO2], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 548, 50, 24, hWnd, ID_BUTTON6, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_BUTTON_MPEG], 0, L"BUTTON", L"Select", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 870, 618, 50, 24, hWnd, ID_BUTTON7, hInst));

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_GROUPBOX], WS_EX_WINDOWEDGE, L"BUTTON", L"Video Shader Effect", WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 50, 270, 800, 190, hWnd, NULL, hInst));

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_CHECK_REVERSE_X], NULL, L"BUTTON", L"Reverse X", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 80, 300, 100, 20, hWnd, ID_CHECKBOX_REVERSEX, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_CHECK_REVERSE_Y], NULL, L"BUTTON", L"Reverse Y", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 190, 300, 100, 20, hWnd, ID_CHECKBOX_REVERSEY, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_CHECK_NEGATIF], NULL, L"BUTTON", L"Negatif", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 300, 300, 100, 20, hWnd, ID_CHECKBOX_NEGATIF, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_CHECK_GRAY], NULL, L"BUTTON", L"Gray", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 410, 300, 100, 20, hWnd, ID_CHECKBOX_GRAY, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_CHECK_GRAYSCALE], NULL, L"BUTTON", L"GrayScale", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 520, 300, 100, 20, hWnd, ID_CHECKBOX_GRAYSCALE, hInst));

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_COLOR], WS_EX_WINDOWEDGE, L"BUTTON", L"Video original", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 80, 340, 120, 20, hWnd, ID_RADIO_ORIGINAL, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_RED], WS_EX_WINDOWEDGE, L"BUTTON", L"Video red", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 80, 365, 120, 20, hWnd, ID_RADIO_RED, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_GREEN], WS_EX_WINDOWEDGE, L"BUTTON", L"Video green", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 80, 390, 120, 20, hWnd, ID_RADIO_GREEN, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_BLUE], WS_EX_WINDOWEDGE, L"BUTTON", L"Video blue", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 80, 415, 120, 20, hWnd, ID_RADIO_BLUE, hInst));

	SendMessage(m_hChilds[CHILD_RADIO_COLOR], BM_SETCHECK, BST_CHECKED, 0);

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_NOCUDA], WS_EX_WINDOWEDGE, L"BUTTON", L"No Cuda Decoder", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, 650, 355, 150, 20, hWnd, ID_RADIO_NOCUDA, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_RADIO_CUDA], WS_EX_WINDOWEDGE, L"BUTTON", L"Cuda Decoder", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 650, 380, 150, 20, hWnd, ID_RADIO_CUDA, hInst));

	SendMessage(m_hChilds[CHILD_RADIO_NOCUDA], BM_SETCHECK, BST_CHECKED, 0);

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_CONTRAST], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 220, 355, 400, 20, hWnd, ID_TCKBAR_CONTRAST, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_SATURATION], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 220, 405, 400, 20, hWnd, ID_TCKBAR_SATURATION, hInst));

	SendMessage(m_hChilds[CHILD_TRACKBAR_CONTRAST], TBM_SETPOS, (WPARAM)TRUE, (LPARAM)50);
	SendMessage(m_hChilds[CHILD_TRACKBAR_SATURATION], TBM_SETPOS, (WPARAM)TRUE, (LPARAM)50);

	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_BRIGHTNESS], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 120, 650, 300, 20, hWnd, ID_TCKBAR_DXVA2_BRIGHTNESS, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_HUE], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 120, 675, 300, 20, hWnd, ID_TCKBAR_DXVA2_HUE, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_CONTRAST], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 120, 700, 300, 20, hWnd, ID_TCKBAR_DXVA2_CONTRAST, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_SATURATION], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 120, 725, 300, 20, hWnd, ID_TCKBAR_DXVA2_SATURATION, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_NOISE], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 580, 650, 300, 20, hWnd, ID_TCKBAR_DXVA2_NOISE, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_EDGE], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 580, 675, 300, 20, hWnd, ID_TCKBAR_DXVA2_EDGE, hInst));
	IF_FAILED_RETURN(hr = CreateChild(m_hChilds[CHILD_TRACKBAR_DXVA2_ANAMORPHIC], WS_EX_WINDOWEDGE, TRACKBAR_CLASS, NULL, WS_VISIBLE | WS_CHILD | TBS_HORZ | WS_EX_OVERLAPPEDWINDOW | TBS_BOTTOM, 580, 700, 300, 20, hWnd, ID_TCKBAR_DXVA2_ANAMORPHIC, hInst));

	return hr;
}

void CMFControlManager::Close(const HWND hWnd){

	for(int i = 0; i < CHILD_END; i++){

		if(IsChild(hWnd, m_hChilds[i])){
			DestroyWindow(m_hChilds[i]);
			m_hChilds[i] = NULL;
		}
	}
}