//----------------------------------------------------------------------------------------------
// StreamVideoRenderer_Sink.cpp
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
#include "StdAfx.h"

HRESULT CStreamVideoRenderer::GetMediaSink(IMFMediaSink** ppMediaSink){

		TRACE_STREAM((L"StreamRenderer::GetMediaSink"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppMediaSink == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*ppMediaSink = m_pSinkVideoRenderer;
		(*ppMediaSink)->AddRef();

		return hr;
}

HRESULT CStreamVideoRenderer::GetIdentifier(DWORD* pdwIdentifier){

		TRACE_STREAM((L"StreamRenderer::GetIdentifier"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwIdentifier == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pdwIdentifier = 0;
		
		return hr;
}

HRESULT CStreamVideoRenderer::GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler){

		TRACE_STREAM((L"StreamRenderer::GetMediaTypeHandler"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppHandler == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = this->QueryInterface(IID_IMFMediaTypeHandler, reinterpret_cast<void**>(ppHandler)));
		
		return hr;
}

HRESULT CStreamVideoRenderer::ProcessSample(IMFSample* pSample){

		TRACE_STREAM((L"StreamRenderer::ProcessSample"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = m_pSinkVideoRenderer->ProcessSample(pSample));

		//IF_FAILED_RETURN(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));

		return hr;
}

HRESULT CStreamVideoRenderer::PlaceMarker(MFSTREAMSINK_MARKER_TYPE /*eMarkerType*/, const PROPVARIANT* /*pvarMarkerValue*/, const PROPVARIANT* /*pvarContextValue*/){

		TRACE_STREAM((L"StreamRenderer::PlaceMarker"));

		// Todo check marker.
		return E_NOTIMPL;
}

HRESULT CStreamVideoRenderer::Flush(){

		TRACE_STREAM((L"StreamRenderer::Flush"));

		// if CStreamVideoRenderer::PlaceMarker is implemented, see :
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms701626(v=vs.85).aspx

		return S_OK;
}