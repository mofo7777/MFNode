//----------------------------------------------------------------------------------------------
// KinectDevice.h
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
#ifndef KINECTDEVICE_H
#define KINECTDEVICE_H

class CDmoMediaBuffer : public IMediaBuffer{

  public:

				CDmoMediaBuffer() : m_dataLength(0) {}

				// IUnknown methods
				STDMETHODIMP_(ULONG) AddRef() { return 2; }
				STDMETHODIMP_(ULONG) Release() { return 1; }
				STDMETHODIMP QueryInterface(REFIID riid, void** ppv){
						
						if(riid == IID_IUnknown){
								
								AddRef();
								*ppv = (IUnknown*)this;
								return NOERROR;
						}
						else if(riid == IID_IMediaBuffer){
								
								AddRef();
								*ppv = (IMediaBuffer*)this;
								return NOERROR;
						}
						else{
								return E_NOINTERFACE;
						}
				}

				// IMediaBuffer methods
    STDMETHODIMP SetLength(DWORD length){ m_dataLength = length; return NOERROR; }
    STDMETHODIMP GetMaxLength(DWORD* pMaxLength){ *pMaxLength = sizeof(m_pData); return NOERROR; }
    STDMETHODIMP GetBufferAndLength(BYTE** ppBuffer, DWORD* pLength){
						
						if(ppBuffer){
								*ppBuffer = m_pData;
						}
						if(pLength){
								*pLength = m_dataLength;
						}
						return NOERROR;
				}
				void Init(ULONG ulData){
						m_dataLength = ulData;
				}

  protected:
				
				// Statically allocated buffer used to hold audio data returned by IMediaObject
				// AudioSamplesPerSecond * AudioBlockAlign = 16000 * 2
				BYTE m_pData[32000];

				// Amount of data currently being held in m_pData
				ULONG m_dataLength;
};

class CKinectDevice{

  public:

				CKinectDevice();
				~CKinectDevice(){ ReleaseKinect(); }

				HRESULT Initialize();
				HRESULT GetVideoKinectType(IMFMediaType**);
				HRESULT GetAudioKinectType(IMFMediaType**);
				HRESULT GetNextVideoFrame(BYTE*, DWORD, LONGLONG&);
				HRESULT GetNextAudioFrame(BYTE*, DWORD&);

  private:

				INuiSensor* m_pNuiSensor;
				HANDLE m_hNextKinectFrameEvent;
				HANDLE m_hKinectStreamHandle;

				IMediaObject* m_pAudioStream;
				CDmoMediaBuffer m_AudioBuffer;

				UINT32 m_uiWidthInPixels;
				UINT32 m_uiHeightInPixels;
				MFRatio m_FrameRate;

				void ReleaseKinect();
				HRESULT InitializeAudioSource();
};
#endif
