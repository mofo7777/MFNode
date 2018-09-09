//----------------------------------------------------------------------------------------------
// MFMpeg2Stream_Stream.cpp
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

HRESULT CMFMpeg2Stream::GetMediaSource(IMFMediaSource** ppMediaSource){

	TRACE_STREAM((L"Stream::GetMediaSource"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppMediaSource == NULL ? E_POINTER : S_OK));

	SourceLock lock(m_pSource);

	IF_FAILED_RETURN(hr = (m_pSource == NULL ? E_UNEXPECTED : S_OK));

	IF_FAILED_RETURN(hr = CheckShutdown());

	// (Does not hold the source's critical section.)
	IF_FAILED_RETURN(hr = m_pSource->QueryInterface(IID_PPV_ARGS(ppMediaSource)));

	return hr;
}

HRESULT CMFMpeg2Stream::GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor){

	TRACE_STREAM((L"Stream::GetStreamDescriptor"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppStreamDescriptor == NULL ? E_POINTER : S_OK));

	SourceLock lock(m_pSource);

	IF_FAILED_RETURN(hr = (m_pStreamDescriptor == NULL ? E_UNEXPECTED : S_OK));
	IF_FAILED_RETURN(hr = CheckShutdown());

	*ppStreamDescriptor = m_pStreamDescriptor;
	(*ppStreamDescriptor)->AddRef();

	return hr;
}

HRESULT CMFMpeg2Stream::RequestSample(IUnknown* pToken){

	TRACE_STREAM((L"Stream::RequestSample"));

	HRESULT hr = S_OK;

	SourceLock lock(m_pSource);

	try{

		IF_FAILED_THROW(hr = CheckShutdown());
		IF_FAILED_THROW(hr = (m_State == StreamStopped ? MF_E_INVALIDREQUEST : S_OK));
		IF_FAILED_THROW(hr = (!m_bActive ? MF_E_INVALIDREQUEST : S_OK));
		IF_FAILED_THROW(hr = (m_bEOS && m_Samples.IsEmpty() ? MF_E_END_OF_STREAM : S_OK));

		IF_FAILED_THROW(hr = m_Requests.InsertBack(pToken));
		IF_FAILED_THROW(hr = DispatchSamples());
	}
	catch(HRESULT){}

	// If there was an error, queue MEError from the source (except after shutdown).
	if(FAILED(hr) && (m_State != StreamFinalized)){
		LOG_HRESULT(hr = m_pSource->QueueEvent(MEError, GUID_NULL, hr, NULL));
	}

	return hr;
}