//----------------------------------------------------------------------------------------------
// CudaFrame.h
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
#ifndef CUDAFRAME_H
#define CUDAFRAME_H

class CCudaFrame{

  public:

				CCudaFrame();
				~CCudaFrame(){}

				BOOL AddFrame(const int, const int, const int);
				const BOOL IsFull() const{ return m_iTotalSlice == MAX_CUDA_DECODED_SURFACES; }
				const BOOL IsEmpty() const{ return m_iTotalSlice == 0; }
				SCUDAFRAME* GetNextFrame();
				void Reset();
				void StatePictureI(){ m_bPictureI = TRUE; }
				int GetFreeIDX() const{ return m_iFreeSlice; }
				void SetTime(const REFERENCE_TIME, const REFERENCE_TIME);
				void CheckPictureTime();

  private:

				SCUDAFRAME CudaFrame[MAX_CUDA_DECODED_SURFACES];
				SCUDAFRAME TmpFrame;
				int m_iTotalSlice;
				int m_iNextSlice;
				int m_iNextTemporal;
				int m_iDecodedSlice;
				int m_iFreeSlice;
				BOOL m_bPictureI;

				struct TIMESTAMP_INFO{

						REFERENCE_TIME rtPTS;
						REFERENCE_TIME rtDTS;
				};

				queue<TIMESTAMP_INFO> m_qTimeStamp;

				void CheckNextSlice(){ m_iNextSlice++; if(m_iNextSlice == MAX_CUDA_DECODED_SURFACES) m_iNextSlice = 0; }
};

inline void CCudaFrame::SetTime(const REFERENCE_TIME rtPTS, const REFERENCE_TIME rtDTS){

		#ifdef TRACE_SET_PTS
		  TRACE((L"SetTime = %I64d : %I64d", rtPTS, rtDTS));
  #endif

		assert(rtPTS != -1);

		TIMESTAMP_INFO rtInfo;
		rtInfo.rtPTS = rtPTS;
		rtInfo.rtDTS = rtDTS;

  m_qTimeStamp.push(rtInfo);
}

inline void CCudaFrame::CheckPictureTime(){

		if(m_qTimeStamp.empty()){

				TIMESTAMP_INFO rtInfo = {-1, -1};
				m_qTimeStamp.push(rtInfo);
  }
}

inline SCUDAFRAME* CCudaFrame::GetNextFrame(){

		if(m_iDecodedSlice == MAX_CUDA_DECODED_SURFACES)
				m_iDecodedSlice = 0;

		m_iTotalSlice--;
		m_iFreeSlice = CudaFrame[m_iDecodedSlice].iIndex;

  #ifdef TRACE_CUDA_PTS
		  TRACE((L"Time = %I64d", CudaFrame[m_iDecodedSlice].rtTime));
  #endif

		return &CudaFrame[m_iDecodedSlice++];
}

#endif