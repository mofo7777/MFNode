//----------------------------------------------------------------------------------------------
// Mpeg12Decoder_Bits.cpp
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
#include "StdAfx.h"

void CMpeg12Decoder::init_get_bits(GetBitContext* s, unsigned char* buffer, int bit_size){

	const int buffer_size = (bit_size + 7) >> 3;

	s->buffer = buffer;
	s->size_in_bits = bit_size;
	s->buffer_end = buffer + buffer_size;
	s->buffer_ptr = buffer;
	s->bit_count = 16;
	s->cache = 0;

	int re_bit_count = s->bit_count;
	int re_cache = s->cache;
	unsigned char* re_buffer_ptr = s->buffer_ptr;

	if(re_bit_count >= 0){

		re_cache += (int)bswap_16(*(unsigned short*)re_buffer_ptr) << re_bit_count;
		re_buffer_ptr += 2;
		re_bit_count -= 16;
	}

	if(re_bit_count >= 0){

		re_cache += (int)bswap_16(*(unsigned short*)re_buffer_ptr) << re_bit_count;
		re_buffer_ptr += 2;
		re_bit_count -= 16;
	}

	s->bit_count = re_bit_count;
	s->cache = re_cache;
	s->buffer_ptr = re_buffer_ptr;
}

unsigned int CMpeg12Decoder::get_bits(GetBitContext *s, int n){

	register int tmp;
	int re_bit_count = s->bit_count;
	int re_cache = s->cache;
	unsigned char* re_buffer_ptr = s->buffer_ptr;

	if(re_bit_count >= 0){

		re_cache += (int)bswap_16(*(unsigned short*)re_buffer_ptr) << re_bit_count;
		//((unsigned short*)re_buffer_ptr)++; 
		re_buffer_ptr += 2;
		re_bit_count -= 16;
	}

	tmp = (((unsigned int)(re_cache)) >> (32 - (n)));

	re_cache <<= (n);
	re_bit_count += (n);
	s->bit_count = re_bit_count;
	s->cache = re_cache;
	s->buffer_ptr = re_buffer_ptr;

	return tmp;
}