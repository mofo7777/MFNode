//----------------------------------------------------------------------------------------------
// MFTCudaDecoder_Def.h
//----------------------------------------------------------------------------------------------
#ifndef MFTCUDADECODER_DEF_H
#define MFTCUDADECODER_DEF_H

#define MAX_CUDA_DECODED_SURFACES  20
#define NUM_CUDA_OUTPUT_SURFACES   1

struct SCUDAFRAME{

	int iTemporal;
	int iPType;
	int iIndex;
	REFERENCE_TIME rtTime;
};

#endif