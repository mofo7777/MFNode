//----------------------------------------------------------------------------------------------
// SinkVideoRenderer.h
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
#ifndef SINKVIDEORENDERER_H
#define SINKVIDEORENDERER_H

class CSinkVideoRenderer
	: BaseObject,
	public IMFMediaSink,
	public IMFClockStateSink,
	public IMFVideoDisplayControl,
	public IMFMediaSinkPreroll,
	public IMFVideoShaderEffect
{

public:

	// SinkVideoRenderer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - SinkVideoRenderer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaSink - SinkVideoRenderer_Sink.cpp
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP AddStreamSink(DWORD, IMFMediaType*, IMFStreamSink**);
	STDMETHODIMP RemoveStreamSink(DWORD);
	STDMETHODIMP GetStreamSinkCount(DWORD*);
	STDMETHODIMP GetStreamSinkByIndex(DWORD, IMFStreamSink**);
	STDMETHODIMP GetStreamSinkById(DWORD, IMFStreamSink**);
	STDMETHODIMP SetPresentationClock(IMFPresentationClock*);
	STDMETHODIMP GetPresentationClock(IMFPresentationClock**);
	STDMETHODIMP Shutdown();

	// IMFClockStateSink - SinkVideoRenderer_Clock.cpp
	STDMETHODIMP OnClockStart(MFTIME, LONGLONG);
	STDMETHODIMP OnClockStop(MFTIME);
	STDMETHODIMP OnClockPause(MFTIME);
	STDMETHODIMP OnClockRestart(MFTIME);
	STDMETHODIMP OnClockSetRate(MFTIME, float);

	// IMFVideoDisplayControl - SinkVideoRenderer_Display.cpp
	STDMETHOD(GetNativeVideoSize)(SIZE*, SIZE*){ return E_NOTIMPL; }
	STDMETHOD(GetIdealVideoSize)(SIZE*, SIZE*){ return E_NOTIMPL; }
	STDMETHOD(SetVideoPosition)(const MFVideoNormalizedRect*, const LPRECT){ return E_NOTIMPL; }
	STDMETHOD(GetVideoPosition)(MFVideoNormalizedRect*, LPRECT){ return E_NOTIMPL; }
	STDMETHOD(SetAspectRatioMode)(DWORD){ return E_NOTIMPL; }
	STDMETHOD(GetAspectRatioMode)(DWORD*){ return E_NOTIMPL; }
	STDMETHOD(SetVideoWindow)(HWND){ return E_NOTIMPL; }
	STDMETHOD(GetVideoWindow)(HWND*){ return E_NOTIMPL; }
	STDMETHOD(RepaintVideo)(){ return E_NOTIMPL; }
	STDMETHOD(GetCurrentImage)(BITMAPINFOHEADER*, BYTE**, DWORD*, LONGLONG*){ return E_NOTIMPL; }
	STDMETHOD(SetBorderColor)(COLORREF){ return E_NOTIMPL; }
	STDMETHOD(GetBorderColor)(COLORREF*){ return E_NOTIMPL; }
	STDMETHOD(SetRenderingPrefs)(DWORD){ return E_NOTIMPL; }
	STDMETHOD(GetRenderingPrefs)(DWORD*){ return E_NOTIMPL; }
	STDMETHOD(SetFullscreen)(BOOL){ return E_NOTIMPL; }
	STDMETHOD(GetFullscreen)(BOOL*){ return E_NOTIMPL; }

	// IMFVideoShaderEffect - SinkVideoRenderer.cpp
	STDMETHOD(SetShaderReverseX)(BOOL);
	STDMETHOD(SetShaderReverseY)(BOOL);
	STDMETHOD(SetShaderNegatif)(BOOL);
	STDMETHOD(SetShaderGray)(BOOL);
	STDMETHOD(SetShaderGrayScale)(BOOL);
	STDMETHOD(SetShaderColor)(UINT);
	STDMETHOD(SetShaderContrast)(FLOAT);
	STDMETHOD(SetShaderSaturation)(FLOAT);

	// IMFMediaSinkPreroll - SinkVideoRenderer.cpp
	STDMETHODIMP NotifyPreroll(MFTIME);

	// SinkVideoRenderer.cpp
	HRESULT ProcessSample(IMFSample*);

	// SinkVideoRenderer_Thread.cpp
	static DWORD WINAPI StaticThreadProc(LPVOID);

private:

	// SinkVideoRenderer.cpp
	CSinkVideoRenderer(HRESULT&);
	virtual ~CSinkVideoRenderer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	BOOL m_bShutdown;

	CStreamVideoRenderer* m_pStreamVideoRenderer;
	IMFPresentationClock* m_pClock;

	CDirectX9Manager m_cDirectX9Manager;
	CMFSafeQueue m_cMFSafeQueue;

	BOOL m_bPreroll;
	LONGLONG m_PerFrame_1_4th;

	HANDLE m_hRendererThread;
	HANDLE m_hThreadReadyEvent;
	DWORD m_dwThreadID;

	enum RendererEvent{ RENDERER_EXIT = WM_USER, RENDERER_PLAY = WM_USER + 1, RENDERER_PAUSE = WM_USER + 2 };

	// SinkVideoRenderer.cpp
	HRESULT GetVideoAttribute(UINT32*, UINT32*);

	// SinkVideoRenderer_Thread.cpp
	HRESULT StartRenderer();
	HRESULT StopRenderer();
	DWORD RendererThreadProc();
	BOOL IsTimeToPresentSample(IMFSample*, LONG&);

	// Inline
	HRESULT CheckShutdown() const{ return (m_bShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif
