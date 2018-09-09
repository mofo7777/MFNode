//----------------------------------------------------------------------------------------------
// CudaManager.cpp
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

HRESULT CCudaManager::InitCudaManager(){

	HRESULT hr = S_OK;

	try{

		if(cuInit(0) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuDeviceGet(&m_CudaDevice, 0) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		size_t totalGlobalMem = 0;
		int iMajor = 0;
		int iMinor = 0;
		char deviceName[256];

		if(cuDeviceComputeCapability(&iMajor, &iMinor, m_CudaDevice) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuDeviceGetName(deviceName, 256, m_CudaDevice) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuDeviceTotalMem(&totalGlobalMem, m_CudaDevice) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuCtxCreate(&m_pCudaContext, CU_CTX_BLOCKING_SYNC, m_CudaDevice) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuvidCtxLockCreate(&m_pVideoCtxLock, m_pCudaContext) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuStreamCreate(&m_ReadbackSID, 0) != CUDA_SUCCESS)
			throw hr = E_FAIL;
	}
	catch(HRESULT){}

	return hr;
}

HRESULT CCudaManager::InitFrameCuda(const UINT uiHeightInPixels, const UINT uiPitch){

	HRESULT hr = S_OK;
	UINT uiFrameSize = (uiPitch * uiHeightInPixels * 3 / 2);

	try{

		if(cuMemAllocHost((void**)&m_btFrame[0], uiFrameSize) != CUDA_SUCCESS)
			throw hr = E_FAIL;

		if(cuMemAllocHost((void**)&m_btFrame[1], uiFrameSize) != CUDA_SUCCESS)
			throw hr = E_FAIL;
	}
	catch(HRESULT){}

	return hr;
}

void CCudaManager::Release(){

	cuCtxPushCurrent(m_pCudaContext);

	if(m_btFrame[0]){

		cuMemFreeHost((void*)m_btFrame[0]);
		m_btFrame[0] = NULL;
	}

	if(m_btFrame[1]){

		cuMemFreeHost((void*)m_btFrame[1]);
		m_btFrame[1] = NULL;
	}

	if(m_ReadbackSID){
		cuStreamDestroy(m_ReadbackSID);
		m_ReadbackSID = NULL;
	}

	cuCtxPopCurrent(NULL);

	if(m_pVideoCtxLock){
		cuvidCtxLockDestroy(m_pVideoCtxLock);
		m_pVideoCtxLock = NULL;
	}

	if(m_pCudaContext){
		cuCtxDestroy(m_pCudaContext);
		m_pCudaContext = NULL;
	}
}

void CCudaManager::SetNV12ToYV12(BYTE** ppData, const UINT uiWidthInPixels, const UINT uiHeightInPixels, const UINT uiPitch){

	BYTE* bNV12 = m_btFrame[0];
	BYTE* bInV;
	BYTE* bYV12 = *ppData;
	BYTE* bOutV;
	UINT uiSmallPitch = uiPitch - uiWidthInPixels;
	UINT uiHalfWidth = uiWidthInPixels / 2;
	UINT uiHalfHeight = uiHeightInPixels / 2;

	for(UINT i = 0; i < uiHeightInPixels; i++){

		memcpy(bYV12, bNV12, uiWidthInPixels);

		bYV12 += uiWidthInPixels;
		bNV12 += uiPitch;
	}

	bInV = bNV12 + 1;
	bOutV = bYV12 + ((uiHalfWidth / 2) * uiHeightInPixels);

	for(UINT i = 0; i < uiHalfHeight; i++){

		for(UINT j = 0; j < uiHalfWidth; j++){

			*bOutV++ = *bNV12++;
			*bYV12++ = *bInV++;
			bNV12++;
			bInV++;
		}
		bNV12 += uiSmallPitch;
		bInV += uiSmallPitch;
	}
}