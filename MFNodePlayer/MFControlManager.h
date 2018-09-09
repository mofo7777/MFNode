//----------------------------------------------------------------------------------------------
// MFControlManager.h
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
#ifndef MFCONTROLMANAGER_H
#define MFCONTROLMANAGER_H

class CMFControlManager{

public:

	CMFControlManager();
	~CMFControlManager(){}

	HRESULT InitChilds(const HINSTANCE, const HWND);
	void Close(const HWND);

	HWND GetChild(const int iChildIndex) const{ assert(iChildIndex < CHILD_END); return m_hChilds[iChildIndex]; }
	const HWND GetChildTrackBar(const HWND, int&) const;

	BOOL IsChildButton(const HWND) const;
	BOOL IsChildColor(const HWND) const;

private:

	HWND m_hChilds[CHILD_END];

	HRESULT CreateChild(HWND&, DWORD, LPCWSTR, LPCWSTR, DWORD, int, int, int, int, HWND, int, HINSTANCE);
};

inline const HWND CMFControlManager::GetChildTrackBar(const HWND hWnd, int& pTrackBar) const{

	HWND hChild = NULL;

	for(int i = CHILD_TRACKBAR_CONTRAST; i < CHILD_END; i++){

		if(hWnd == m_hChilds[i]){

			hChild = m_hChilds[i];
			pTrackBar = i;
			break;
		}
	}

	return hChild;
}

inline BOOL CMFControlManager::IsChildButton(const HWND hWnd) const{

	BOOL bIsChild = FALSE;

	for(int i = CHILD_BUTTON_WAVE1; i < CHILD_GROUPBOX; i++){

		if(hWnd == m_hChilds[i]){

			bIsChild = TRUE;
			break;
		}
	}

	return bIsChild;
}

inline BOOL CMFControlManager::IsChildColor(const HWND hWnd) const{

	BOOL bIsChild = FALSE;

	for(int i = CHILD_GROUPBOX; i < CHILD_END; i++){

		if(hWnd == m_hChilds[i]){

			bIsChild = TRUE;
			break;
		}
	}

	return bIsChild;
}

inline HRESULT CMFControlManager::CreateChild(HWND& hChild, DWORD dwExStyle, LPCWSTR wszClass, LPCWSTR wszName, DWORD dwStyle,
	int iX, int iY, int iWidth, int iHeight, HWND hParent, int iMenu, HINSTANCE hInst){

	hChild = CreateWindowEx(dwExStyle, wszClass, wszName, dwStyle, iX, iY, iWidth, iHeight, hParent, (HMENU)iMenu, hInst, NULL);

	if(hChild == NULL)
		return E_FAIL;

	return S_OK;
}

#endif