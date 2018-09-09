//----------------------------------------------------------------------------------------------
// MFNodeForm.h
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
#ifndef MFNODEFORM_H
#define MFNODEFORM_H

class CMFNodeForm{

  public:

    CMFNodeForm();
    ~CMFNodeForm();

				// MFNodeForm.cpp
				HRESULT InitForm(const HINSTANCE);
				// MFNodeForm_Event.cpp
				LRESULT CALLBACK MFNodeFormProc(HWND, UINT, WPARAM, LPARAM);

				// inline
				static CMFNodeForm* GetMFNodeForm(){ return m_pCMFNodeForm; }

  private:

				static CMFNodeForm* m_pCMFNodeForm;
				CMFNodePlayer* m_pPlayer;
				CMFControlManager m_cMFControlManager;
				
				HINSTANCE m_hInst;
    HWND      m_hWnd;
				BOOL      m_bRepaintVideo;
				BOOL      m_bWaiting;
				BOOL      m_bSetCursor;
				HBRUSH    m_hBackGround;

				WCHAR m_wszFile1[MAX_PATH];

				// MFNodeForm.cpp
				HRESULT OpenWaveMixer();
				HRESULT OpenSession(const CURRENT_SESSION);
				HRESULT OpenSequencer();
				HRESULT OpenFreeSession(const WCHAR*);
				HRESULT CloseSession();
				void AddMediaFileToEdit(const int);
				void EnableSessionControls(const BOOL);
				void EnableControls(const BOOL);
				BOOL CheckWaveFile(const WCHAR*, const WCHAR*);
				void MFNodeCheck();
				void UpdateDxva2Bar();

				// MFNodeForm_Event.cpp
				void OnExit();
				void OnHorizontalScroll(const HWND);
				void ResizeVideo(const WORD, const WORD);
				void OnSpaceBar();
				void OnPlayerEvent(WPARAM, LPARAM);
				void OnPaint(const HWND);
				BOOL OnCommand(const DWORD);
				void OnDrop(WPARAM);

				#ifdef TEST_CUDA_DECODER
				  HRESULT OpenCudaTestSession(const WCHAR*);
				#elif defined TEST_DXVA2_DECODER
				  HRESULT OpenDxva2TestSession(const WCHAR*);
				#endif
};

#endif