//----------------------------------------------------------------------------------------------
// MFSkJpegHttpStreamer.h
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
#ifndef MFSKJPEGHTTPSTREAMER_H
#define MFSKJPEGHTTPSTREAMER_H

class CMFSkJpegHttpStreamer : BaseObject, public IMFMediaSink, public IMFClockStateSink{

public:

	// MFSkJpegHttpStreamer.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - MFSkJpegHttpStreamer.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaSink - MFSkJpegHttpStreamer_Sink.cpp
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP AddStreamSink(DWORD, IMFMediaType*, IMFStreamSink**);
	STDMETHODIMP RemoveStreamSink(DWORD);
	STDMETHODIMP GetStreamSinkCount(DWORD*);
	STDMETHODIMP GetStreamSinkByIndex(DWORD, IMFStreamSink**);
	STDMETHODIMP GetStreamSinkById(DWORD, IMFStreamSink**);
	STDMETHODIMP SetPresentationClock(IMFPresentationClock*);
	STDMETHODIMP GetPresentationClock(IMFPresentationClock**);
	STDMETHODIMP Shutdown();

	// IMFClockStateSink - MFSkJpegHttpStreamer_Clock.cpp
	STDMETHODIMP OnClockStart(MFTIME, LONGLONG);
	STDMETHODIMP OnClockStop(MFTIME);
	STDMETHODIMP OnClockPause(MFTIME);
	STDMETHODIMP OnClockRestart(MFTIME);
	STDMETHODIMP OnClockSetRate(MFTIME, float);

private:

	// MFSkJpegHttpStreamer.cpp
	CMFSkJpegHttpStreamer(HRESULT&);
	virtual ~CMFSkJpegHttpStreamer();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	BOOL m_IsShutdown;

	CMFSkStreamJpeg* m_pJpegStream;
	IMFPresentationClock* m_pClock;

	HRESULT CheckShutdown() const{ return (m_IsShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif
