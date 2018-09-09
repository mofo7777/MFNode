//-----------------------------------------------------------------------------------------------
// Mp2Decoder.h
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
#ifndef MPEG12DECODER_H
#define MPEG12DECODER_H

class CMpeg12Decoder{

public:

	CMpeg12Decoder(){}
	~CMpeg12Decoder(){}

	void Initialize();
	HRESULT	DecodeFrame(BYTE*, const UINT, UINT*, short*, UINT*);

private:

	int mpa_decode_header(MPADecodeContext*, unsigned int);
	int check_header(unsigned int);
	int decode_header(MPADecodeContext*, unsigned int);
	int mp_decode_frame(MPADecodeContext*, short*);
	int mp_decode_layer1(MPADecodeContext*);
	int mp_decode_layer2(MPADecodeContext*);
	void synth_filter(MPADecodeContext*, int, short*, int, int sb_samples[SBLIMIT]);
	int l1_unscale(int, int, int);
	int l2_unscale_group(int, int, int);
	void dct32(int*, int*);

	// Mpeg12Decoder_Bits.cpp
	void init_get_bits(GetBitContext*, unsigned char*, int);
	unsigned int get_bits(GetBitContext*, int);

	MPADecodeContext m_ctx;
};

#endif