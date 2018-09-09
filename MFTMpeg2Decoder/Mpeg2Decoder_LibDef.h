//----------------------------------------------------------------------------------------------
// Mpeg2Decoder_LibDef.h
// Copyright (C) 2012 Dumonteil David
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
#ifndef MP2DECODER_LIBDEF_H
#define MP2DECODER_LIBDEF_H

#define MPEG2_ACCEL_X86_MMX    1
#define MPEG2_ACCEL_X86_3DNOW  2
#define MPEG2_ACCEL_X86_MMXEXT 4
#define MPEG2_ACCEL_X86_SSE2   8
#define MPEG2_ACCEL_X86_SSE3   16
#define MPEG2_ACCEL_DETECT     0x80000000

typedef enum{

		STATE_BUFFER            = 0,
		STATE_SEQUENCE          = 1,
		STATE_SEQUENCE_REPEATED = 2,
		STATE_GOP               = 3,
		STATE_PICTURE           = 4,
		STATE_SLICE_1ST         = 5,
		STATE_PICTURE_2ND       = 6,
		STATE_SLICE             = 7,
		STATE_END               = 8,
		STATE_INVALID           = 9,
		STATE_INVALID_END       = 10,
		STATE_SEQUENCE_MODIFIED = 11

}mpeg2_state_t;

typedef struct mpeg2dec_s mpeg2dec_t;
typedef struct mpeg2_decoder_s mpeg2_decoder_t;

typedef struct{
		
		unsigned char* ref[2][3];
		unsigned char** ref2[2];
		int pmv[2][2];
		int f_code[2];

}motion_t;

typedef void mpeg2_mc_fct(unsigned char*, const unsigned char*, int, int);
typedef void motion_parser_t(mpeg2_decoder_t* decoder, motion_t* motion, mpeg2_mc_fct* const* table);

struct mpeg2_decoder_s{

		// first, state that carries information from one macroblock to the
		// next inside a slice, and is never used outside of mpeg2_slice()

		// bit parsing stuff
		unsigned int bitstream_buf;		// current 32 bit working set
		int bitstream_bits;			// used bits in working set
		const unsigned char* bitstream_ptr;	// buffer with stream data

		unsigned char* dest[3];

		int offset;
		int stride;
		int uv_stride;
  int slice_stride;
  int slice_uv_stride;
  int stride_frame;
  unsigned int limit_x;
  unsigned int limit_y_16;
  unsigned int limit_y_8;
  unsigned int limit_y;

  // Motion vectors
  // The f_ and b_ correspond to the forward and backward motion
  // predictors
  motion_t b_motion;
  motion_t f_motion;
  motion_parser_t* motion_parser[5];

  // predictor for DC coefficients in intra blocks
  short dc_dct_pred[3];

  // DCT coefficients
  short DCTblock[64];

  unsigned char* picture_dest[3];
  void (*convert) (void * convert_id, unsigned char* const * src, unsigned int v_offset);
		void * convert_id;

  int dmv_offset;
  unsigned int v_offset;

  // now non-slice-specific information

  // sequence header stuff
  unsigned short* quantizer_matrix[4];
  unsigned short (* chroma_quantizer[2])[64];
  unsigned short quantizer_prescale[4][32][64];

  // The width and height of the picture snapped to macroblock units
  int width;
  int height;
  int vertical_position_extension;
  int chroma_format;

  // picture header stuff

  // what type of picture this is (I, P, B, D)
  int coding_type;

  // picture coding extension stuff

  // quantization factor for intra dc coefficients
  int intra_dc_precision;
  // top/bottom/both fields
  int picture_structure;
  // bool to indicate all predictions are frame based
  int frame_pred_frame_dct;
  // bool to indicate whether intra blocks have motion vectors
  // (for concealment)
  int concealment_motion_vectors;
  // bool to use different vlc tables
  int intra_vlc_format;
  // used for DMV MC
  int top_field_first;

  // stuff derived from bitstream

  // pointer to the zigzag scan we're supposed to be using
  const unsigned char* scan;

  int second_field;

  int mpeg1;

  // XXX: stuff due to xine shit
  char q_scale_type;
};

