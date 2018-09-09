//----------------------------------------------------------------------------------------------
// MFParser.h
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
#ifndef MFPARSER_H
#define MFPARSER_H

class CMFParser{

  public:

				CMFParser();
				~CMFParser(){}

				HRESULT ParseBytes(const BYTE*, DWORD, DWORD*);

				DWORD GetSystemHeader(MPEG2SystemHeader*);

				BOOL HasSystemHeader() const{ return m_bHasSysHeader; }
				BOOL HasPacket() const{ return m_bHasPacket; }
				BOOL IsEndOfStream() const{ return m_bEOS; }
				const BOOL IsMpeg1() const{ return m_bMpeg1; }

				DWORD PayloadSize() const{ assert(m_bHasPacket); return m_curPacketHeader.cbPayload; }
				StreamType GetPacketType() const{ assert(m_bHasPacket); return m_curPacketHeader.type; }
				const MPEG2PacketHeader& PacketHeader() { assert(m_bHasPacket); return m_curPacketHeader; }

				void ClearSysHeader(){ m_bHasSysHeader = FALSE; }
				void ClearPacket(){ m_bHasPacket = FALSE; }
				void Reset(){ m_bHasFoundSysHeader = FALSE; m_bHasSysHeader = FALSE; m_bHasPacket = FALSE; m_bEOS = FALSE; }
				void ResetEOS(){ m_bEOS = FALSE; }

				const BOOL GetNumPack() const{ return m_dwPack; }
				const DWORD GetNumCode() const{ return m_dwStartCode; }
				const DWORD GetByteAte() const{ return m_dwByteAte; }

				// MFParser_AVHeader.cpp
				HRESULT ReadVideoSequenceHeader(const BYTE*, DWORD, MPEG2VideoSeqHeader&);
				HRESULT ReadAudioFrameHeader(const BYTE*, DWORD, MPEG2AudioFrameHeader&, DWORD*);
				HRESULT ReadAC3AudioFrameHeader(const BYTE*, DWORD, WAVEFORMATEX&, DWORD*);

  private:

				MPEG2SystemHeader m_SysHeader;
				MPEG2PacketHeader m_curPacketHeader;

				BOOL m_bHasFoundSysHeader;
				BOOL m_bHasSysHeader;
				BOOL m_bHasPacket;

				BOOL m_bEOS;
				BOOL m_bMpeg1;

				DWORD m_dwPack;
				DWORD m_dwStartCode;
				DWORD m_dwByteAte;

				LONGLONG m_SCR;
				DWORD m_SCRExt;
    DWORD m_muxRate;

				HRESULT FindNextStartCode(const BYTE*, DWORD, DWORD*);
    HRESULT ParsePackHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte);
    HRESULT ParseSystemHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte);
				HRESULT ParsePacketHeaderMpeg1(const BYTE*, DWORD, DWORD*);
				HRESULT ParsePacketHeaderMpeg2(const BYTE*, DWORD, DWORD*);

				HRESULT ParseStreamData(const BYTE*, MPEG2StreamHeader&);
				HRESULT ParseStreamId(BYTE, StreamType*, BYTE*);
				HRESULT ParsePTS(const BYTE*, LONGLONG*);
				HRESULT ParseDTSPTS(const BYTE*, LONGLONG*, LONGLONG*);

				// MFParser_AVHeader.cpp
				HRESULT GetPixelAspectRatioMpeg1(BYTE, MFRatio*);
				HRESULT GetFrameRateMpeg1(BYTE, MFRatio*);
				HRESULT GetPixelAspectRatioMpeg2(BYTE, MFRatio*);
				HRESULT GetFrameRateMpeg2(BYTE, MFRatio*);
				HRESULT GetAudioBitRateMpeg1(MPEG2AudioLayer, BYTE, DWORD*);
				HRESULT GetAudioBitRateMpeg2(MPEG2AudioLayer, BYTE, DWORD*);
				HRESULT GetSamplingFrequency(const BYTE, const BYTE, DWORD*);

				// inline
				void OnEndOfStream(){ m_bEOS = TRUE; ClearPacket(); ClearSysHeader(); }
				HRESULT AdvanceBufferPointer(const BYTE*&, DWORD&, DWORD);
				HRESULT ValidateBufferSize(DWORD cbBufferSize, DWORD cbMinRequiredSize){ return (cbBufferSize >= cbMinRequiredSize? S_OK : E_FAIL); }
};

inline DWORD CMFParser::GetSystemHeader(MPEG2SystemHeader* pSysHeader){
		
		DWORD dwStreams = 0;

		if(m_SysHeader.streams[0].type != StreamType_Unknown)
				dwStreams++;

		if(m_SysHeader.streams[0].type != StreamType_Unknown)
				dwStreams++;

		if(dwStreams == 0)
				return dwStreams;

		CopyMemory(pSysHeader, &m_SysHeader, sizeof(MPEG2SystemHeader));

		return dwStreams;
}

inline HRESULT CMFParser::AdvanceBufferPointer(const BYTE*& pData, DWORD& cbBufferSize, DWORD cbAdvance){
		
		assert(cbBufferSize >= cbAdvance);
		
		if(cbBufferSize < cbAdvance){
				return E_FAIL;
		}
		
		cbBufferSize -= cbAdvance;
		pData += cbAdvance;
		
		return S_OK;
}

#endif