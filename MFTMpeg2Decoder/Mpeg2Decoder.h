//----------------------------------------------------------------------------------------------
// Mpeg2Decoder.h
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
#ifndef MP2DECODER_H
#define MP2DECODER_H

class CMpeg2Decoder : public IUnknown, RefCountedObject{

		public:
				
				static HRESULT CreateInstance(CMpeg2Decoder**);

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
    STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

				// Inline
				HRESULT Mpeg2DecodeInit();
				HRESULT Mpeg2SetInputBuffer(BYTE*, DWORD);
				int Mpeg2DecodeFrame(){ return mpeg2_parse(m_mpeg2ds); }
				void Mpeg2DecodeClose(){ if(m_mpeg2ds){ mpeg2_close(m_mpeg2ds); m_mpeg2ds = NULL; } }
				BOOL Mpeg2HaveImage();
				const unsigned char* Mpeg2GetDisplayBufferY();
				const unsigned char* Mpeg2GetDisplayBufferU();
				const unsigned char* Mpeg2GetDisplayBufferV();
				unsigned int GetPeriod();
				unsigned int GetWidth(){ const mpeg2_info_t* info = mpeg2_info(m_mpeg2ds); return info->sequence->width; }

		private:

				CMpeg2Decoder() : m_mpeg2ds(NULL){}
				virtual ~CMpeg2Decoder(){ Mpeg2DecodeClose(); }

				CriticSection m_CriticSection;

				mpeg2dec_t* m_mpeg2ds;
};

inline HRESULT CMpeg2Decoder::Mpeg2DecodeInit(){

		if(m_mpeg2ds){
				mpeg2_close(m_mpeg2ds);
		}

		mpeg2_accel(MPEG2_ACCEL_X86_MMXEXT);

		m_mpeg2ds = mpeg2_init();

		return m_mpeg2ds ? S_OK : E_FAIL;
}

inline HRESULT CMpeg2Decoder::Mpeg2SetInputBuffer(BYTE* pBuf, DWORD dwLenghtBuf){

		HRESULT hr = S_OK;

		if(m_mpeg2ds == NULL){
				hr = E_FAIL;
		}
		else{
				mpeg2_buffer(m_mpeg2ds, pBuf, pBuf + dwLenghtBuf);
		}
		return hr;
}

inline BOOL CMpeg2Decoder::Mpeg2HaveImage(){

		const mpeg2_info_t* info = NULL;
		
		info = mpeg2_info(m_mpeg2ds);

		if(info){
				return info->display_fbuf ? TRUE : FALSE;
		}
		else{
				return FALSE;
		}
}

inline const unsigned char* CMpeg2Decoder::Mpeg2GetDisplayBufferY(){

		const mpeg2_info_t* info = NULL;
		
		info = mpeg2_info(m_mpeg2ds);

		if(info && info->display_fbuf){
				return info->display_fbuf->buf[0];
		}
		else{
				return NULL;
		}
}

inline const unsigned char* CMpeg2Decoder::Mpeg2GetDisplayBufferU(){

		const mpeg2_info_t* info = NULL;
		
		info = mpeg2_info(m_mpeg2ds);

		if(info && info->display_fbuf){
				return info->display_fbuf->buf[1];
		}
		else{
				return NULL;
		}
}

inline const unsigned char* CMpeg2Decoder::Mpeg2GetDisplayBufferV(){

		const mpeg2_info_t* info = NULL;
		
		info = mpeg2_info(m_mpeg2ds);

		if(info && info->display_fbuf){
				return info->display_fbuf->buf[2];
		}
		else{
				return NULL;
		}
}

inline unsigned int CMpeg2Decoder::GetPeriod(){

		unsigned int uiRes = 0;
		const mpeg2_info_t* info = NULL;

		info = mpeg2_info(m_mpeg2ds);

		if(info && info->sequence)
				uiRes = info->sequence->frame_period;

		return uiRes;
}

#endif