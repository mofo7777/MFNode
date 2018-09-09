//-----------------------------------------------------------------------------------------------
// MFTMpegAudioDecoder.cpp
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
//-----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CMFTMpegAudioDecoder::CMFTMpegAudioDecoder()
		: m_nRefCount(1),
		  m_pInputType(NULL),
				m_pLastInputBuffer(NULL),
				m_pOutputType(NULL),
				m_pOutputBuffer(NULL),
				m_pAudioSample(NULL),
				m_HasAudio(FALSE),
				m_pInputBuffer(NULL),
				m_rtTime(0),
				m_uiAvgRate(0),
				m_bValidTime(FALSE)
{
		//ZeroMemory(m_sAudioBuffer, 4608);
}

CMFTMpegAudioDecoder::~CMFTMpegAudioDecoder(){

		AutoLock lock(m_CriticSection);

		FreeStreamingResources();

		SAFE_RELEASE(m_pInputType);
		SAFE_RELEASE(m_pLastInputBuffer);
		SAFE_RELEASE(m_pOutputType);
}

HRESULT CMFTMpegAudioDecoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		IF_FAILED_RETURN(ppv == NULL ? E_POINTER : S_OK);
		IF_FAILED_RETURN(pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK);

		HRESULT hr = S_OK;

		CMFTMpegAudioDecoder* pMFT = new (std::nothrow)CMFTMpegAudioDecoder;

		IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

		LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

		SAFE_RELEASE(pMFT);

		return hr;
}

HRESULT CMFTMpegAudioDecoder::QueryInterface(REFIID riid, void** ppv){
		
		TRACE_TRANSFORM((L"Transform::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CMFTMpegAudioDecoder, IMFTransform),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

ULONG CMFTMpegAudioDecoder::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Transform::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFTMpegAudioDecoder::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"Transform::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CMFTMpegAudioDecoder::AllocateStreamingResources(){

		TRACE_TRANSFORM((L"Transform::AllocateStreamingResources"));

		// The client must set media type before allocating streaming resources.
		IF_FAILED_RETURN(m_pInputType == NULL ? MF_E_TRANSFORM_TYPE_NOT_SET : S_OK);

		HRESULT hr = S_OK;

		if(m_pAudioSample)
				return hr;

		m_cMpeg12Decoder.Initialize();

		IF_FAILED_RETURN(hr = MFCreateSample(&m_pAudioSample));

		SAFE_RELEASE(m_pInputBuffer);
		SAFE_RELEASE(m_pLastInputBuffer);

		m_rtTime = 0;
		m_HasAudio = FALSE;

		return hr;
}

void CMFTMpegAudioDecoder::FreeStreamingResources(){

		TRACE_TRANSFORM((L"Transform::FreeStreamingResources"));

		ReleaseBufferSample();
		SAFE_RELEASE(m_pOutputBuffer);
		SAFE_RELEASE(m_pAudioSample);
		SAFE_RELEASE(m_pInputBuffer);
		SAFE_RELEASE(m_pLastInputBuffer);

		// Check set to zero......
		m_rtTime = 0;
		m_HasAudio = FALSE;
		m_bValidTime = FALSE;
}

void CMFTMpegAudioDecoder::OnDiscontinuity(){

		TRACE_TRANSFORM((L"Transform::OnDiscontinuity"));

		// Check set to zero......
		m_rtTime = 0;
}

void CMFTMpegAudioDecoder::OnFlush(){

		TRACE_TRANSFORM((L"Transform::OnFlush"));

		OnDiscontinuity();

		ReleaseBufferSample();
		SAFE_RELEASE(m_pAudioSample);
		m_HasAudio = FALSE;
}