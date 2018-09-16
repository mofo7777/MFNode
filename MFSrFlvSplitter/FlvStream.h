//----------------------------------------------------------------------------------------------
// FlvStream.h
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
#ifndef FLVSTREAM_H
#define FLVSTREAM_H

class CFlvStream : public IMFMediaStream{

public:

	static HRESULT CreateInstance(CFlvStream**, CFlvSource*, IMFStreamDescriptor*, const DWORD, HRESULT&);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaStream
	STDMETHODIMP GetMediaSource(IMFMediaSource**);
	STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor**);
	STDMETHODIMP RequestSample(IUnknown*);

	// FlvStream.cpp
	HRESULT Shutdown();
	HRESULT DeliverSample(IMFSample*);

	// Inline
	void Activate(const BOOL);
	const BOOL IsActive() const{ return m_bActive; }

private:

	CFlvStream(CFlvSource*, IMFStreamDescriptor*, const DWORD dwID, HRESULT&);
	virtual ~CFlvStream(){ TRACE_STREAM((L"Stream::DTOR")); Shutdown(); }

	CriticSection m_CriticSection;
	volatile long m_nRefCount;
	StreamState m_State;

	IMFMediaEventQueue* m_pEventQueue;
	IMFStreamDescriptor* m_pStreamDescriptor;

	CFlvSource* m_pSource;
	BOOL m_bActive;
	DWORD m_dwID;

	HRESULT CheckShutdown() const{ return (m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK); }
};

inline void CFlvStream::Activate(const BOOL bActive){

	TRACE_STREAM((L"Stream::Activate"));

	AutoLock lock(m_CriticSection);

	m_bActive = bActive;
}

#endif
