//----------------------------------------------------------------------------------------------
// CudaFrame.cpp
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

CCudaFrame::CCudaFrame()
		: m_iTotalSlice(0),
		  m_iNextSlice(0),
				m_iNextTemporal(0),
				m_iDecodedSlice(0),
				m_iFreeSlice(0),
				m_bPictureI(FALSE)
{
		ZeroMemory(CudaFrame, sizeof(CudaFrame));
		memset(&TmpFrame, -1, sizeof(TmpFrame));
}

BOOL CCudaFrame::AddFrame(const int iIndex, const int iPType, const int iTemporal){

		if(m_iTotalSlice == MAX_CUDA_DECODED_SURFACES)
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

				if(m_iNextTemporal == TmpFrame.iTemporal){

						CudaFrame[m_iNextSlice].iIndex = TmpFrame.iIndex;
		    CudaFrame[m_iNextSlice].iPType = TmpFrame.iPType;
		    CudaFrame[m_iNextSlice].iTemporal = TmpFrame.iTemporal;
		    CudaFrame[m_iNextSlice].rtTime = TmpFrame.rtTime;
						CheckNextSlice();
				}

				memset(&TmpFrame, -1, sizeof(TmpFrame));
				m_iNextTemporal = 0;
				m_bPictureI = FALSE;
		}

		if(iTemporal == m_iNextTemporal){

				CudaFrame[m_iNextSlice].iIndex = iIndex;
		  CudaFrame[m_iNextSlice].iPType = iPType;
		  CudaFrame[m_iNextSlice].iTemporal = iTemporal;
				CudaFrame[m_iNextSlice].rtTime = rtTime;
				m_iNextTemporal++;
		  CheckNextSlice();
		}
		else{

				if(m_iNextTemporal == TmpFrame.iTemporal){

						CudaFrame[m_iNextSlice].iIndex = TmpFrame.iIndex;
		    CudaFrame[m_iNextSlice].iPType = TmpFrame.iPType;
		    CudaFrame[m_iNextSlice].iTemporal = TmpFrame.iTemporal;
		    CudaFrame[m_iNextSlice].rtTime = TmpFrame.rtTime;
		    m_iNextTemporal++;
		    CheckNextSlice();
				}

				TmpFrame.iIndex = iIndex;
				TmpFrame.iPType = iPType;
				TmpFrame.iTemporal = iTemporal;
				TmpFrame.rtTime = rtTime;
		}

		m_iTotalSlice++;
		m_iFreeSlice++;

		return TRUE;
}

void CCudaFrame::Reset(){

		while(!m_qTimeStamp.empty()){
				m_qTimeStamp.pop();
  }

		m_iTotalSlice = 0;
		m_iNextSlice = 0;
		m_iNextTemporal = 0;
		m_iDecodedSlice = 0;
		m_iFreeSlice = 0;
		m_bPictureI = FALSE;

		ZeroMemory(CudaFrame, sizeof(CudaFrame));
		memset(&TmpFrame, -1, sizeof(TmpFrame));
}