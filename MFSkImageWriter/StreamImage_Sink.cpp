//----------------------------------------------------------------------------------------------
// StreamImage_Sink.cpp
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

HRESULT CStreamImage::GetMediaSink(IMFMediaSink** ppMediaSink){

		TRACE_STREAM((L"ImageStream::GetMediaSink"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppMediaSink == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*ppMediaSink = m_pSink;
		(*ppMediaSink)->AddRef();

		return hr;
}

HRESULT CStreamImage::GetIdentifier(DWORD* pdwIdentifier){

		TRACE_STREAM((L"ImageStream::GetIdentifier"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pdwIdentifier == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		*pdwIdentifier = 0;
		
		return hr;
}

HRESULT CStreamImage::GetMediaTypeHandler(IMFMediaTypeHandler** ppHandler){

		TRACE_STREAM((L"ImageStream::GetMediaTypeHandler"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (ppHandler == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		IF_FAILED_RETURN(hr = this->QueryInterface(IID_IMFMediaTypeHandler, reinterpret_cast<void**>(ppHandler)));
		
		return hr;
}

HRESULT CStreamImage::ProcessSample(IMFSample* pSample){

		TRACE_STREAM((L"ImageStream::ProcessSample"));

		HRESULT hr;
		IF_FAILED_RETURN(hr = (pSample == NULL ? E_INVALIDARG : S_OK));

		AutoLock lock(m_CriticSection);

		IF_FAILED_RETURN(hr = CheckShutdown());

		if(m_State != StreamStarted)
				return hr;

		// We do not handle queue and marker, just send if stream is started
		IMFMediaBuffer* pBuffer = NULL;
		BYTE* pData = NULL;
		DWORD dwLenght = 0;

		try{

				IF_FAILED_THROW(hr = pSample->GetBufferByIndex(0, &pBuffer));
				IF_FAILED_THROW(hr = pBuffer->Lock(&pData, NULL, &dwLenght));

				IF_FAILED_THROW(hr = CreateBmpFile(pData, m_uiWidth, m_uiHeight, dwLenght));
				
				LOG_HRESULT(hr = pBuffer->Unlock());
				pData = NULL;

				IF_FAILED_THROW(hr = QueueEvent(MEStreamSinkRequestSample, GUID_NULL, hr, NULL));
		}
		catch(HRESULT){}

		if(pBuffer && pData){
				LOG_HRESULT(pBuffer->Unlock());
		}

		SAFE_RELEASE(pBuffer);

		return hr;
}

HRESULT CStreamImage::PlaceMarker(MFSTREAMSINK_MARKER_TYPE /*eMarkerType*/, const PROPVARIANT* /*pvarMarkerValue*/, const PROPVARIANT* /*pvarContextValue*/){

		TRACE_STREAM((L"ImageStream::PlaceMarker"));

		// This Sink will be use by MFSrScreenCapture that normally never send marker.
		return E_NOTIMPL;
}

HRESULT CStreamImage::Flush(){

		TRACE_STREAM((L"ImageStream::Flush"));

		// if CStreamImage::PlaceMarker is implemented, see :
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms701626(v=vs.85).aspx
		return S_OK;
}

HRESULT CStreamImage::CreateBmpFile(const BYTE* pData, const UINT32 uiWidth, const UINT32 uiHeight, const DWORD dwFrameSize){

		HRESULT hr = S_OK;

		HANDLE hFile = INVALID_HANDLE_VALUE;
		DWORD dwWritten;
		UINT uiStride;

		// RGB24
		const int iRgbSize = 3;
		const BYTE bSize = 0x18;
		// RGB32
		//const int iRgbSize = 4;
		//const BYTE bSize = 0x20;
		
		BYTE header24[54] = {0x42, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00,
		                     0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																	  				0x01, 0x00, bSize, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
																			  		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

		DWORD dwSizeFile = uiWidth * uiHeight * iRgbSize;

		dwSizeFile += 54;
		header24[2] = dwSizeFile & 0x000000ff;
		header24[3] = static_cast<BYTE>((dwSizeFile & 0x0000ff00) >> 8);
		header24[4] = static_cast<BYTE>((dwSizeFile & 0x00ff0000) >> 16);
		header24[5] = (dwSizeFile & 0xff000000) >> 24;
		dwSizeFile -= 54;
		header24[18] = uiWidth & 0x000000ff;
		header24[19] = (uiWidth & 0x0000ff00) >> 8;
		header24[20] = static_cast<BYTE>((uiWidth & 0x00ff0000) >> 16);
		header24[21] = (uiWidth & 0xff000000) >> 24;

		header24[22] = uiHeight & 0x000000ff;
		header24[23] = (uiHeight & 0x0000ff00) >> 8;
		header24[24] = static_cast<BYTE>((uiHeight & 0x00ff0000) >> 16);
		header24[25] = (uiHeight & 0xff000000) >> 24;
		
		header24[34] = dwSizeFile & 0x000000ff;
		header24[35] = (dwSizeFile & 0x0000ff00) >> 8;
		header24[36] = static_cast<BYTE>((dwSizeFile & 0x00ff0000) >> 16);
		header24[37] = static_cast<BYTE>((dwSizeFile & 0xff000000) >> 24);

		static DWORD dwIndex = 0;
		dwIndex++;

		wostringstream woss;
		woss << dwIndex;

		// Change this !
		wstring wszFile = L"C:\\Media\\Image";
		wszFile += woss.str();
		wszFile += L".bmp";

		try{

				hFile = CreateFile(wszFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

				IF_FAILED_THROW(hr = (hFile == INVALID_HANDLE_VALUE ? E_FAIL: S_OK));

				IF_FAILED_THROW(hr = (WriteFile(hFile, (LPCVOID)header24, 54, &dwWritten, 0) == FALSE));
				IF_FAILED_THROW(hr = (dwWritten == 0 ? E_FAIL: S_OK));

				uiStride = uiWidth * iRgbSize;

				const BYTE* bufsrc = pData + (dwFrameSize - uiStride);

				for(UINT32 ui = 0; ui < uiHeight; ui++){

						IF_FAILED_THROW(hr = (WriteFile(hFile, (LPCVOID)bufsrc, uiStride, &dwWritten, 0) == FALSE));
						IF_FAILED_THROW(hr = (dwWritten == 0 ? E_FAIL: S_OK));

						bufsrc -= uiStride;
				}
		}
		catch(HRESULT){}

		CLOSE_HANDLE_IF(hFile);

		return hr;
}