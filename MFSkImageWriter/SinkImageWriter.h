//----------------------------------------------------------------------------------------------
// SinkImageWriter.h
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
#ifndef MFSKIMAGEWRITER_H
#define MFSKIMAGEWRITER_H

class CSkinkImageWriter : BaseObject, public IMFMediaSink, public IMFClockStateSink{

public:

	// SinkImageWriter.cpp
	static HRESULT CreateInstance(IUnknown*, REFIID, void**);

	// IUnknown - SinkImageWriter.cpp
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaSink - SinkImageWriter_Sink.cpp
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP AddStreamSink(DWORD, IMFMediaType*, IMFStreamSink**);
	STDMETHODIMP RemoveStreamSink(DWORD);
	STDMETHODIMP GetStreamSinkCount(DWORD*);
	STDMETHODIMP GetStreamSinkByIndex(DWORD, IMFStreamSink**);
	STDMETHODIMP GetStreamSinkById(DWORD, IMFStreamSink**);
	STDMETHODIMP SetPresentationClock(IMFPresentationClock*);
	STDMETHODIMP GetPresentationClock(IMFPresentationClock**);
	STDMETHODIMP Shutdown();

	// IMFClockStateSink - SinkImageWriter_Clock.cpp
	STDMETHODIMP OnClockStart(MFTIME, LONGLONG);
	STDMETHODIMP OnClockStop(MFTIME);
	STDMETHODIMP OnClockPause(MFTIME);
	STDMETHODIMP OnClockRestart(MFTIME);
	STDMETHODIMP OnClockSetRate(MFTIME, float);

private:

	// SinkImageWriter.cpp
	CSkinkImageWriter(HRESULT&);
	virtual ~CSkinkImageWriter();

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	BOOL m_IsShutdown;

	CStreamImage* m_pStreamImage;
	IMFPresentationClock* m_pPresentationClock;
	IMFClock* m_pClock;
	MFCLOCK_STATE m_ClockState;

	HRESULT CheckShutdown() const{ return (m_IsShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif