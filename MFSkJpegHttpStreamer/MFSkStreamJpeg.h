//----------------------------------------------------------------------------------------------
// MFSkStreamJpeg.h
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
#ifndef MFSKSTREAMJPEG_H
#define MFSKSTREAMJPEG_H

class CMFSkStreamJpeg : BaseObject, public IMFStreamSink, public IMFMediaTypeHandler{

		public:

				// MFSkStreamJpeg.cpp
				static HRESULT CreateInstance(CMFSkJpegHttpStreamer*, CMFSkStreamJpeg**, HRESULT&);

				// IUnknown - MFSkStreamJpeg.cpp
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

				// IMFStreamSink - MFSkStreamJpeg_Sink.cpp
				STDMETHODIMP GetMediaSink(IMFMediaSink** ppMediaSink);
				STDMETHODIMP GetIdentifier(DWORD* pdwIdentifier);
				STDMETHODIMP GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler);
				STDMETHODIMP ProcessSample(IMFSample* pSample);
				STDMETHODIMP PlaceMarker(MFSTREAMSINK_MARKER_TYPE eMarkerType, const PROPVARIANT* pvarMarkerValue, const PROPVARIANT* pvarContextValue);
				STDMETHODIMP Flush();

				// IMFMediaEventGenerator - MFSkStreamJpeg_Event.cpp
				STDMETHODIMP GetEvent(DWORD, IMFMediaEvent**);
				STDMETHODIMP BeginGetEvent(IMFAsyncCallback*, IUnknown*);
				STDMETHODIMP EndGetEvent(IMFAsyncResult*, IMFMediaEvent**);
				STDMETHODIMP QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*);

				// IMFMediaTypeHandler - MFSkStreamJpeg_Type.cpp
				STDMETHODIMP IsMediaTypeSupported(IMFMediaType*, IMFMediaType**);
    STDMETHODIMP GetMediaTypeCount(DWORD*);
    STDMETHODIMP GetMediaTypeByIndex(DWORD, IMFMediaType**);
    STDMETHODIMP SetCurrentMediaType(IMFMediaType*);
    STDMETHODIMP GetCurrentMediaType(IMFMediaType**);
    STDMETHODIMP GetMajorType(GUID*);

				// MFSkStreamJpeg.cpp
				HRESULT Start(MFTIME);
				HRESULT Stop();
    HRESULT Pause();
				HRESULT Restart();
				HRESULT Shutdown();

		private:

				// MFSkStreamJpeg.cpp
				CMFSkStreamJpeg(IMFMediaSink*, HRESULT&);
				virtual ~CMFSkStreamJpeg();

    CriticSection m_CriticSection;
				volatile long m_nRefCount;
    StreamState m_State;

				IMFMediaEventQueue* m_pEventQueue;
    IMFMediaType* m_pMediaType;
				IMFMediaSink* m_pSink;

				CHttpServer m_cHttpServer;

				// Inline
				HRESULT CheckShutdown() const{ return ( m_State == StreamFinalized ? MF_E_SHUTDOWN : S_OK ); }
};

#endif