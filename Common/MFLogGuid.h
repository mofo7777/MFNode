//----------------------------------------------------------------------------------------------
// MFLogGuid.h
//----------------------------------------------------------------------------------------------
#ifndef MFLOGGUID_H
#define MFLOGGUID_H

#if (_DEBUG && MF_USE_LOGGING)

inline LPCWSTR GetGUIDStringConst(const GUID& guid){

	// here you can add your own CLSID
	IF_EQUAL_RETURN(guid, GUID_NULL);
	// MFNode CLSID
	IF_EQUAL_RETURN(guid, CLSID_MFTMpeg2Decoder);
	IF_EQUAL_RETURN(guid, CLSID_MFSrKinectCapture);
	IF_EQUAL_RETURN(guid, CLSID_MFSrScreenCapture);
	IF_EQUAL_RETURN(guid, CLSID_MFTWaveMixer);
	IF_EQUAL_RETURN(guid, CLSID_MFSrMpeg12Decoder);
	IF_EQUAL_RETURN(guid, CLSID_MFTMpeg12Decoder);
	IF_EQUAL_RETURN(guid, CLSID_MFTCudaDecoder);
	IF_EQUAL_RETURN(guid, CLSID_MFSrMpeg2Splitter);
	IF_EQUAL_RETURN(guid, CLSID_MFTJpegEncoder);
	IF_EQUAL_RETURN(guid, CLSID_MFSkJpegHttpStreamer);
	IF_EQUAL_RETURN(guid, CLSID_WAVEByteStreamPlugin);
	// Other
	IF_EQUAL_RETURN(guid, CLSID_CMpeg4DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg43DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg4sDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg4sDecMFT);
	IF_EQUAL_RETURN(guid, CLSID_CZuneM4S2DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg4EncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg4sEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMSSCDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMSSCEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMSSCEncMediaObject2);
	IF_EQUAL_RETURN(guid, CLSID_CWMADecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMAEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMATransMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMSPDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMSPEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMSPEncMediaObject2);
	IF_EQUAL_RETURN(guid, CLSID_CWMTDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMTEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMVDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMVEncMediaObject2);
	IF_EQUAL_RETURN(guid, CLSID_CWMVXEncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMV9EncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWVC1DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWVC1EncMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CDeColorConvMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CDVDecoderMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CDVEncoderMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMpeg2DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CPK_DS_MPEG2Decoder);
	IF_EQUAL_RETURN(guid, CLSID_CAC3DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CPK_DS_AC3Decoder);
	IF_EQUAL_RETURN(guid, CLSID_CMP3DecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CResamplerMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CResizerMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CInterlaceMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CWMAudioLFXAPO);
	IF_EQUAL_RETURN(guid, CLSID_CWMAudioGFXAPO);
	IF_EQUAL_RETURN(guid, CLSID_CWMAudioSpdTxDMO);
	IF_EQUAL_RETURN(guid, CLSID_CWMAudioAEC);
	IF_EQUAL_RETURN(guid, CLSID_CClusterDetectorDmo);
	IF_EQUAL_RETURN(guid, CLSID_CColorControlDmo);
	IF_EQUAL_RETURN(guid, CLSID_CColorConvertDMO);
	IF_EQUAL_RETURN(guid, CLSID_CColorLegalizerDmo);
	IF_EQUAL_RETURN(guid, CLSID_CFrameInterpDMO);
	IF_EQUAL_RETURN(guid, CLSID_CFrameRateConvertDmo);
	IF_EQUAL_RETURN(guid, CLSID_CResizerDMO);
	IF_EQUAL_RETURN(guid, CLSID_CShotDetectorDmo);
	IF_EQUAL_RETURN(guid, CLSID_CSmpteTransformsDmo);
	IF_EQUAL_RETURN(guid, CLSID_CThumbnailGeneratorDmo);
	IF_EQUAL_RETURN(guid, CLSID_CTocGeneratorDmo);
	IF_EQUAL_RETURN(guid, CLSID_CMPEGAACDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CNokiaAACDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CVodafoneAACDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CZuneAACCCDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CNokiaAACCCDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CVodafoneAACCCDecMediaObject);
	IF_EQUAL_RETURN(guid, CLSID_CMPEG2EncoderDS);
	IF_EQUAL_RETURN(guid, CLSID_CMPEG2EncoderVideoDS);
	IF_EQUAL_RETURN(guid, CLSID_CMPEG2EncoderAudioDS);
	IF_EQUAL_RETURN(guid, CLSID_CMPEG2AudDecoderDS);
	IF_EQUAL_RETURN(guid, CLSID_CMPEG2VidDecoderDS);
	IF_EQUAL_RETURN(guid, CLSID_CDTVAudDecoderDS);
	IF_EQUAL_RETURN(guid, CLSID_CDTVVidDecoderDS);
	IF_EQUAL_RETURN(guid, CLSID_CMSAC3Enc);
	IF_EQUAL_RETURN(guid, CLSID_CMSH264DecoderMFT);

	return NULL;
}

