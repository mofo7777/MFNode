//----------------------------------------------------------------------------------------------
// MFSafeQueue.h
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
#ifndef MFSAFEQUEUE_H
#define MFSAFEQUEUE_H

class CMFSafeQueue{

		public:

				CMFSafeQueue(){}
				~CMFSafeQueue(){}

				void PushSample(IMFSample* pSample){

						assert(pSample);

						AutoLock lock(m_CriticSection);

						m_qVideoSample.push(pSample);
						pSample->AddRef();
				}

				HRESULT GetSampleNoAddRef(IMFSample** ppSample){

						assert(ppSample);

						AutoLock lock(m_CriticSection);

						HRESULT hr;

						if(m_qVideoSample.empty()){
								hr = S_FALSE;
						}
						else{

								*ppSample = m_qVideoSample.front();
								hr = S_OK;
						}

						return hr;
				}

				void PopSample(){

						AutoLock lock(m_CriticSection);

						if(m_qVideoSample.empty()){
								return;
						}

						IMFSample* pSample = m_qVideoSample.front();
						SAFE_RELEASE(pSample);
      m_qVideoSample.pop();
				}

				void ReleaseQueue(){

						AutoLock lock(m_CriticSection);

						IMFSample* pSample = NULL;

						while(!m_qVideoSample.empty()){

								pSample = m_qVideoSample.front();
				    SAFE_RELEASE(pSample);
        m_qVideoSample.pop();
						}
				}

				const UINT32 GetSize() const{ return m_qVideoSample.size(); }

		private:

				CriticSection m_CriticSection;

				queue<IMFSample*> m_qVideoSample;
};

#endif