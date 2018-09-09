//----------------------------------------------------------------------------------------------
// Vp6Decoder_Mmx.cpp
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

void CVp6Decoder::Vp6CopyMmx(void* dst, const void* src, size_t len){

	__asm{

		mov     esi, dword ptr[src]
		mov     edi, dword ptr[dst]
		mov     ecx, len
		shr     ecx, 6

		memcpy_accel_mmx_loop:

		movq    mm0, qword ptr[esi]
			movq    mm1, qword ptr[esi + 8 * 1]
			movq    mm2, qword ptr[esi + 8 * 2]
			movq    mm3, qword ptr[esi + 8 * 3]
			movq    mm4, qword ptr[esi + 8 * 4]
			movq    mm5, qword ptr[esi + 8 * 5]
			movq    mm6, qword ptr[esi + 8 * 6]
			movq    mm7, qword ptr[esi + 8 * 7]
			movq    qword ptr[edi], mm0
			movq    qword ptr[edi + 8 * 1], mm1
			movq    qword ptr[edi + 8 * 2], mm2
			movq    qword ptr[edi + 8 * 3], mm3
			movq    qword ptr[edi + 8 * 4], mm4
			movq    qword ptr[edi + 8 * 5], mm5
			movq    qword ptr[edi + 8 * 6], mm6
			movq    qword ptr[edi + 8 * 7], mm7
			add     esi, 64
			add     edi, 64
			loop	memcpy_accel_mmx_loop
			mov     ecx, len
			and     ecx, 63
			cmp     ecx, 0
			je		memcpy_accel_mmx_end

			memcpy_accel_mmx_loop2 :

		mov		dl, byte ptr[esi]
			mov		byte ptr[edi], dl
			inc		esi
			inc		edi
			dec		ecx
			jne		memcpy_accel_mmx_loop2

			memcpy_accel_mmx_end :

		emms
	}
}

void CVp6Decoder::Vp6CopyFrameYV12Stride(const unsigned int uiSize, const UINT32 uiWidth, const UINT32 uiHeight, BYTE* pOut, const BYTE* pInY, const BYTE* pInU, const BYTE* pInV){

	for(DWORD i = 0; i < uiHeight; i++, pInY += uiSize, pOut += uiWidth)
		Vp6CopyMmx(pOut, pInY, uiWidth);

	const UINT32 iInPitch = uiSize >> 1;
	const UINT32 iPitch = uiWidth >> 1;
	const UINT32 uiUVHeight = uiHeight >> 1;

	for(DWORD i = 0; i < uiUVHeight; i++, pInV += iInPitch, pOut += iPitch)
		Vp6CopyMmx(pOut, pInV, iPitch);

	for(DWORD i = 0; i < uiUVHeight; i++, pInU += iInPitch, pOut += iPitch)
		Vp6CopyMmx(pOut, pInU, iPitch);
}