//----------------------------------------------------------------------------------------------
// MFNodePlayer_Capture.cpp
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
#include "Stdafx.h"

HRESULT CMFNodePlayer::CreateCaptureSource(IMFMediaSource** ppSource){

		TRACE_PLAYER((L"Player::CreateCaptureSource"));

		HRESULT hr = S_OK;

		IMFCollection* pCollection = NULL;
  IMFMediaSource* pAggregateSource = NULL;
		IMFMediaSource* pVideoSource = NULL;
  IMFMediaSource* pAudioSource = NULL;

		wstring wszVideoName = VIDEO_CAPTURE_NAME;
		wstring wszAudioName = AUDIO_CAPTURE_NAME;

		try{

				// Here we assume that audio and video capture exist
				// Change values (VIDEO_CAPTURE_... and AUDIO_CAPTURE_...) in StdAfx.h according to your audio/video capture
				IF_FAILED_THROW(hr = CreateAVCaptureSource(&pVideoSource, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID, wszVideoName, VIDEO_CAPTURE_INDEX));
				IF_FAILED_THROW(hr = CreateAVCaptureSource(&pAudioSource, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID, wszAudioName, AUDIO_CAPTURE_INDEX));

				IF_FAILED_THROW(hr = MFCreateCollection(&pCollection));

				IF_FAILED_THROW(hr = pCollection->AddElement(pVideoSource));
				IF_FAILED_THROW(hr = pCollection->AddElement(pAudioSource));

				IF_FAILED_THROW(hr = MFCreateAggregateSource(pCollection, &pAggregateSource));

				*ppSource = pAggregateSource;
				(*ppSource)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pVideoSource);
		SAFE_RELEASE(pAudioSource);
		SAFE_RELEASE(pAggregateSource);
		SAFE_RELEASE(pCollection);

		return hr;
}

HRESULT CMFNodePlayer::CreateAVCaptureSource(IMFMediaSource** ppSource, const GUID& guid, const wstring& wszCaptureName, const UINT32 uiCaptureIndex){

		TRACE_PLAYER((L"Player::CreateAVCaptureSource"));

		HRESULT hr = S_OK;

  IMFAttributes* pAttributes = NULL;
		IMFMediaSource* pSource = NULL;

		UINT32 uiDevices = 0;
  IMFActivate** ppDevices = NULL;

  WCHAR* szFriendlyName = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateAttributes(&pAttributes, 1));

				IF_FAILED_THROW(hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, guid));

				IF_FAILED_THROW(hr = MFEnumDeviceSources(pAttributes, &ppDevices, &uiDevices));

				// Change iCaptureIndex according to your audio/video capture, see CreateCaptureSource above
				IF_FAILED_THROW(hr = ((uiDevices == 0 || uiDevices < uiCaptureIndex) ? E_FAIL : S_OK));

				IF_FAILED_THROW(hr = ppDevices[uiCaptureIndex]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &szFriendlyName, NULL));

				// Change wszCaptureName according to your audio/video capture, see CreateCaptureSource above
				IF_FAILED_THROW(hr = (wszCaptureName != szFriendlyName ? E_FAIL : S_OK));

				// There is a problem with ActivateObject and Activation Object : check MSDN forum, seems to be a bug.
				IF_FAILED_THROW(hr = ppDevices[uiCaptureIndex]->ActivateObject(__uuidof(IMFMediaSource), reinterpret_cast<void**>(&pSource)));

				IF_FAILED_THROW(hr = SelectMediaType(pSource, (guid == MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID ? TRUE : FALSE)));

				*ppSource = pSource;
				(*ppSource)->AddRef();
		}
		catch(HRESULT){}

		CoTaskMemFree(szFriendlyName);

		for(UINT32 i = 0; i < uiDevices; i++)
    SAFE_RELEASE(ppDevices[i]);

		CoTaskMemFree(ppDevices);

		SAFE_RELEASE(pAttributes);
		SAFE_RELEASE(pSource);

		return hr;
}

HRESULT CMFNodePlayer::SelectMediaType(IMFMediaSource* pSource, const BOOL bVideo){

		TRACE_PLAYER((L"Player::SelectMediaType"));

		HRESULT hr = S_OK;

		IMFPresentationDescriptor* pPresentationDescriptor = NULL;
		IMFStreamDescriptor* pSourceSD = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFMediaType* pType = NULL;

		DWORD dwStreamCount;
		DWORD dwTypeCount;
		BOOL bSelected = FALSE;
		GUID guidMajorType = GUID_NULL;

		try{

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pPresentationDescriptor));

				IF_FAILED_THROW(hr = pPresentationDescriptor->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD dwStream = 0; dwStream < dwStreamCount; dwStream++){

						IF_FAILED_THROW(hr = pPresentationDescriptor->GetStreamDescriptorByIndex(dwStream, &bSelected, &pSourceSD));

						if(bSelected){

								IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
								IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

								if(bVideo && guidMajorType == MFMediaType_Video){

										IF_FAILED_THROW(hr = pHandler->GetMediaTypeCount(&dwTypeCount));

										// Change VIDEO_CAPTURE_TYPE_INDEX according to your video capture, see CreateCaptureSource above
				      IF_FAILED_THROW(hr = ((dwTypeCount == 0 || dwTypeCount < VIDEO_CAPTURE_TYPE_INDEX) ? E_FAIL : S_OK));

										IF_FAILED_THROW(hr = pHandler->GetMediaTypeByIndex(VIDEO_CAPTURE_TYPE_INDEX, &pType));
										//LogMediaType(pType);
										IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));
										break;
								}
								else if(guidMajorType == MFMediaType_Audio){

										IF_FAILED_THROW(hr = pHandler->GetMediaTypeCount(&dwTypeCount));

										// Change AUDIO_CAPTURE_TYPE_INDEX according to your audio capture, see CreateCaptureSource above
				      IF_FAILED_THROW(hr = ((dwTypeCount == 0 || dwTypeCount < AUDIO_CAPTURE_TYPE_INDEX) ? E_FAIL : S_OK));

										IF_FAILED_THROW(hr = pHandler->GetMediaTypeByIndex(AUDIO_CAPTURE_TYPE_INDEX, &pType));
										//LogMediaType(pType);
										IF_FAILED_THROW(hr = pHandler->SetCurrentMediaType(pType));
										break;
								}
								else{

										IF_FAILED_THROW(hr = E_FAIL);
								}
						}

						SAFE_RELEASE(pType);
						SAFE_RELEASE(pHandler);
						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pType);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pSourceSD);
		SAFE_RELEASE(pPresentationDescriptor);

		return hr;
}