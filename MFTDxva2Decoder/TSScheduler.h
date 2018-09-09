//----------------------------------------------------------------------------------------------
// TSScheduler.h
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
#ifndef TSSCHEDULER_H
#define TSSCHEDULER_H

class CTSScheduler{

public:

		CTSScheduler();
		~CTSScheduler(){}

		BOOL AddFrame(const int, const int, const int);
		const BOOL IsFull() const{ return m_iTotalSlice == NUM_DXVA2_SURFACE; }
		const BOOL IsEmpty() const{ return m_iTotalSlice == 0; }
		STemporalRef* GetNextFrame();
		void Reset();
		void StatePictureI(){ m_bPictureI = TRUE; }
		int GetFreeIDX() const{ return m_iFreeSlice; }
		void SetTime(const REFERENCE_TIME, const REFERENCE_TIME);
		void CheckPictureTime();

private:

		STemporalRef m_sTemporalRef[NUM_DXVA2_SURFACE];
		STemporalRef m_TmpTemporalRef;
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

		void CheckNextSlice(){ m_iNextSlice++; if(m_iNextSlice == NUM_DXVA2_SURFACE) m_iNextSlice = 0; }
};

inline void CTSScheduler::SetTime(const REFERENCE_TIME rtPTS, const REFERENCE_TIME rtDTS){

  #ifdef TRACE_SET_PTS
		  TRACE((L"SetTime = %I64d : %I64d", rtPTS, rtDTS));
  #endif

		assert(rtPTS != -1);

		TIMESTAMP_INFO rtInfo;
		rtInfo.rtPTS = rtPTS;
		rtInfo.rtDTS = rtDTS;

		m_qTimeStamp.push(rtInfo);
}

inline void CTSScheduler::CheckPictureTime(){

		if(m_qTimeStamp.empty()){

				TIMESTAMP_INFO rtInfo = { -1, -1 };
				m_qTimeStamp.push(rtInfo);
		}
}

inline STemporalRef* CTSScheduler::GetNextFrame(){

		if(m_iDecodedSlice == NUM_DXVA2_SURFACE)
				m_iDecodedSlice = 0;

		m_iTotalSlice--;
		m_iFreeSlice = m_sTemporalRef[m_iDecodedSlice].iIndex;

  #ifdef TRACE_MPEG_PTS
		  TRACE((L"Time = %I64d", m_sTemporalRef[m_iDecodedSlice].rtTime));
  #endif

		return &m_sTemporalRef[m_iDecodedSlice++];
}

#endif