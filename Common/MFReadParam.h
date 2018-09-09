//----------------------------------------------------------------------------------------------
// MFReadParam.h
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
#ifndef MFREADPARAM_H
#define MFREADPARAM_H

class CMFReadParam : public IUnknown{
		
		public:

    CMFReadParam(BYTE* pData, ULONG ulToRead) : m_nRefCount(1), m_pData(pData), m_ulToRead(ulToRead){}
				CMFReadParam() : m_nRefCount(1), m_pData(NULL), m_ulToRead(0){}

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv){
						
						static const QITAB qit[] = {
								QITABENT(CMFReadParam, IUnknown),
								{0}
						};
						
						return QISearch(this, qit, riid, ppv);
				}

				STDMETHODIMP_(ULONG) AddRef(){ return InterlockedIncrement(&m_nRefCount); }

				STDMETHODIMP_(ULONG) Release(){
						
						LONG cRef = InterlockedDecrement(&m_nRefCount);
						
						if(cRef == 0){
								delete this;
						}
						return cRef;
				}

				BYTE* GetDataPtr(){ return m_pData; }
				ULONG GetByteToRead(){ return m_ulToRead; }
				DWORD* GetpByteRead(){ return &m_dwRead; }
				DWORD GetByteRead(){ return m_dwRead; }

				void SetDataPtr(BYTE* pData){ m_pData = pData; }
				void SetByteToRead(ULONG ulToRead){ m_ulToRead = ulToRead; }

  private:

				volatile long m_nRefCount;

				BYTE* m_pData;
				ULONG m_ulToRead;
				DWORD m_dwRead;
};

#endif