//----------------------------------------------------------------------------------------------
// Dxva2Decoder_Trace.cpp
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

#ifdef TRACE_DXVA2

void CDxva2Decoder::TraceConfigPictureDecode(){

	WCHAR szGuid1[40] = {0};
	WCHAR szGuid2[40] = {0};
	WCHAR szGuid3[40] = {0};

	StringFromGUID2(m_pConfigs->guidConfigBitstreamEncryption, szGuid1, 40);
	StringFromGUID2(m_pConfigs->guidConfigMBcontrolEncryption, szGuid2, 40);
	StringFromGUID2(m_pConfigs->guidConfigResidDiffEncryption, szGuid3, 40);

	TRACE((L"guidConfigBitstreamEncryption = %s", szGuid1));
	TRACE((L"guidConfigMBcontrolEncryption = %s", szGuid2));
	TRACE((L"guidConfigResidDiffEncryption = %s", szGuid3));
	TRACE((L"ConfigBitstreamRaw = %d", m_pConfigs->ConfigBitstreamRaw));
	TRACE((L"ConfigMBcontrolRasterOrder = %d", m_pConfigs->ConfigMBcontrolRasterOrder));
	TRACE((L"ConfigResidDiffHost = %d", m_pConfigs->ConfigResidDiffHost));
	TRACE((L"ConfigSpatialResid8 = %d", m_pConfigs->ConfigSpatialResid8));
	TRACE((L"ConfigResid8Subtraction = %d", m_pConfigs->ConfigResid8Subtraction));
	TRACE((L"ConfigSpatialHost8or9Clipping = %d", m_pConfigs->ConfigSpatialHost8or9Clipping));
	TRACE((L"ConfigSpatialResidInterleaved = %d", m_pConfigs->ConfigSpatialResidInterleaved));
	TRACE((L"ConfigIntraResidUnsigned = %d", m_pConfigs->ConfigIntraResidUnsigned));
	TRACE((L"ConfigResidDiffAccelerator = %d", m_pConfigs->ConfigResidDiffAccelerator));
	TRACE((L"ConfigHostInverseScan = %d", m_pConfigs->ConfigHostInverseScan));
	TRACE((L"ConfigSpecificIDCT = %d", m_pConfigs->ConfigSpecificIDCT));
	TRACE((L"Config4GroupedCoefs = %d", m_pConfigs->Config4GroupedCoefs));
	TRACE((L"ConfigMinRenderTargetBuffCount = %d", m_pConfigs->ConfigMinRenderTargetBuffCount));
	TRACE((L"ConfigDecoderSpecific = %d\n", m_pConfigs->ConfigDecoderSpecific));
}

void CDxva2Decoder::TracePictureParams(){

	if(m_VideoParam.picture_coding_type == 1)
		TRACE((L"PictureType : I"));
	else if(m_VideoParam.picture_coding_type == 2)
		TRACE((L"PictureType : P"));
	else if(m_VideoParam.picture_coding_type == 3)
		TRACE((L"PictureType : B"));
	else
		TRACE((L"PictureType : Unknown"));

	TRACE((L"wDecodedPictureIndex = %d", m_PictureParams.wDecodedPictureIndex));
	TRACE((L"wDeblockedPictureIndex = %d", m_PictureParams.wDeblockedPictureIndex));
	TRACE((L"wForwardRefPictureIndex = %d", m_PictureParams.wForwardRefPictureIndex));
	TRACE((L"wBackwardRefPictureIndex = %d", m_PictureParams.wBackwardRefPictureIndex));
	TRACE((L"wPicWidthInMBminus1 = %d", m_PictureParams.wPicWidthInMBminus1));
	TRACE((L"wPicHeightInMBminus1 = %d", m_PictureParams.wPicHeightInMBminus1));
	TRACE((L"bMacroblockWidthMinus1 = %d", m_PictureParams.bMacroblockWidthMinus1));
	TRACE((L"bMacroblockHeightMinus1 = %d", m_PictureParams.bMacroblockHeightMinus1));
	TRACE((L"bBlockWidthMinus1 = %d", m_PictureParams.bBlockWidthMinus1));
	TRACE((L"bBlockHeightMinus1 = %d", m_PictureParams.bBlockHeightMinus1));
	TRACE((L"bBPPminus1 = %d", m_PictureParams.bBPPminus1));
	TRACE((L"bPicStructure = %d", m_PictureParams.bPicStructure));
	TRACE((L"bSecondField = %d", m_PictureParams.bSecondField));
	TRACE((L"bPicIntra = %d", m_PictureParams.bPicIntra));
	TRACE((L"bPicBackwardPrediction = %d", m_PictureParams.bPicBackwardPrediction));
	TRACE((L"bBidirectionalAveragingMode = %d", m_PictureParams.bBidirectionalAveragingMode));
	TRACE((L"bMVprecisionAndChromaRelation = %d", m_PictureParams.bMVprecisionAndChromaRelation));
	TRACE((L"bChromaFormat = %d", m_PictureParams.bChromaFormat));
	TRACE((L"bPicScanFixed = %d", m_PictureParams.bPicScanFixed));
	TRACE((L"bPicScanMethod = %d", m_PictureParams.bPicScanMethod));
	TRACE((L"bPicReadbackRequests = %d", m_PictureParams.bPicReadbackRequests));
	TRACE((L"bRcontrol = %d", m_PictureParams.bRcontrol));
	TRACE((L"bPicSpatialResid8 = %d", m_PictureParams.bPicSpatialResid8));
	TRACE((L"bPicOverflowBlocks = %d", m_PictureParams.bPicOverflowBlocks));
	TRACE((L"bPicExtrapolation = %d", m_PictureParams.bPicExtrapolation));
	TRACE((L"bPicDeblocked = %d", m_PictureParams.bPicDeblocked));
	TRACE((L"bPicDeblockConfined = %d", m_PictureParams.bPicDeblockConfined));
	TRACE((L"bPic4MVallowed = %d", m_PictureParams.bPic4MVallowed));
	TRACE((L"bPicOBMC = %d", m_PictureParams.bPicOBMC));
	TRACE((L"bPicBinPB = %d", m_PictureParams.bPicBinPB));
	TRACE((L"bMV_RPS = %d", m_PictureParams.bMV_RPS));
	TRACE((L"bReservedBits = %d", m_PictureParams.bReservedBits));
	TRACE((L"wBitstreamFcodes = %d", m_PictureParams.wBitstreamFcodes));
	TRACE((L"wBitstreamPCEelements = %d", m_PictureParams.wBitstreamPCEelements));
	TRACE((L"bBitstreamConcealmentNeed = %d", m_PictureParams.bBitstreamConcealmentNeed));
	TRACE((L"bBitstreamConcealmentMethod = %d\n", m_PictureParams.bBitstreamConcealmentMethod));
}

