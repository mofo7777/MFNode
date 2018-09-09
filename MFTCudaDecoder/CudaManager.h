//----------------------------------------------------------------------------------------------
// CudaManager.h
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
#ifndef CUDAMANAGER_H
#define CUDAMANAGER_H

class CCudaManager{

public:

	CCudaManager() : m_pCudaContext(NULL), m_pVideoCtxLock(NULL), m_ReadbackSID(NULL){ m_btFrame[0] = NULL; m_btFrame[1] = NULL; }
	~CCudaManager(){ Release(); }

	HRESULT InitCudaManager();
	HRESULT InitFrameCuda(const UINT, const UINT);
	void Release();

	const CUvideoctxlock GetVideoLock() const{ return m_pVideoCtxLock; }
	const CUcontext GetCudaContext() const{ return m_pCudaContext; }
	const CUstream GetCudaStream() const{ return m_ReadbackSID; }

	BYTE* GetFrame(){ return m_btFrame[0]; }
	void SetNV12ToYV12(BYTE**, const UINT, const UINT, const UINT);

private:

	CUdevice m_CudaDevice;
	CUcontext m_pCudaContext;
	CUstream m_ReadbackSID;
	CUvideoctxlock m_pVideoCtxLock;

	BYTE* m_btFrame[2];
};

#endif