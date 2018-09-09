//----------------------------------------------------------------------------------------------
// Mpeg12Decoder.cpp
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
#include "stdafx.h"
#include "Mpeg12LibTab.h"

// lower 2 bits: modulo 3, higher bits: shift
static unsigned short scale_factor_modshift[64];
// [i][j]:  2^(-j/3) * FRAC_ONE * 2^(i+2) / (2^(i+2) - 1)
static int scale_factor_mult[15][3];
// mult table for layer 2 group quantization

static int scale_factor_mult2[3][3] = {
    SCALE_GEN(4.0 / 3.0), // 3 steps
    SCALE_GEN(4.0 / 5.0), // 5 steps
    SCALE_GEN(4.0 / 9.0), // 9 steps
};

static MPA_INT window[512];

static inline int round_sample(int sum){

		int sum1;
		
		sum1 = (sum + (1 << (OUT_SHIFT - 1))) >> OUT_SHIFT;
		
		if(sum1 < -32768)
				sum1 = -32768;
		else if(sum1 > 32767)
				sum1 = 32767;
		return sum1;
}

static inline int l2_select_table(int bitrate, int nb_channels, int freq, int lsf){

		int ch_bitrate, table;

		ch_bitrate = bitrate / nb_channels;
		
		if(!lsf){

				if((freq == 48000 && ch_bitrate >= 56) || (ch_bitrate >= 56 && ch_bitrate <= 80)) 
						table = 0;
				else if(freq != 48000 && ch_bitrate >= 96)
						table = 1;
				else if(freq != 32000 && ch_bitrate <= 48) 
						table = 2;
				else 
						table = 3;
		}
		else{
				table = 4;
		}
		return table;
}

void CMpeg12Decoder::Initialize(){

		MPADecodeContext* s;
		static int init = 0;
		int i;

		s = &m_ctx;
		memset(s, 0, sizeof(MPADecodeContext));

		s->inbuf_index = 0;
		s->inbuf = &s->inbuf1[s->inbuf_index][BACKSTEP_SIZE];
		s->inbuf_ptr = s->inbuf;

		if(init){
				return;
		}

		// scale factors table for layer 1/2
		for(i = 0; i < 64; i++){

				int shift, mod;
				// 1.0 (i = 3) is normalized to 2 ^ FRAC_BITS
				shift = (i / 3);
				mod = i % 3;
				scale_factor_modshift[i] = mod | (shift << 2);
		}

		// scale factor multiply for layer 1
		for(i = 0; i < 15; i++){

				int n, norm;
				n = i + 2;
				norm = ((int64_t_C(1) << n) * FRAC_ONE) / ((1 << n) - 1);
				scale_factor_mult[i][0] = MULL(FIXR(1.0 * 2.0), norm);
				scale_factor_mult[i][1] = MULL(FIXR(0.7937005259 * 2.0), norm);
				scale_factor_mult[i][2] = MULL(FIXR(0.6299605249 * 2.0), norm);
		}

		// window max = 18760, max sum over all 16 coefs : 44736
		for(i = 0; i < 257; i++){

				int v;
				v = mpa_enwindow[i];

    v = (v + (1 << (16 - WFRAC_BITS - 1))) >> (16 - WFRAC_BITS);

				window[i] = v;

				if((i & 63) != 0)
						v = -v;
				if(i != 0)
						window[512 - i] = v;
		}

		init = 1;
}

