//----------------------------------------------------------------------------------------------
// MFTJpegEncoder.cpp
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
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

CMFTJpegEncoder::CMFTJpegEncoder()
		: m_nRefCount(1),
		  m_pInputType(NULL),
				m_pOutputType(NULL),
				m_pSampleOut(NULL),
				m_bHaveImage(FALSE),
				m_rtFrame(0),
				m_ui64AvgPerFrame(0),
				m_iJpegQuality(50),
				m_GdiToken(0),
				m_cJpegStream(NULL),
				m_iJpegStream(NULL),
				m_pBmpImageFile(NULL)
{
		TRACE_TRANSFORM((L"MFTJpeg::CTOR"));
}

CMFTJpegEncoder::~CMFTJpegEncoder(){

		TRACE_TRANSFORM((L"MFTJpeg::DTOR"));

		SAFE_DELETE(m_pBmpImageFile);
		SAFE_RELEASE(m_iJpegStream);
		SAFE_RELEASE(m_cJpegStream);

		SAFE_RELEASE(m_pInputType);
		SAFE_RELEASE(m_pOutputType);
		SAFE_RELEASE(m_pSampleOut);
}

HRESULT CMFTJpegEncoder::CreateInstance(IUnknown* pUnkOuter, REFIID iid, void** ppv){

		TRACE_TRANSFORM((L"MFTJpeg::CreateInstance"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppv == NULL ? E_POINTER : S_OK));
		IF_FAILED_RETURN(hr = (pUnkOuter != NULL ? CLASS_E_NOAGGREGATION : S_OK));

		CMFTJpegEncoder* pMFT = new (std::nothrow)CMFTJpegEncoder;

		IF_FAILED_RETURN(pMFT == NULL ? E_OUTOFMEMORY : S_OK);

		LOG_HRESULT(hr = pMFT->QueryInterface(iid, ppv));

		SAFE_RELEASE(pMFT);

		return hr;
}

ULONG CMFTJpegEncoder::AddRef(){

		LONG lRef = InterlockedIncrement(&m_nRefCount);

		TRACE_REFCOUNT((L"MFTJpeg::AddRef m_nRefCount = %d", lRef));

		return lRef;
}

ULONG CMFTJpegEncoder::Release(){
		
		ULONG uCount = InterlockedDecrement(&m_nRefCount);

		TRACE_REFCOUNT((L"MFTJpeg::Release m_nRefCount = %d", uCount));
		
		if(uCount == 0){
				delete this;
		}
		
		return uCount;
}

HRESULT CMFTJpegEncoder::QueryInterface(REFIID riid, void** ppv){

		TRACE_TRANSFORM((L"MFTJpeg::QI : riid = %s", GetIIDString(riid)));

		static const QITAB qit[] = { QITABENT(CMFTJpegEncoder, IMFTransform), {0} };

		return QISearch(this, qit, riid, ppv);
}

HRESULT CMFTJpegEncoder::BeginStreaming(){

		HRESULT hr;

		IF_FAILED_RETURN(hr = ((m_pInputType == NULL || m_pOutputType == NULL) ? MF_E_TRANSFORM_TYPE_NOT_SET : S_OK));

		// MFT_MESSAGE_NOTIFY_END_STREAMING must be called or call EndStreaming.
		IF_FAILED_RETURN(hr = (m_pSampleOut != NULL ? MF_E_INVALIDREQUEST : S_OK));

		IMFSample* pSampleOut = NULL;
		Status GdiStatus;
		UINT32 uiWidth = 0;
		UINT32 uiHeight = 0;
		GUID SubType = {0};
		int iPixelFormat;
		int iRgbByte;
		DWORD dwRatio;

		try{

				GdiplusStartupInput gdiplusStartupInput;
				GdiStatus = GdiplusStartup(&m_GdiToken, &gdiplusStartupInput, NULL);

				IF_FAILED_THROW(hr = (GdiStatus != Ok ? E_FAIL : S_OK));

				IF_FAILED_THROW(hr = CJpegStream::CreateInstance(&m_cJpegStream));
				IF_FAILED_THROW(hr = m_cJpegStream->QueryInterface(IID_IStream, reinterpret_cast<void**>(&m_iJpegStream)));

				IF_FAILED_THROW(hr = MFGetAttributeSize(m_pInputType, MF_MT_FRAME_SIZE, &uiWidth, &uiHeight));
				IF_FAILED_THROW(hr = m_pInputType->GetGUID(MF_MT_SUBTYPE, &SubType));

				if(SubType == MFVideoFormat_RGB24){
						iRgbByte = 3;
						iPixelFormat = PixelFormat24bppRGB;
				}
				else{
						iRgbByte = 4;
						iPixelFormat = PixelFormat32bppRGB;
				}

				// Calcute around m_iJpegQuality. For now fixed ratio to 5.0f (m_iJpegQuality == 50)
				dwRatio = static_cast<DWORD>((uiWidth * uiHeight * iRgbByte) / 5.0f);
				ULARGE_INTEGER ul;
				ul.LowPart = dwRatio;

				m_cJpegStream->Initialize(dwRatio);
				IF_FAILED_THROW(hr = m_cJpegStream->SetSize(ul));

				// ???
				// m_pBmpImageFile = new (std::nothrow)Bitmap(uiWidth, uiHeight, iPixelFormat);
				m_pBmpImageFile = new Bitmap(uiWidth, uiHeight, iPixelFormat);

				IF_FAILED_THROW(hr = (m_pBmpImageFile == NULL ? E_OUTOFMEMORY : S_OK));

				IF_FAILED_THROW(hr = GetEncoderClsid(L"image/jpeg", &m_JpgClsid));

				IF_FAILED_THROW(hr = MFCreateSample(&pSampleOut));

				m_pSampleOut = pSampleOut;
				m_pSampleOut->AddRef();
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pSampleOut);

		return hr;
}

void CMFTJpegEncoder::EndStreaming(){

		m_rtFrame = 0;
		m_ui64AvgPerFrame = 0;

		if(m_GdiToken){

				GdiplusShutdown(m_GdiToken);
				m_GdiToken = 0;
		}

		SAFE_DELETE(m_pBmpImageFile);
		SAFE_RELEASE(m_iJpegStream);
		SAFE_RELEASE(m_cJpegStream);
		SAFE_RELEASE(m_pSampleOut);
}

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms533843(v=vs.85).aspx
HRESULT CMFTJpegEncoder::GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
		
		HRESULT hr = E_FAIL;
		UINT iNum = 0;
		UINT iSize = 0;
		ImageCodecInfo* pImageCodecInfo = NULL;

		GetImageEncodersSize(&iNum, &iSize);
		
		if(iSize == 0)
				return hr;

		pImageCodecInfo = (ImageCodecInfo*)(malloc(iSize));
		
		if(pImageCodecInfo == NULL)
				return hr;

		GetImageEncoders(iNum, iSize, pImageCodecInfo);

		for(UINT i = 0; i < iNum; i++){
				
				if(wcscmp(pImageCodecInfo[i].MimeType, format) == 0){
						
						*pClsid = pImageCodecInfo[i].Clsid;
						hr = S_OK;
						break;
				}    
		}

		free(pImageCodecInfo);
		
	return hr;
}