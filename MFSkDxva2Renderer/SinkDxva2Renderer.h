//----------------------------------------------------------------------------------------------
// SinkDxva2Renderer.h
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
#ifndef SINKDXVA2RENDERER_H
#define SINKDXVA2RENDERER_H

class CSinkDxva2Renderer
	: BaseObject,
	public IMFMediaSink,
	public IMFClockStateSink,
	public IMFMediaSinkPreroll,
	public IMFDxva2RendererSettings
{

public:

	// SinkDxva2Renderer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - SinkDxva2Renderer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaSink - SinkDxva2Renderer_Sink.cpp
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP AddStreamSink(DWORD, IMFMediaType*, IMFStreamSink**);
	STDMETHODIMP RemoveStreamSink(DWORD);
	STDMETHODIMP GetStreamSinkCount(DWORD*);
	STDMETHODIMP GetStreamSinkByIndex(DWORD, IMFStreamSink**);
	STDMETHODIMP GetStreamSinkById(DWORD, IMFStreamSink**);
	STDMETHODIMP SetPresentationClock(IMFPresentationClock*);
	STDMETHODIMP GetPresentationClock(IMFPresentationClock**);
	STDMETHODIMP Shutdown();

	// IMFClockStateSink - SinkDxva2Renderer_Clock.cpp
	STDMETHODIMP OnClockStart(MFTIME, LONGLONG);
	STDMETHODIMP OnClockStop(MFTIME);
	STDMETHODIMP OnClockPause(MFTIME);
	STDMETHODIMP OnClockRestart(MFTIME);
	STDMETHODIMP OnClockSetRate(MFTIME, float);

	// IMFDxva2RendererSettings - SinkDxva2Renderer_Settings.cpp
	STDMETHODIMP SetBrightness(BOOL, INT);
	STDMETHODIMP GetBrightness(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetContrast(BOOL, INT);
	STDMETHODIMP GetContrast(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetHue(BOOL, INT);
	STDMETHODIMP GetHue(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetSaturation(BOOL, INT);
	STDMETHODIMP GetSaturation(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetNoiseReduction(BOOL, INT);
	STDMETHODIMP GetNoiseReduction(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetEdgeEnhancement(BOOL, INT);
	STDMETHODIMP GetEdgeEnhancement(BOOL*, INT*, INT*, INT*);
	STDMETHODIMP SetAnamorphicScaling(BOOL, INT);
	STDMETHODIMP GetAnamorphicScaling(BOOL*, INT*, INT*, INT*);

	// IMFMediaSinkPreroll - SinkDxva2Renderer.cpp
	STDMETHODIMP NotifyPreroll(MFTIME);

	// SinkDxva2Renderer.cpp
	HRESULT ProcessSample(IMFSample*);

	// SinkDxva2Renderer_Thread.cpp
	static DWORD WINAPI StaticThreadProc(LPVOID);

private:

	// SinkDxva2Renderer.cpp
	CSinkDxva2Renderer(HRESULT&);
	virtual ~CSinkDxva2Renderer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	BOOL m_bShutdown;

	CStreamDxva2Renderer* m_pStreamDxva2Renderer;
	IMFPresentationClock* m_pClock;

	CDxva2Manager m_cDxva2Manager;
	CMFSafeQueue m_cMFSafeQueue;

	BOOL m_bPreroll;
	LONGLONG m_PerFrame_1_4th;

	HANDLE m_hRendererThread;
	HANDLE m_hThreadReadyEvent;
	DWORD m_dwThreadID;

	enum RendererEvent{ RENDERER_EXIT = WM_USER, RENDERER_PLAY = WM_USER + 1, RENDERER_PAUSE = WM_USER + 2 };

	// SinkDxva2Renderer.cpp
	HRESULT GetVideoAttribute(UINT32*, UINT32*, UINT32*, D3DFORMAT*);

	// SinkDxva2Renderer_Thread.cpp
	HRESULT StartRenderer();
	HRESULT StopRenderer();
	DWORD RendererThreadProc();
	BOOL IsTimeToPresentSample(IMFSample*, LONG&);

	// Inline
	HRESULT CheckShutdown() const{ return (m_bShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif