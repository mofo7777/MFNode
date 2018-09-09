//----------------------------------------------------------------------------------------------
// SurfaceParam.h
// Copyright (C) 2015 Dumonteil David
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
#ifndef SURFACEPARAM_H
#define SURFACEPARAM_H

class CSurfaceParam : RefCountedObject, public IUnknown{

public:

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void** ppv){ static const QITAB qit[] = {QITABENT(CSurfaceParam, IUnknown), { 0 }}; return QISearch(this, qit, riid, ppv); }
	STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
	STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

	CSurfaceParam(const DWORD iSurfaceIndex) : m_iSurfaceIndex(iSurfaceIndex){}
	virtual ~CSurfaceParam(){}

	const DWORD GetSurfaceIndex() const{ return m_iSurfaceIndex; }

private:

	DWORD m_iSurfaceIndex;
};

#endif