void CDxva2Decoder::TraceQuantaMatrix(){

	TRACE((L"bNewQmatrix = %d : %d : %d : %d", m_QuantaMatrix.bNewQmatrix[0], m_QuantaMatrix.bNewQmatrix[1], m_QuantaMatrix.bNewQmatrix[2], m_QuantaMatrix.bNewQmatrix[3]));

	TRACE_NO_END_LINE((L"Intra = "));

	for(int i = 0; i < 64; i++){
		TRACE_NO_END_LINE((L"%d ", m_QuantaMatrix.Qmatrix[0][i]));
	}

	TRACE_NO_END_LINE((L"\nInter = "));

	for(int i = 0; i < 64; i++){
		TRACE_NO_END_LINE((L"%d ", m_QuantaMatrix.Qmatrix[1][i]));
	}

	TRACE_NO_END_LINE((L"\nChromaIntra = "));

	for(int i = 0; i < 64; i++){
		TRACE_NO_END_LINE((L"%d ", m_QuantaMatrix.Qmatrix[2][i]));
	}

	TRACE_NO_END_LINE((L"\nChromaInter = "));

	for(int i = 0; i < 64; i++){
		TRACE_NO_END_LINE((L"%d ", m_QuantaMatrix.Qmatrix[3][i]));
	}

	TRACE_NO_END_LINE((L"\n"));
}

void CDxva2Decoder::TraceSlices(){

	for(int i = 0; i < m_VideoParam.uiNumSlices; i++){

		TRACE((L"Slice%d", i));
		TRACE((L"wHorizontalPosition = %d", m_SliceInfo[i].wHorizontalPosition));
		TRACE((L"wVerticalPosition = %d", m_SliceInfo[i].wVerticalPosition));
		TRACE((L"dwSliceBitsInBuffer = %d", m_SliceInfo[i].dwSliceBitsInBuffer));
		TRACE((L"dwSliceDataLocation = %d", m_SliceInfo[i].dwSliceDataLocation));
		TRACE((L"bStartCodeBitOffset = %d", m_SliceInfo[i].bStartCodeBitOffset));
		TRACE((L"bReservedBits = %d", m_SliceInfo[i].bReservedBits));
		TRACE((L"wMBbitOffset = %d", m_SliceInfo[i].wMBbitOffset));
		TRACE((L"wNumberMBsInSlice = %d", m_SliceInfo[i].wNumberMBsInSlice));
		TRACE((L"wQuantizerScaleCode = %d", m_SliceInfo[i].wQuantizerScaleCode));
		TRACE((L"wBadSliceChopping = %d\n", m_SliceInfo[i].wBadSliceChopping));
	}
}

void CDxva2Decoder::TraceCompBuffers(){

	for(int i = 0; i < m_ExecuteParams.NumCompBuffers; i++){

		TRACE((L"CompressedBuffer%d", i));
		TRACE((L"CompressedBufferType = %d", m_ExecuteParams.pCompressedBuffers[i].CompressedBufferType));
		TRACE((L"BufferIndex = %d", m_ExecuteParams.pCompressedBuffers[i].BufferIndex));
		TRACE((L"DataOffset = %d", m_ExecuteParams.pCompressedBuffers[i].DataOffset));
		TRACE((L"DataSize = %d", m_ExecuteParams.pCompressedBuffers[i].DataSize));
		TRACE((L"FirstMBaddress = %d", m_ExecuteParams.pCompressedBuffers[i].FirstMBaddress));
		TRACE((L"NumMBsInBuffer = %d", m_ExecuteParams.pCompressedBuffers[i].NumMBsInBuffer));
		TRACE((L"Width = %d", m_ExecuteParams.pCompressedBuffers[i].Width));
		TRACE((L"Height = %d", m_ExecuteParams.pCompressedBuffers[i].Height));
		TRACE((L"Stride = %d", m_ExecuteParams.pCompressedBuffers[i].Stride));
		TRACE((L"ReservedBits = %d", m_ExecuteParams.pCompressedBuffers[i].ReservedBits));
		TRACE((L"pvPVPState = %d\n", (m_ExecuteParams.pCompressedBuffers[i].pvPVPState ? 1 : 0)));
	}
}

#endif