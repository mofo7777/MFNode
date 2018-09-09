//----------------------------------------------------------------------------------------------
// CudaDecoder.h
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
#ifndef CUDADECODER_H
#define CUDADECODER_H

class CCudaDecoder{

  public:
				
				CCudaDecoder() : m_pDecoder(NULL){}
				~CCudaDecoder(){ Release(); }

				HRESULT InitDecoder(const CUvideoctxlock, const UINT32, const UINT32, const BOOL);
				void Release(){ if(m_pDecoder){ cuvidDestroyDecoder(m_pDecoder); m_pDecoder = NULL; } }
				const BOOL IsInit() const{ return m_pDecoder != NULL; }

				HRESULT DecodePicture(CUVIDPICPARAMS*);
				HRESULT MapFrame(int, CUdeviceptr*, unsigned int*, CUVIDPROCPARAMS*);
				HRESULT UnmapFrame(CUdeviceptr);

  private:

				CUvideodecoder m_pDecoder;
};

#endif