HRESULT CMpeg12Decoder::DecodeFrame(BYTE* pBuffer, const UINT BufferSize, UINT* pFrameSize, short* pData, UINT* pDataSize){

		*pFrameSize = 0;
		*pDataSize = 0;

		if(BufferSize < 4){
				return S_FALSE;
		}

		MPADecodeContext* s = &m_ctx;

		if(mpa_decode_header(s, *((unsigned int*)pBuffer)) < 0){
				return E_FAIL;
		}

		if(BufferSize < s->frame_size){

				*pFrameSize = 0;
				return S_FALSE;
		}

		*pFrameSize = s->frame_size;

		unsigned char* buf_ptr;
		int len, out_size;
		short* out_samples = (short*)pData;

		buf_ptr = pBuffer;

		len = s->inbuf_ptr - s->inbuf;

		if(len != 0){

				return E_FAIL;
		}

		if(s->frame_size > MPA_MAX_CODED_FRAME_SIZE)
				s->frame_size = MPA_MAX_CODED_FRAME_SIZE;

		len = s->frame_size - len;

		memcpy(s->inbuf_ptr, buf_ptr, s->frame_size);
		buf_ptr += len;
		s->inbuf_ptr += len;

		if(s->frame_size > 0 && (s->inbuf_ptr - s->inbuf) >= s->frame_size){

				out_size = mp_decode_frame(s, out_samples);

				s->inbuf_ptr = s->inbuf;
				*pFrameSize = s->frame_size;
				s->frame_size = 0;
				*pDataSize = out_size;
				return S_OK;
		}
		else{

				return E_FAIL;
		}
}

int CMpeg12Decoder::mpa_decode_header(MPADecodeContext* avctx, unsigned int head){

		unsigned int nHead = SWAP32(head);

		MPADecodeContext* s = avctx;

		if(check_header(nHead) != 0){
				return -1;
		}

		if(decode_header(s, nHead) != 0){
				return -1;
		}

		if(s->layer != 1 && s->layer != 2){
				return -1;
		}

		return s->frame_size;
}

// fast header check for resync
int CMpeg12Decoder::check_header(unsigned int header){

		// header
		if((header & 0xffe00000) != 0xffe00000)
				return -1;
		// layer check
		if(((header >> 17) & 3) == 0)
				return -1;
		// bit rate
		if(((header >> 12) & 0xf) == 0xf)
				return -1;
		// frequency
		if(((header >> 10) & 3) == 3)
				return -1;
		return 0;
}

// header + layer + bitrate + freq + lsf/mpeg25
//#define SAME_HEADER_MASK (0xffe00000 | (3 << 17) | (0xf << 12) | (3 << 10) | (3 << 19))

// header decoding. MUST check the header before because no
// consistency check is done there. Return 1 if free format found and
// that the frame size must be computed externally
int CMpeg12Decoder::decode_header(MPADecodeContext* s, unsigned int header){

		int sample_rate, frame_size, mpeg25, padding;
		int sample_rate_index, bitrate_index;
		
		if(header & (1<<20)){
				s->lsf = (header & (1<<19)) ? 0 : 1;
				mpeg25 = 0;
		}
		else{
				s->lsf = 1;
				mpeg25 = 1;
		}

		s->layer = 4 - ((header >> 17) & 3);

		// extract frequency
		sample_rate_index = (header >> 10) & 3;
		sample_rate = mpa_freq_tab[sample_rate_index] >> (s->lsf + mpeg25);
		sample_rate_index += 3 * (s->lsf + mpeg25);

		s->sample_rate_index = sample_rate_index;
		s->error_protection = ((header >> 16) & 1) ^ 1;
		s->sample_rate = sample_rate;

		bitrate_index = (header >> 12) & 0xf;
		padding = (header >> 9) & 1;
		s->mode = (header >> 6) & 3;
		s->mode_ext = (header >> 4) & 3;

		if(s->mode == MPA_MONO)
				s->nb_channels = 1;
		else
				s->nb_channels = 2;

		if(bitrate_index != 0){

				frame_size = mpa_bitrate_tab[s->lsf][s->layer - 1][bitrate_index];
				s->bit_rate = frame_size * 1000;

				switch(s->layer){

						case 1:
								frame_size = (frame_size * 12000) / sample_rate;
								frame_size = (frame_size + padding) * 4;
								break;
						
						case 2:
								frame_size = (frame_size * 144000) / sample_rate;
								frame_size += padding;
								break;
						
						default:
						case 3:
								frame_size = (frame_size * 144000) / (sample_rate << s->lsf);
								frame_size += padding;
								break;
				}
				s->frame_size = frame_size;
		}
		else{

				// This decoder do not handle free format...
				return 1;

				// if no frame size computed, signal it
				/*if(!s->free_format_frame_size)
						return 1;
				// free format: compute bitrate and real frame size from the
				//frame size we extracted by reading the bitstream
				s->frame_size = s->free_format_frame_size;
				
				switch(s->layer){
				
				  case 1:
								s->frame_size += padding  * 4;
								s->bit_rate = (s->frame_size * sample_rate) / 48000;
								break;
						
						case 2:
								s->frame_size += padding;
								s->bit_rate = (s->frame_size * sample_rate) / 144000;
								break;
						
						default:
						case 3:
								s->frame_size += padding;
								s->bit_rate = (s->frame_size * (sample_rate << s->lsf)) / 144000;
								break;
				}*/
		}
		return 0;
}

