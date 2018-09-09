//----------------------------------------------------------------------------------------------
// KinectSource.h
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
#ifndef KINECTSOURCE_H
#define KINECTSOURCE_H

class CKinectSource : BaseObject, RefCountedObject, public IMFMediaSource{

public:

	static HRESULT CreateInstance(CKinectSource**);

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID, void**);
	STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
	STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

	// IMFMediaEventGenerator
	STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
	STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
	STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
	STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

	// IMFMediaSource
	STDMETHODIMP CreatePresentationDescriptor(IMFPresentationDescriptor**);
	STDMETHODIMP GetCharacteristics(DWORD*);
	STDMETHODIMP Pause();
	STDMETHODIMP Shutdown();
	STDMETHODIMP Start(IMFPresentationDescriptor*, const GUID*, const PROPVARIANT*);
	STDMETHODIMP Stop();

	// CKinectSource
	HRESULT LoadKinect(LPCWSTR);
	SourceState GetState() const { return m_State; }

	// CKinectDevice
	HRESULT KinectGetNextVideoFrame(BYTE*, DWORD, LONGLONG&);
	HRESULT KinectGetNextAudioFrame(BYTE*, DWORD&);

private:

	CKinectSource(HRESULT&);
	virtual ~CKinectSource(){ Shutdown(); }

	CKinectDevice* m_pKinectDevice;
	CKinectStream* m_pKinectStream[2];

	CriticSection m_CriticSection;
	SourceState m_State;

	IMFMediaEventQueue* m_pEventQueue;
	IMFPresentationDescriptor* m_pPresentationDescriptor;

	HRESULT CreateKinectPresentationDescriptor();
	HRESULT ValidatePresentationDescriptor(IMFPresentationDescriptor*);
	HRESULT QueueNewStreamEvent(IMFPresentationDescriptor*);

	HRESULT CheckShutdown() const{ return (m_State == SourceShutdown ? MF_E_SHUTDOWN : S_OK); }
};

inline HRESULT CKinectSource::KinectGetNextVideoFrame(BYTE* pBuffer, DWORD cbBuffer, LONGLONG& duration){

	AutoLock lock(m_CriticSection);

	if(m_pKinectDevice)
		return m_pKinectDevice->GetNextVideoFrame(pBuffer, cbBuffer, duration);
	else
		return E_POINTER;
}

inline HRESULT CKinectSource::KinectGetNextAudioFrame(BYTE* pBuffer, DWORD& cbBuffer){

	AutoLock lock(m_CriticSection);

	if(m_pKinectDevice)
		return m_pKinectDevice->GetNextAudioFrame(pBuffer, cbBuffer);
	else
		return E_POINTER;
}

#endif