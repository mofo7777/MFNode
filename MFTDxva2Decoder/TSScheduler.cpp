//----------------------------------------------------------------------------------------------
// TSScheduler.cpp
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
#include "Stdafx.h"

CTSScheduler::CTSScheduler()
	: m_iTotalSlice(0),
	m_iNextSlice(0),
	m_iNextTemporal(0),
	m_iDecodedSlice(0),
	m_iFreeSlice(0),
	m_bPictureI(FALSE)
{
	ZeroMemory(m_sTemporalRef, sizeof(m_sTemporalRef));
	memset(&m_TmpTemporalRef, -1, sizeof(m_TmpTemporalRef));
}

BOOL CTSScheduler::AddFrame(const int iIndex, const int iPType, const int iTemporal){

	if(m_iTotalSlice == NUM_DXVA2_SURFACE)
		return FALSE;

	REFERENCE_TIME rtTime;

	if(m_qTimeStamp.empty()){
		rtTime = -1;
	}
	else{

		const TIMESTAMP_INFO rtInfo = m_qTimeStamp.front();

		if(iPType == PICTURE_TYPE_B && rtInfo.rtDTS != -1 && rtInfo.rtPTS != rtInfo.rtDTS){
			rtTime = -1;
		}
		else{
			m_qTimeStamp.pop();
			rtTime = rtInfo.rtPTS;
		}
	}

	if(m_bPictureI){

		if(m_iNextTemporal == m_TmpTemporalRef.iTemporal){

			m_sTemporalRef[m_iNextSlice].iIndex = m_TmpTemporalRef.iIndex;
			m_sTemporalRef[m_iNextSlice].iPType = m_TmpTemporalRef.iPType;
			m_sTemporalRef[m_iNextSlice].iTemporal = m_TmpTemporalRef.iTemporal;
			m_sTemporalRef[m_iNextSlice].rtTime = m_TmpTemporalRef.rtTime;
			CheckNextSlice();
		}

		memset(&m_TmpTemporalRef, -1, sizeof(m_TmpTemporalRef));
		m_iNextTemporal = 0;
		m_bPictureI = FALSE;
	}

	if(iTemporal == m_iNextTemporal){

		m_sTemporalRef[m_iNextSlice].iIndex = iIndex;
		m_sTemporalRef[m_iNextSlice].iPType = iPType;
		m_sTemporalRef[m_iNextSlice].iTemporal = iTemporal;
		m_sTemporalRef[m_iNextSlice].rtTime = rtTime;
		m_iNextTemporal++;
		CheckNextSlice();
	}
	else{

		if(m_iNextTemporal == m_TmpTemporalRef.iTemporal){

			m_sTemporalRef[m_iNextSlice].iIndex = m_TmpTemporalRef.iIndex;
			m_sTemporalRef[m_iNextSlice].iPType = m_TmpTemporalRef.iPType;
			m_sTemporalRef[m_iNextSlice].iTemporal = m_TmpTemporalRef.iTemporal;
			m_sTemporalRef[m_iNextSlice].rtTime = m_TmpTemporalRef.rtTime;
			m_iNextTemporal++;
			CheckNextSlice();
		}

		m_TmpTemporalRef.iIndex = iIndex;
		m_TmpTemporalRef.iPType = iPType;
		m_TmpTemporalRef.iTemporal = iTemporal;
		m_TmpTemporalRef.rtTime = rtTime;
	}

	m_iTotalSlice++;
	m_iFreeSlice++;

	return TRUE;
}

void CTSScheduler::Reset(){

	while(!m_qTimeStamp.empty()){
		m_qTimeStamp.pop();
	}

	m_iTotalSlice = 0;
	m_iNextSlice = 0;
	m_iNextTemporal = 0;
	m_iDecodedSlice = 0;
	m_iFreeSlice = 0;
	m_bPictureI = FALSE;

	ZeroMemory(m_sTemporalRef, sizeof(m_sTemporalRef));
	memset(&m_TmpTemporalRef, -1, sizeof(m_TmpTemporalRef));
}