int CMpeg12Decoder::mp_decode_frame(MPADecodeContext* s, short* samples){

		int i, nb_frames, ch;
		short *samples_ptr;

		init_get_bits(&s->gb, s->inbuf + HEADER_SIZE, (s->inbuf_ptr - s->inbuf - HEADER_SIZE) * 8);

		// skip error protection field
		if(s->error_protection)
				get_bits(&s->gb, 16);

		switch(s->layer){
		
		  case 1:
						nb_frames = mp_decode_layer1(s);
						break;
		
		  case 2:
						nb_frames = mp_decode_layer2(s);
						break;

    default:
						nb_frames = 0;
		}

		// apply the synthesis filter
		for(ch = 0; ch < s->nb_channels; ch++){
				
				samples_ptr = samples + ch;
				
				for(i = 0; i < nb_frames; i++){
						
						synth_filter(s, ch, samples_ptr, s->nb_channels, s->sb_samples[ch][i]);
						samples_ptr += 32 * s->nb_channels;
				}
		}

		return nb_frames * 32 * sizeof(short) * s->nb_channels;
}

int CMpeg12Decoder::mp_decode_layer1(MPADecodeContext* s){

		int bound, i, v, n, ch, j, mant;
		unsigned char allocation[MPA_MAX_CHANNELS][SBLIMIT];
		unsigned char scale_factors[MPA_MAX_CHANNELS][SBLIMIT];

		if(s->mode == MPA_JSTEREO)
				bound = (s->mode_ext + 1) * 4;
		else
				bound = SBLIMIT;

		// allocation bits
		for(i = 0; i < bound; i++){
				for(ch = 0; ch < s->nb_channels; ch++){
						allocation[ch][i] = get_bits(&s->gb, 4);
				}
		}
		
		for(i = bound; i < SBLIMIT; i++){
				allocation[0][i] = get_bits(&s->gb, 4);
		}

		// scale factors
		for(i = 0; i < bound; i++){
				for(ch = 0; ch < s->nb_channels; ch++){
						
						if(allocation[ch][i])
								scale_factors[ch][i] = get_bits(&s->gb, 6);
				}
		}
		
		for(i = bound; i < SBLIMIT; i++){
				
				if(allocation[0][i]){
						scale_factors[0][i] = get_bits(&s->gb, 6);
						scale_factors[1][i] = get_bits(&s->gb, 6);
				}
		}

		// compute samples
		for(j = 0; j < 12; j++){

				for(i = 0; i < bound; i++){
						for(ch = 0; ch < s->nb_channels; ch++){
								
								n = allocation[ch][i];
								
								if(n){
										mant = get_bits(&s->gb, n + 1);
										v = l1_unscale(n, mant, scale_factors[ch][i]);
								}
								else{
										v = 0;
								}
								s->sb_samples[ch][j][i] = v;
						}
				}
				
				for(i = bound; i < SBLIMIT;i++){
						
						n = allocation[0][i];
						
						if(n){
								
								mant = get_bits(&s->gb, n + 1);
								v = l1_unscale(n, mant, scale_factors[0][i]);
								s->sb_samples[0][j][i] = v;
								v = l1_unscale(n, mant, scale_factors[1][i]);
								s->sb_samples[1][j][i] = v;
						}
						else{
								s->sb_samples[0][j][i] = 0;
								s->sb_samples[1][j][i] = 0;
						}
				}
		}
		
		return 12;
}