typedef struct mpeg2_sequence_s{

		unsigned int width, height;
		unsigned int chroma_width, chroma_height;
		unsigned int byte_rate;
		unsigned int vbv_buffer_size;
		unsigned int flags;

		unsigned int picture_width, picture_height;
		unsigned int display_width, display_height;
		unsigned int pixel_width, pixel_height;
		unsigned int frame_period;

		unsigned char profile_level_id;
		unsigned char colour_primaries;
		unsigned char transfer_characteristics;
		unsigned char matrix_coefficients;

}mpeg2_sequence_t;

typedef struct mpeg2_gop_s{

		unsigned char hours;
		unsigned char minutes;
		unsigned char seconds;
		unsigned char pictures;
		unsigned int flags;

}mpeg2_gop_t;

typedef struct mpeg2_picture_s{

		unsigned int temporal_reference;
		unsigned int nb_fields;
		unsigned int tag, tag2;
		unsigned int flags;
		
		struct{

				int x, y;

		}display_offset[3];

}mpeg2_picture_t;

typedef struct mpeg2_fbuf_s{
		
		unsigned char* buf[3];
		void* id;

}mpeg2_fbuf_t;

typedef struct{

		mpeg2_fbuf_t fbuf;

}fbuf_alloc_t;

struct mpeg2dec_s{

		mpeg2_decoder_t decoder;

		//mpeg2_info_t info;

		unsigned int shift;
		int is_display_initialized;
		mpeg2_state_t (*action)(struct mpeg2dec_s* mpeg2dec);
		mpeg2_state_t state;
		unsigned int ext_state;

		// allocated in init - gcc has problems allocating such big structures
		unsigned char* chunk_buffer;
		// pointer to start of the current chunk
		unsigned char* chunk_start;
		// pointer to current position in chunk_buffer
		unsigned char* chunk_ptr;
		// last start code ?
		unsigned char code;

		// picture tags
		unsigned int tag_current, tag2_current, tag_previous, tag2_previous;
		int num_tags;
		int bytes_since_tag;

		int first;
		int alloc_index_user;
		int alloc_index;
		unsigned char first_decode_slice;
		unsigned char nb_decode_slices;

		//unsigned int user_data_len;// david no need

		mpeg2_sequence_t new_sequence;
		mpeg2_sequence_t sequence;
		mpeg2_gop_t new_gop;
		mpeg2_gop_t gop;
		mpeg2_picture_t new_picture;
		mpeg2_picture_t pictures[4];
		mpeg2_picture_t* picture;
		/*const*/mpeg2_fbuf_t* fbuf[3];	// 0: current fbuf, 1-2: prediction fbufs

		fbuf_alloc_t fbuf_alloc[3];
		int custom_fbuf;

		unsigned char* yuv_buf[3][3];
		int yuv_index;
		//mpeg2_convert_t* convert;// david no need
		void* convert_arg;
		unsigned int convert_id_size;
		int convert_stride;
		void(*convert_start) (void* id, const mpeg2_fbuf_t* fbuf, const mpeg2_picture_t* picture, const mpeg2_gop_t* gop);

		unsigned char* buf_start;
		unsigned char* buf_end;

		short display_offset_x, display_offset_y;

		int copy_matrix;
		char scaled[4]; // XXX: MOVED
		//int8_t q_scale_type, scaled[4];
		unsigned char quantizer_matrix[4][64];
		unsigned char new_quantizer_matrix[4][64];
};

typedef struct mpeg2_info_s{

		const mpeg2_sequence_t* sequence;
		const mpeg2_gop_t* gop;
		const mpeg2_picture_t* current_picture;
		const mpeg2_picture_t* current_picture_2nd;
		const mpeg2_fbuf_t* current_fbuf;
		const mpeg2_picture_t* display_picture;
		const mpeg2_picture_t* display_picture_2nd;
		const mpeg2_fbuf_t* display_fbuf;
		const mpeg2_fbuf_t* discard_fbuf;
		const unsigned char* user_data;
		unsigned int user_data_len;

}mpeg2_info_t;

extern "C" mpeg2dec_t* mpeg2_init();
extern "C" void mpeg2_close(mpeg2dec_t*);
extern "C" void mpeg2_buffer(mpeg2dec_t*, unsigned char*, unsigned char*);
extern "C"  mpeg2_state_t mpeg2_parse(mpeg2dec_t* mpeg2dec);
extern "C"  const mpeg2_info_t* mpeg2_info(mpeg2dec_t*);
extern "C"  unsigned int mpeg2_accel (unsigned int);

#endif