//----------------------------------------------------------------------------------------------
// Mpeg2Decoder.cpp
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
#include "StdAfx.h"

HRESULT CMpeg2Decoder::CreateInstance(CMpeg2Decoder** ppDec){

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppDec == NULL ? E_POINTER : S_OK));

		CMpeg2Decoder* pDec = new (std::nothrow)CMpeg2Decoder;

		IF_FAILED_RETURN(pDec == NULL ? E_OUTOFMEMORY : S_OK);

		*ppDec = pDec;
		(*ppDec)->AddRef();

		SAFE_RELEASE(pDec);

		return hr;
}

HRESULT CMpeg2Decoder::QueryInterface(REFIID riid, void** ppv){

		static const QITAB qit[] = { QITABENT(CMpeg2Decoder, IUnknown), {0} };
		return QISearch(this, qit, riid, ppv);
}