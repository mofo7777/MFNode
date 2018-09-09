//----------------------------------------------------------------------------------------------
// A52Decoder.cpp
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

CA52Decoder::CA52Decoder()
	: m_nRefCount(1),
	m_pInputType(NULL),
	m_pOutputType(NULL),
	m_pInputSample(NULL),
	m_A52State(NULL),
	m_uiChannels(0),
	m_uiSamplePerSec(0),
	m_rtSampleTime(0),
	m_bValidTime(FALSE)
{

	TRACE_TRANSFORM((L"A52Decoder::CTOR"));
}

CA52Decoder::~CA52Decoder(){

	TRACE_TRANSFORM((L"A52Decoder::DTOR"));

	AutoLock lock(m_CriticSection);

	if(m_A52State){
		a52_free(m_A52State);
		m_A52State = NULL;
	}

	SAFE_RELEASE(m_pInputSample);

	SAFE_RELEASE(m_pInputType);
	SAFE_RELEASE(m_pOutputType);
}

HRESULT CA52Decoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

	TRACE_TRANSFORM((L"A52Decoder::CreateInstance"));

	HRESULT hr;
	IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
	IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

	CA52Decoder* pMFT = new (std::nothrow)CA52Decoder;

	IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

	LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

	SAFE_RELEASE(pMFT);

	return hr;
}

ULONG CA52Decoder::AddRef(){

	LONG lRef = InterlockedIncrement(&m_nRefCount);

	TRACE_REFCOUNT((L"A52Decoder::AddRef m_nRefCount = %d", lRef));

	return lRef;
}

ULONG CA52Decoder::Release(){

	ULONG uCount = InterlockedDecrement(&m_nRefCount);

	TRACE_REFCOUNT((L"A52Decoder::Release m_nRefCount = %d", uCount));

	if(uCount == 0){
		delete this;
	}

	return uCount;
}

HRESULT CA52Decoder::QueryInterface(REFIID riid, void** ppv){

	TRACE_TRANSFORM((L"A52Decoder::QI : riid = %s", GetIIDString(riid)));

	static const QITAB qit[] = {
			QITABENT(CA52Decoder, IMFTransform),
			{0}
	};

	return QISearch(this, qit, riid, ppv);
}

void CA52Decoder::OnFlush(){

	m_bDraining = FALSE;
	SAFE_RELEASE(m_pInputSample);
}

HRESULT CA52Decoder::OnStartStreaming(){

	HRESULT hr = S_OK;

	m_bDraining = FALSE;
	SAFE_RELEASE(m_pInputSample);

	if(m_A52State == NULL){

		m_A52State = a52_init();
		IF_FAILED_RETURN(hr = (m_A52State == NULL ? E_POINTER : S_OK));
	}

	return hr;
}