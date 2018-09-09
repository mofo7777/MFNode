//----------------------------------------------------------------------------------------------
// Dxva2Manager.h
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
#ifndef DXVA2MANAGER_H
#define DXVA2MANAGER_H

class CDxva2Manager{

public:

	CDxva2Manager();
	~CDxva2Manager(){ ReleaseDxva2(); }

	HRESULT InitDxva2(const UINT32, const UINT32, const UINT32, const D3DFORMAT);
	void ReleaseDxva2();
	HRESULT OnFrameMove(IMFSample*);
	HRESULT OnFrameRender();

	HRESULT SetProcAmp(const DXVAHD_FILTER, const BOOL, const INT);
	HRESULT GetProcAmp(const DXVAHD_FILTER, BOOL*, INT*, INT*, INT*);

	static DWORD WINAPI StaticThreadProc(LPVOID);

private:

	IDXVAHD_Device * m_pDXVAHD;
	IDXVAHD_VideoProcessor* m_pDXVAVP;

	IDirect3D9Ex*       m_pD3D9;
	IDirect3DDevice9Ex* m_pDevice9;
	IDirect3DSurface9*  m_pSurface9;

	HWND m_hWnd;
	UINT m_uiFrameIndex;

	UINT32 m_uiWidth;
	UINT32 m_uiHeight;
	UINT32 m_uiStride;
	UINT32 m_uiYuvHeight;
	D3DFORMAT m_VideoFmt;

	HANDLE m_hRendererThread;
	HANDLE m_hThreadReadyEvent;
	DWORD m_dwThreadID;

	struct PROCAMP{

		DXVAHD_FILTER_RANGE_DATA Range;
		DXVAHD_STREAM_STATE_FILTER_DATA FilterState;
		DXVAHD_FILTER Filter;
		DXVAHD_STREAM_STATE Stream;
		BOOL bSupported;
	};

	PROCAMP m_ProcAmp[7];

	BOOL InitWindow(const UINT32, const UINT32);
	HRESULT InitDevice9();
	HRESULT InitVideoProcessor();
	void GetProcAmpData(const DXVAHD_VPDEVCAPS&);
	HRESULT ConfigureVideoProcessor();
	HRESULT FillTextureWithBlack();
};

#endif