//-----------------------------------------------------------------------------------------------
// Mpeg12LibDefinition.h
// Copyright (C) 2013 Dumonteil David
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

// This program use lipmp2dec and libmpeg123 source code :
// https://sourceforge.net/projects/libmp2codec/
// http://www.mpg123.de/
//----------------------------------------------------------------------------------------------
#ifndef MPEG12LIBDEFINITION_H
#define MPEG12LIBDEFINITION_H

// max compressed frame size
#define MPA_MAX_CODED_FRAME_SIZE 1792
#define MPA_MAX_CHANNELS 2
#define MAX_AUDIO_BUFFER_SIZE 4608

#define BACKSTEP_SIZE 512

// number of subbands
#define SBLIMIT 32

// fractional bits for sb_samples and dct
#define FRAC_BITS   15

#if FRAC_BITS <= 15
typedef short MPA_INT;
#else
typedef int MPA_INT;
#endif

struct GetBitContext{
		
		const unsigned char* buffer;
		const unsigned char* buffer_end;
		unsigned char* buffer_ptr;
		unsigned int cache;
		int bit_count;
		int size_in_bits;
};

struct MPADecodeContext{

		int bit_rate;
		int nb_channels;
		int sample_rate;
		int sample_rate_index; // between 0 and 8
		int lsf;
		int mode;
		int mode_ext;
		int error_protection;
		int layer;
		int frame_size;
		
		// input buffer
		unsigned char inbuf1[2][MPA_MAX_CODED_FRAME_SIZE + BACKSTEP_SIZE];
		int inbuf_index;
		unsigned char* inbuf_ptr;
		unsigned char* inbuf;

		int sb_samples[MPA_MAX_CHANNELS][36][SBLIMIT];

		int synth_buf_offset[MPA_MAX_CHANNELS];

		MPA_INT synth_buf[MPA_MAX_CHANNELS][512 * 2];

		GetBitContext gb;
};

#endif