int CMpeg12Decoder::mp_decode_layer2(MPADecodeContext* s){

		int sblimit; // number of used subbands
		const unsigned char *alloc_table;
		int table, bit_alloc_bits, i, j, ch, bound, v;
		unsigned char bit_alloc[MPA_MAX_CHANNELS][SBLIMIT];
		unsigned char scale_code[MPA_MAX_CHANNELS][SBLIMIT];
		unsigned char scale_factors[MPA_MAX_CHANNELS][SBLIMIT][3], *sf;
		int scale, qindex, bits, steps, k, l, m, b;

		// select decoding table
		table = l2_select_table(s->bit_rate / 1000, s->nb_channels, s->sample_rate, s->lsf);
		sblimit = sblimit_table[table];
		alloc_table = alloc_tables[table];

		if(s->mode == MPA_JSTEREO) 
				bound = (s->mode_ext + 1) * 4;
		else
				bound = sblimit;

		// sanity check
		if(bound > sblimit)
				bound = sblimit;

		// parse bit allocation
		j = 0;
		
		for(i=0;i<bound;i++) {
        bit_alloc_bits = alloc_table[j];
        for(ch=0;ch<s->nb_channels;ch++) {
            bit_alloc[ch][i] = get_bits(&s->gb, bit_alloc_bits);
        }
        j += 1 << bit_alloc_bits;
    }
    for(i=bound;i<sblimit;i++) {
        bit_alloc_bits = alloc_table[j];
        v = get_bits(&s->gb, bit_alloc_bits);
        bit_alloc[0][i] = v;
        bit_alloc[1][i] = v;
        j += 1 << bit_alloc_bits;
    }

    // scale codes
    for(i=0;i<sblimit;i++) {
        for(ch=0;ch<s->nb_channels;ch++) {
            if (bit_alloc[ch][i]) 
                scale_code[ch][i] = get_bits(&s->gb, 2);
        }
    }
    
    // scale factors
    for(i=0;i<sblimit;i++) {

        for(ch=0;ch<s->nb_channels;ch++) {

            if (bit_alloc[ch][i]) {

                sf = scale_factors[ch][i];

                switch(scale_code[ch][i]) {

                default:
                case 0:
                    sf[0] = get_bits(&s->gb, 6);
                    sf[1] = get_bits(&s->gb, 6);
                    sf[2] = get_bits(&s->gb, 6);
                    break;
                case 2:
                    sf[0] = get_bits(&s->gb, 6);
                    sf[1] = sf[0];
                    sf[2] = sf[0];
                    break;
                case 1:
                    sf[0] = get_bits(&s->gb, 6);
                    sf[2] = get_bits(&s->gb, 6);
                    sf[1] = sf[0];
                    break;
                case 3:
                    sf[0] = get_bits(&s->gb, 6);
                    sf[2] = get_bits(&s->gb, 6);
                    sf[1] = sf[2];
                    break;
                }
            }
        }
    }

   // samples
    for(k=0;k<3;k++) {
        for(l=0;l<12;l+=3) {
            j = 0;
            for(i=0;i<bound;i++) {
                bit_alloc_bits = alloc_table[j];
                for(ch=0;ch<s->nb_channels;ch++) {
                    b = bit_alloc[ch][i];
                    if (b) {
                        scale = scale_factors[ch][i][k];
                        qindex = alloc_table[j+b];
                        bits = quant_bits[qindex];
                        if (bits < 0) {
                            // 3 values at the same time
                            v = get_bits(&s->gb, -bits);
                            steps = quant_steps[qindex];
                            s->sb_samples[ch][k * 12 + l + 0][i] = 
                                l2_unscale_group(steps, v % steps, scale);
                            v = v / steps;
                            s->sb_samples[ch][k * 12 + l + 1][i] = 
                                l2_unscale_group(steps, v % steps, scale);
                            v = v / steps;
                            s->sb_samples[ch][k * 12 + l + 2][i] = 
                                l2_unscale_group(steps, v, scale);
                        } else {
                            for(m=0;m<3;m++) {
                                v = get_bits(&s->gb, bits);
                                v = l1_unscale(bits - 1, v, scale);
                                s->sb_samples[ch][k * 12 + l + m][i] = v;
                            }
                        }
                    } else {
                        s->sb_samples[ch][k * 12 + l + 0][i] = 0;
                        s->sb_samples[ch][k * 12 + l + 1][i] = 0;
                        s->sb_samples[ch][k * 12 + l + 2][i] = 0;
                    }
                }
                // next subband in alloc table
                j += 1 << bit_alloc_bits; 
            }
            // XXX: find a way to avoid this duplication of code
            for(i=bound;i<sblimit;i++) {
                bit_alloc_bits = alloc_table[j];
                b = bit_alloc[0][i];
                if (b) {
                    int mant, scale0, scale1;
                    scale0 = scale_factors[0][i][k];
                    scale1 = scale_factors[1][i][k];
                    qindex = alloc_table[j+b];
                    bits = quant_bits[qindex];
                    if (bits < 0) {
                        // 3 values at the same time
                        v = get_bits(&s->gb, -bits);
                        steps = quant_steps[qindex];
                        mant = v % steps;
                        v = v / steps;
                        s->sb_samples[0][k * 12 + l + 0][i] = 
                            l2_unscale_group(steps, mant, scale0);
                        s->sb_samples[1][k * 12 + l + 0][i] = 
                            l2_unscale_group(steps, mant, scale1);
                        mant = v % steps;
                        v = v / steps;
                        s->sb_samples[0][k * 12 + l + 1][i] = 
                            l2_unscale_group(steps, mant, scale0);
                        s->sb_samples[1][k * 12 + l + 1][i] = 
                            l2_unscale_group(steps, mant, scale1);
                        s->sb_samples[0][k * 12 + l + 2][i] = 
                            l2_unscale_group(steps, v, scale0);
                        s->sb_samples[1][k * 12 + l + 2][i] = 
                            l2_unscale_group(steps, v, scale1);
                    } else {
                        for(m=0;m<3;m++) {
                            mant = get_bits(&s->gb, bits);
                            s->sb_samples[0][k * 12 + l + m][i] = 
                                l1_unscale(bits - 1, mant, scale0);
                            s->sb_samples[1][k * 12 + l + m][i] = 
                                l1_unscale(bits - 1, mant, scale1);
                        }
                    }
                } else {
                    s->sb_samples[0][k * 12 + l + 0][i] = 0;
                    s->sb_samples[0][k * 12 + l + 1][i] = 0;
                    s->sb_samples[0][k * 12 + l + 2][i] = 0;
                    s->sb_samples[1][k * 12 + l + 0][i] = 0;
                    s->sb_samples[1][k * 12 + l + 1][i] = 0;
                    s->sb_samples[1][k * 12 + l + 2][i] = 0;
                }
                // next subband in alloc table
                j += 1 << bit_alloc_bits; 
            }
            // fill remaining samples to zero
            for(i=sblimit;i<SBLIMIT;i++) {
                for(ch=0;ch<s->nb_channels;ch++) {
                    s->sb_samples[ch][k * 12 + l + 0][i] = 0;
                    s->sb_samples[ch][k * 12 + l + 1][i] = 0;
                    s->sb_samples[ch][k * 12 + l + 2][i] = 0;
                }
            }
        }
    }
    return 3 * 12;
}

