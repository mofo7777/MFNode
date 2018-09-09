//----------------------------------------------------------------------------------------------
// DirectX9Manager.h
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
#ifndef DIRECTX9MANAGER_H
#define DIRECTX9MANAGER_H

class CDirectX9Manager{

		public:

				CDirectX9Manager();
				~CDirectX9Manager(){ ReleaseDirectX9(); }

				HRESULT InitDirectX9(const UINT32, const UINT32);
				void ReleaseDirectX9();
				HRESULT OnFrameMove(IMFSample*);
				HRESULT OnFrameRender();

				static DWORD WINAPI StaticThreadProc(LPVOID);

				void SetShaderReverseX(const BOOL bReverseX){ m_bReverseX = bReverseX; m_bSetbReverseX = TRUE; }
				void SetShaderReverseY(const BOOL bReverseY){ m_bReverseY = bReverseY; m_bSetbReverseY = TRUE; }
				void SetShaderNegatif(const BOOL bNegatif){ m_bNegatif = bNegatif; m_bSetNegatif = TRUE; }
				void SetShaderGray(const BOOL bGray){ m_bGray = bGray; m_bSetGray = TRUE; }
				void SetShaderGrayScale(const BOOL bGray){ m_bGrayScale = bGray; m_bSetGrayScale = TRUE; }
				void SetVideoColor(const UINT uiColorMode){ m_uiColorMode = uiColorMode; m_bSetColorMode = TRUE; }
				void SetVideoContrast(const FLOAT fContrast){ m_fContrast = fContrast; m_bSetContrast = TRUE; }
				void SetVideoSaturation(const FLOAT fSaturation){ m_fSaturation = fSaturation; m_bSetSaturation = TRUE; }

		private:

				IDirect3D9* m_pD3D9;
				IDirect3DDevice9* m_pDevice9;
				IDirect3DTexture9* m_pVideoTextureY;
				IDirect3DTexture9* m_pVideoTextureU;
				IDirect3DTexture9* m_pVideoTextureV;
				ID3DXEffect* m_pVideoEffect;

				CQuad m_cVideoFrame;

				HWND m_hWnd;

				UINT32 m_uiWidth;
				UINT32 m_uiHeight;

				HANDLE m_hRendererThread;
    HANDLE m_hThreadReadyEvent;
				DWORD m_dwThreadID;

				BOOL m_bReverseX;
				BOOL m_bSetbReverseX;
				BOOL m_bReverseY;
				BOOL m_bSetbReverseY;
				BOOL m_bNegatif;
				BOOL m_bSetNegatif;
				BOOL m_bGray;
				BOOL m_bSetGray;
				BOOL m_bGrayScale;
				BOOL m_bSetGrayScale;
				UINT m_uiColorMode;
				BOOL m_bSetColorMode;
				FLOAT m_fContrast;
				BOOL m_bSetContrast;
				FLOAT m_fSaturation;
				BOOL m_bSetSaturation;

				BOOL InitWindow(const UINT32, const UINT32);
				HRESULT InitWorldViewProj(const UINT32, const UINT32);
				HRESULT FillTextureForBlack();
};

#endif