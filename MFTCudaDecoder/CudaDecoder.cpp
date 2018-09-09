//----------------------------------------------------------------------------------------------
// CudaDecoder.cpp
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

HRESULT CCudaDecoder::InitDecoder(const CUvideoctxlock VidCtxLock, const UINT32 uiWidthInPixels, const UINT32 uiHeightInPixels, const BOOL bMpeg1){

		CUVIDDECODECREATEINFO VideoDecodeCreateInfo;

		// Fill the decoder-create-info struct from the given video-format struct.
		memset(&VideoDecodeCreateInfo, 0, sizeof(CUVIDDECODECREATEINFO));
		
		// Create video decoder
		if(bMpeg1){
				VideoDecodeCreateInfo.CodecType = cudaVideoCodec_MPEG1;
		}
		else{
				VideoDecodeCreateInfo.CodecType = cudaVideoCodec_MPEG2;
		}

		VideoDecodeCreateInfo.ulWidth             = uiWidthInPixels;
		VideoDecodeCreateInfo.ulHeight            = uiHeightInPixels;
		VideoDecodeCreateInfo.ulNumDecodeSurfaces = MAX_CUDA_DECODED_SURFACES;
		
		// Limit decode memory to 24MB (16M pixels at 4:2:0 = 24M bytes)
		while(VideoDecodeCreateInfo.ulNumDecodeSurfaces * uiWidthInPixels * uiHeightInPixels > 16 * 1024 * 1024){
				VideoDecodeCreateInfo.ulNumDecodeSurfaces--;
		}
		
		VideoDecodeCreateInfo.ChromaFormat        = cudaVideoChromaFormat_420;
		VideoDecodeCreateInfo.OutputFormat        = cudaVideoSurfaceFormat_NV12;
		VideoDecodeCreateInfo.DeinterlaceMode     = cudaVideoDeinterlaceMode_Adaptive;

		// No scaling
		VideoDecodeCreateInfo.ulTargetWidth       = uiWidthInPixels;
		VideoDecodeCreateInfo.ulTargetHeight      = uiHeightInPixels;
		// We won't simultaneously map more than 8 surfaces
		VideoDecodeCreateInfo.ulNumOutputSurfaces = NUM_CUDA_OUTPUT_SURFACES;
		// cudaVideoCreate_Default cudaVideoCreate_PreferCUDA cudaVideoCreate_PreferDXVA cudaVideoCreate_PreferCUVID
		VideoDecodeCreateInfo.ulCreationFlags     = cudaVideoCreate_PreferCUVID;
		VideoDecodeCreateInfo.vidLock             = VidCtxLock;
		
		// create the decoder
		CUresult Result = cuvidCreateDecoder(&m_pDecoder, &VideoDecodeCreateInfo);

		if(Result != CUDA_SUCCESS)
				return E_FAIL;

		return S_OK;
}

HRESULT CCudaDecoder::DecodePicture(CUVIDPICPARAMS* pPictureParameters){

		CUresult Result = cuvidDecodePicture(m_pDecoder, pPictureParameters);

		if(Result != CUDA_SUCCESS)
				return E_FAIL;

		return S_OK;
}

HRESULT CCudaDecoder::MapFrame(int iPictureIndex, CUdeviceptr* ppDevice, unsigned int* pPitch, CUVIDPROCPARAMS* pVideoProcessingParameters){
		
		CUresult Result = cuvidMapVideoFrame(m_pDecoder, iPictureIndex, ppDevice, pPitch, pVideoProcessingParameters);

		if(Result != CUDA_SUCCESS)
				return E_FAIL;

		return S_OK;
}

HRESULT CCudaDecoder::UnmapFrame(CUdeviceptr pDevice){

		CUresult Result = cuvidUnmapVideoFrame(m_pDecoder, pDevice);

		if(Result != CUDA_SUCCESS)
				return E_FAIL;

		return S_OK;
}