// 32 sub band synthesis filter. Input: 32 sub band samples, Output: 32 samples.
// XXX: optimize by avoiding ring buffer usage
void CMpeg12Decoder::synth_filter(MPADecodeContext* s1, int ch, short* samples, int incr, int sb_samples[SBLIMIT]){

		int tmp[32];
		register MPA_INT* synth_buf;
		register const MPA_INT *w, *w2, *p;
		int j, offset, v;
		short *samples2;

  #if FRAC_BITS <= 15
		  int sum, sum2;
  #else
		  int64_t sum, sum2;
  #endif

		dct32(tmp, sb_samples);

		offset = s1->synth_buf_offset[ch];
		synth_buf = s1->synth_buf[ch] + offset;

		for(j = 0; j < 32; j++){

				v = tmp[j];

    #if FRAC_BITS <= 15
		  // NOTE: can cause a loss in precision if very high amplitude sound
		  if(v > 32767)
				  v = 32767;
		  else if(v < -32768)
				  v = -32768;
    #endif

				synth_buf[j] = v;
		}
		
		// copy to avoid wrap
		memcpy(synth_buf + 512, synth_buf, 32 * sizeof(MPA_INT));

		samples2 = samples + 31 * incr;
		w = window;
		w2 = window + 31;

		sum = 0;
		p = synth_buf + 16;
		SUM8(sum, +=, w, p);
		p = synth_buf + 48;
		SUM8(sum, -=, w + 32, p);
		*samples = round_sample(sum);
		samples += incr;
		w++;

		// we calculate two samples at the same time to avoid one memory access per two sample
		for(j = 1; j < 16;j++){

				sum = 0;
				sum2 = 0;
				p = synth_buf + 16 + j;
				SUM8P2(sum, +=, sum2, -=, w, w2, p);
				p = synth_buf + 48 - j;
				SUM8P2(sum, -=, sum2, -=, w + 32, w2 + 32, p);

				*samples = round_sample(sum);
				samples += incr;
				*samples2 = round_sample(sum2);
				samples2 -= incr;
				w++;
				w2--;
		}

		p = synth_buf + 32;
		sum = 0;
		SUM8(sum, -=, w + 32, p);
		*samples = round_sample(sum);

		offset = (offset - 32) & 511;
		s1->synth_buf_offset[ch] = offset;
}

