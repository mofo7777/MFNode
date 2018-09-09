//----------------------------------------------------------------------------------------------
// KinectStream.h
// Copyright (C) 2012 Dumonteil David
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
#ifndef KINECTSTREAM_H
#define KINECTSTREAM_H

class CKinectSource;

class CKinectStream : BaseObject, RefCountedObject, public IMFMediaStream{

public:

	static HRESULT CreateInstance(CKinectStream**, CKinectSource*, IMFStreamDescriptor*, const DWORD, const int, HRESULT&);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
	STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaStream
	STDMETHODIMP GetMediaSource(IMFMediaSource**);
	STDMETHODIMP GetStreamDescriptor(IMFStreamDescriptor**);
	STDMETHODIMP RequestSample(IUnknown*);

	// CKinectStream
	HRESULT Shutdown();
	HRESULT Flush(){ return S_OK; }

private:

	CKinectStream(CKinectSource*, IMFStreamDescriptor*, const DWORD, const int, HRESULT&);
	virtual ~CKinectStream(){ Shutdown(); }

	CKinectSource* m_pKinectSource;

	CriticSection m_CriticSection;
	SourceState m_State;
	//BOOL m_bActive;
	int m_iID;

	LONGLONG m_rtCurrentPosition;
	//BOOL     m_discontinuity;        // Is the next sample a discontinuity?

	IMFMediaEventQueue* m_pEventQueue;
	IMFStreamDescriptor* m_pStreamDescriptor;

	IMFMediaBuffer* m_pMediaBuffer;
	BYTE* m_pVideoBuffer;
	DWORD m_dwVideoSize;

	//SampleQueue m_sampleQueue; // Queue for samples while paused.
	HRESULT CreateVideoKinectSample(IMFSample**);
	HRESULT CreateAudioKinectSample(IMFSample**);

	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
};

#endif