//----------------------------------------------------------------------------------------------
// Vp6Decoder.cpp
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

CVp6Decoder::CVp6Decoder()
		: m_nRefCount(1),
		  //m_pDecoder2(NULL),
		  m_pInputType(NULL),
		  m_pOutputType(NULL),
    m_pInputSample(NULL),
				m_dwSampleSize(0),
				m_uiWidthInPixels(0),
				m_uiHeightInPixels(0),
				m_bDraining(FALSE)
				//m_rtAvgPerFrameInput(0)
{

				TRACE_TRANSFORM((L"VP6Decoder::CTOR"));

				//m_FrameRate.Numerator = m_FrameRate.Denominator = 0;
}

CVp6Decoder::~CVp6Decoder(){

		TRACE_TRANSFORM((L"VP6Decoder::DTOR"));

		AutoLock lock(m_CriticSection);

		//SAFE_DELETE(m_pDecoder2);
		m_pDecoder.ReleaseDecoder();
		SAFE_RELEASE(m_pInputSample);

		SAFE_RELEASE(m_pInputType);
		SAFE_RELEASE(m_pOutputType);
}

HRESULT CVp6Decoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		TRACE_TRANSFORM((L"VP6Decoder::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CVp6Decoder* pMFT = new (std::nothrow)CVp6Decoder;

		IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

		LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

		SAFE_RELEASE(pMFT);

		return hr;
}

ULONG CVp6Decoder::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"VP6Decoder::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CVp6Decoder::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"VP6Decoder::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CVp6Decoder::QueryInterface(REFIID riid, void** ppv){

		TRACE_TRANSFORM((L"VP6Decoder::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = {
				QITABENT(CVp6Decoder, IMFTransform),
				{0}
		};
		
		return QISearch(this, qit, riid, ppv);
}

void CVp6Decoder::OnFlush(){

		m_bDraining = FALSE;
		SAFE_RELEASE(m_pInputSample);
}