// layer 1 unscaling
// n = number of bits of the mantissa minus 1
int CMpeg12Decoder::l1_unscale(int n, int mant, int scale_factor){

		int shift, mod;
		__int64 val;

		shift = scale_factor_modshift[scale_factor];
		mod = shift & 3;
		shift >>= 2;
		val = MUL64(mant + (-1 << n) + 1, scale_factor_mult[n - 1][mod]);
		shift += n;
		// NOTE: at this point, 1 <= shift >= 21 + 15
		return (int)((val + (1L << (shift - 1))) >> shift);
}

int CMpeg12Decoder::l2_unscale_group(int steps, int mant, int scale_factor){

		int shift, mod, val;

		shift = scale_factor_modshift[scale_factor];
		mod = shift & 3;
		shift >>= 2;

		val = (mant - (steps >> 1)) * scale_factor_mult2[steps >> 2][mod];
		// NOTE: at this point, 0 <= shift <= 21
		if(shift > 0)
				val = (val + (1 << (shift - 1))) >> shift;
		return val;
}

// DCT32 without 1/sqrt(2) coef zero scaling.
void CMpeg12Decoder::dct32(int *out, int *tab)
{
    int tmp0, tmp1;

    // pass 1
    BF(0, 31, COS0_0);
    BF(1, 30, COS0_1);
    BF(2, 29, COS0_2);
    BF(3, 28, COS0_3);
    BF(4, 27, COS0_4);
    BF(5, 26, COS0_5);
    BF(6, 25, COS0_6);
    BF(7, 24, COS0_7);
    BF(8, 23, COS0_8);
    BF(9, 22, COS0_9);
    BF(10, 21, COS0_10);
    BF(11, 20, COS0_11);
    BF(12, 19, COS0_12);
    BF(13, 18, COS0_13);
    BF(14, 17, COS0_14);
    BF(15, 16, COS0_15);

    // pass 2
    BF(0, 15, COS1_0);
    BF(1, 14, COS1_1);
    BF(2, 13, COS1_2);
    BF(3, 12, COS1_3);
    BF(4, 11, COS1_4);
    BF(5, 10, COS1_5);
    BF(6,  9, COS1_6);
    BF(7,  8, COS1_7);
    
    BF(16, 31, -COS1_0);
    BF(17, 30, -COS1_1);
    BF(18, 29, -COS1_2);
    BF(19, 28, -COS1_3);
    BF(20, 27, -COS1_4);
    BF(21, 26, -COS1_5);
    BF(22, 25, -COS1_6);
    BF(23, 24, -COS1_7);
    
    // pass 3
    BF(0, 7, COS2_0);
    BF(1, 6, COS2_1);
    BF(2, 5, COS2_2);
    BF(3, 4, COS2_3);
    
    BF(8, 15, -COS2_0);
    BF(9, 14, -COS2_1);
    BF(10, 13, -COS2_2);
    BF(11, 12, -COS2_3);
    
    BF(16, 23, COS2_0);
    BF(17, 22, COS2_1);
    BF(18, 21, COS2_2);
    BF(19, 20, COS2_3);
    
    BF(24, 31, -COS2_0);
    BF(25, 30, -COS2_1);
    BF(26, 29, -COS2_2);
    BF(27, 28, -COS2_3);

    // pass 4
    BF(0, 3, COS3_0);
    BF(1, 2, COS3_1);
    
    BF(4, 7, -COS3_0);
    BF(5, 6, -COS3_1);
    
    BF(8, 11, COS3_0);
    BF(9, 10, COS3_1);
    
    BF(12, 15, -COS3_0);
    BF(13, 14, -COS3_1);
    
    BF(16, 19, COS3_0);
    BF(17, 18, COS3_1);
    
    BF(20, 23, -COS3_0);
    BF(21, 22, -COS3_1);
    
    BF(24, 27, COS3_0);
    BF(25, 26, COS3_1);
    
    BF(28, 31, -COS3_0);
    BF(29, 30, -COS3_1);
    
    // pass 5
    BF1(0, 1, 2, 3);
    BF2(4, 5, 6, 7);
    BF1(8, 9, 10, 11);
    BF2(12, 13, 14, 15);
    BF1(16, 17, 18, 19);
    BF2(20, 21, 22, 23);
    BF1(24, 25, 26, 27);
    BF2(28, 29, 30, 31);
    
    // pass 6
    
    ADD( 8, 12);
    ADD(12, 10);
    ADD(10, 14);
    ADD(14,  9);
    ADD( 9, 13);
    ADD(13, 11);
    ADD(11, 15);

    out[ 0] = tab[0];
    out[16] = tab[1];
    out[ 8] = tab[2];
    out[24] = tab[3];
    out[ 4] = tab[4];
    out[20] = tab[5];
    out[12] = tab[6];
    out[28] = tab[7];
    out[ 2] = tab[8];
    out[18] = tab[9];
    out[10] = tab[10];
    out[26] = tab[11];
    out[ 6] = tab[12];
    out[22] = tab[13];
    out[14] = tab[14];
    out[30] = tab[15];
    
    ADD(24, 28);
    ADD(28, 26);
    ADD(26, 30);
    ADD(30, 25);
    ADD(25, 29);
    ADD(29, 27);
    ADD(27, 31);

    out[ 1] = tab[16] + tab[24];
    out[17] = tab[17] + tab[25];
    out[ 9] = tab[18] + tab[26];
    out[25] = tab[19] + tab[27];
    out[ 5] = tab[20] + tab[28];
    out[21] = tab[21] + tab[29];
    out[13] = tab[22] + tab[30];
    out[29] = tab[23] + tab[31];
    out[ 3] = tab[24] + tab[20];
    out[19] = tab[25] + tab[21];
    out[11] = tab[26] + tab[22];
    out[27] = tab[27] + tab[23];
    out[ 7] = tab[28] + tab[18];
    out[23] = tab[29] + tab[19];
    out[15] = tab[30] + tab[17];
    out[31] = tab[31];
}
