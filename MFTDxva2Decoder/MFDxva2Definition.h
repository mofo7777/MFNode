//----------------------------------------------------------------------------------------------
// MFDxva2Definition.h
//----------------------------------------------------------------------------------------------
#ifndef MFDXVA2DEFINITION_H
#define MFDXVA2DEFINITION_H

#define NUM_DXVA2_SURFACE 24

struct VIDEO_PARAMS{

	unsigned int uiNumSlices;
	unsigned int uiBitstreamDataLen;
	const unsigned char* pBitstreamData;
	const unsigned int* pSliceDataOffsets;

	int iCurPictureId;
	int iForwardRefIdx;
	int iBackwardRefIdx;
	int PicWidthInMbs;
	int FrameHeightInMbs;

	int field_pic_flag;
	int bottom_field_flag;
	int second_field;

	int ref_pic_flag;
	int intra_pic_flag;
	int picture_coding_type;
	int f_code[2][2];
	int intra_dc_precision;
	int picture_structure;
	int top_field_first;
	int frame_pred_frame_dct;
	int concealment_motion_vectors;
	int q_scale_type;
	int intra_vlc_format;
	int alternate_scan;
	int repeat_first_field;
	int chroma_420_type;
	int progressive_frame;
	int full_pel_forward_vector;
	int full_pel_backward_vector;

	unsigned char QuantMatrixIntra[64];
	unsigned char QuantMatrixInter[64];
};

struct STemporalRef{

	int iTemporal;
	int iPType;
	int iIndex;
	REFERENCE_TIME rtTime;
};

#endif