inline HRESULT GetGUIDString(const GUID& guid, WCHAR** ppwsz){

	HRESULT hr = S_OK;
	size_t cchLength = 0;

	WCHAR* pName = NULL;

	LPCWSTR pcwsz = GetGUIDStringConst(guid);

	try{

		if(pcwsz){

			IF_FAILED_THROW(hr = StringCchLength(pcwsz, STRSAFE_MAX_CCH, &cchLength));

			pName = (WCHAR*)CoTaskMemAlloc((cchLength + 1) * sizeof(WCHAR));

			IF_FAILED_THROW(hr = StringCchCopy(pName, cchLength + 1, pcwsz));
		}
		else{
			IF_FAILED_THROW(hr = StringFromCLSID(guid, &pName));
		}

		*ppwsz = pName;
	}
	catch(HRESULT){ LOG_HRESULT(hr); }

	if(FAILED(hr)){

		*ppwsz = NULL;
		CoTaskMemFree(pName);
	}

	return hr;
}

inline HRESULT GetGuidFilename(const GUID& guid, WCHAR* szFilename, size_t len){

	HRESULT hr;
	LPOLESTR strCLSID = NULL;

	hr = StringFromCLSID(guid, &strCLSID);

	if(SUCCEEDED(hr)){

		WCHAR szKey[512];

		hr = StringCchPrintf(szKey, 512, TEXT("Software\\Classes\\CLSID\\%s\\InprocServer32\0"), (LPCTSTR)strCLSID);

		HKEY hkeyFilter = 0;
		DWORD dwSize = MAX_PATH;
		BYTE szFile[MAX_PATH];
		int rc = 0;

		// Open the CLSID key that contains information about the filter
		rc = RegOpenKey(HKEY_LOCAL_MACHINE, szKey, &hkeyFilter);

		if(rc == ERROR_SUCCESS){

			rc = RegQueryValueEx(hkeyFilter, NULL, NULL, NULL, szFile, &dwSize);

			if(rc == ERROR_SUCCESS)
				hr = StringCchPrintf(szFilename, len, TEXT("%s\0"), szFile);
			else
				hr = StringCchCopy(szFilename, len, TEXT("<Unknown>\0"));

			RegCloseKey(hkeyFilter);
		}
		else{
			hr = StringCchCopy(szFilename, len, TEXT("<No registry entry>\0"));
		}
	}

	CoTaskMemFree(strCLSID);

	return hr;
}

inline void LogGuid(const GUID& guid){

	WCHAR* pGuidName = NULL;
	HRESULT hr = GetGUIDString(guid, &pGuidName);

	if(SUCCEEDED(hr)){
		TRACE((L"Log Guid : %s\n", pGuidName));
	}

	if(FAILED(hr) || pGuidName == NULL){

		LPOLESTR strCLSID = NULL;
		hr = StringFromCLSID(guid, &strCLSID);

		if(SUCCEEDED(hr)){
			TRACE((L"Log Guid : %s\n", pGuidName));
		}
		else{

			TRACE((L"Log Guid : Error\n"));
		}

		CoTaskMemFree(strCLSID);
	}

	CoTaskMemFree(pGuidName);
}

inline void LogGuidAndFilename(const GUID& guid){

	WCHAR szFile[512] = {0};
	WCHAR* pGuidName = NULL;
	HRESULT hr = GetGUIDString(guid, &pGuidName);

	if(FAILED(hr) || pGuidName == NULL){

		LPOLESTR strCLSID = NULL;
		StringFromCLSID(guid, &strCLSID);
		CoTaskMemFree(strCLSID);
	}

	if(SUCCEEDED(GetGuidFilename(guid, szFile, 512))){
		TRACE((L"%s File : %s\n", pGuidName, szFile));
	}
	else{
		TRACE((L"%s Guid File : Error\n", pGuidName));
	}

	CoTaskMemFree(pGuidName);
}

#else
#define LogGuid(x)
#define LogGuidAndFilename(x)
#endif

#endif