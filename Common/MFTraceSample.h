//----------------------------------------------------------------------------------------------
// MFTraceSample.h
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
#ifndef MFTRACESAMPLE_H
#define MFTRACESAMPLE_H

#if (_DEBUG && MF_USE_LOGGING)

void MFTraceSample(IMFSample* pSample){

	TRACE((L"MFTraceSample"));

	if(pSample == NULL){
		TRACE((L"Sample NULL"));
		return;
	}

	HRESULT hr;
	LONGLONG llValue;
	UINT32 ui32Value;
	UINT64 ui64Value;
	BOOL bValue = FALSE;
	IUnknown* pToken = NULL;

	hr = pSample->GetSampleDuration(&llValue);

	if(hr == S_OK){

		TRACE_NO_END_LINE((L"Sample duration : "));
		MFTimeString(llValue);
	}
	else if(hr == MF_E_NO_SAMPLE_DURATION){
		TRACE((L"Sample no specific duration"));
	}

	hr = pSample->GetSampleTime(&llValue);

	if(hr == S_OK){

		TRACE_NO_END_LINE((L"Sample time : "));
		MFTimeString(llValue);
	}
	else if(hr == MF_E_NO_SAMPLE_TIMESTAMP){
		TRACE((L"Sample no specific presentation time"));
	}

	//  Windows 8 only
	//MFSampleExtension_3DVideo                          : UINT32 (TRUE/FALSE)
	//MFSampleExtension_3DVideo_SampleFormat             : UINT32
	//MFSampleExtension_CaptureMetadata                  : IUnknown
	//MFSampleExtension_Content_KeyID                    : Guid
	//MFSampleExtension_Encryption_SampleID              : ???
	//MFSampleExtension_Encryption_SubSampleMappingSplit : Blob
	//MFSampleExtension_FrameCorruption                  : UINT32 (0/1)
	//MFSampleExtension_LongTermReferenceFrameInfo       : UINT32
	//MFSampleExtension_MeanAbsoluteDifference           : UINT32
	//MFSampleExtension_PhotoThumbnail                   : IUnknown
	//MFSampleExtension_PhotoThumbnailMediaType          : IUnknown
	//MFSampleExtension_ROIRectangle                     : Blob (ROI_AREA)
	//MFSampleExtension_VideoEncodePictureType           : UINT32 (eAVEncH264PictureType_B)
	//MFSampleExtension_VideoEncodeQP                    : UINT64

	//  PMP
	//MFSampleExtension_PacketCrossOffsets : Byte Array

	hr = pSample->GetUINT32(MFSampleExtension_Discontinuity, &ui32Value);

	if(hr == S_OK){

		if(ui32Value == 0)
			TRACE((L"Sample Discontinuity False"));
		else
			TRACE((L"Sample Discontinuity True"));
	}

	hr = pSample->GetUINT32(MFSampleExtension_CleanPoint, &ui32Value);

	if(hr == S_OK){

		if(ui32Value == 0)
			TRACE((L"Sample CleanPoint False"));
		else
			TRACE((L"Sample CleanPoint True"));
	}

	hr = pSample->GetUINT32(MFSampleExtension_Interlaced, &ui32Value);

	if(hr == S_OK){

		bValue = (ui32Value == 0 ? FALSE : TRUE);

		if(bValue)
			TRACE((L"Sample interlaced frame"));
		else
			TRACE((L"Sample progressive frame"));
	}

	hr = pSample->GetUINT32(MFSampleExtension_BottomFieldFirst, &ui32Value);

	if(hr == S_OK){

		DWORD dwCount;
		hr = pSample->GetBufferCount(&dwCount);

		if(hr == S_OK){

			if(dwCount == 2 && bValue){

				if(ui32Value == 0)
					TRACE((L"Sample interlaced top frame first"));
				else
					TRACE((L"Sample interlaced bottom frame first"));
			}
			else if(dwCount == 1 && bValue){

				if(ui32Value == 0)
					TRACE((L"Sample interlaced top frame"));
				else
					TRACE((L"Sample interlaced bottom frame"));
			}
			else if(bValue == FALSE){

				if(ui32Value == 0)
					TRACE((L"Sample progrssive frame should ouptup top first"));
				else
					TRACE((L"Sample progrssive frame should ouptup bottom first"));
			}
		}
	}

	hr = pSample->GetUINT32(MFSampleExtension_DerivedFromTopField, &ui32Value);

	if(hr == S_OK){

		if(ui32Value == 0)
			TRACE((L"Sample upper field interpolated from lower field"));
		else
			TRACE((L"Sample lower field interpolated from upper field"));
	}

	hr = pSample->GetUINT32(MFSampleExtension_RepeatFirstField, &ui32Value);

	if(hr == S_OK){

		if(ui32Value == 0)
			TRACE((L"Sample First Field is not repeated"));
		else
			TRACE((L"Sample First Field is repeated"));
	}

	hr = pSample->GetUINT32(MFSampleExtension_SingleField, &ui32Value);

	if(hr == S_OK){

		if(ui32Value == 0)
			TRACE((L"Sample contains complete frame"));
		else
			TRACE((L"Sample contains one field"));
	}

	hr = pSample->GetUnknown(MFSampleExtension_Token, IID_IUnknown, reinterpret_cast<void**>(&pToken));

	if(hr == S_OK && pToken){
		TRACE((L"Sample contains Token"));
	}

	SAFE_RELEASE(pToken);

	hr = pSample->GetUINT64(MFSampleExtension_DeviceTimestamp, &ui64Value);

	if(hr == S_OK){

		TRACE_NO_END_LINE((L"Sample device timestamp : "));
		MFTimeString(ui64Value);
	}
}

#else
#define MFTraceSample(x)
#endif

#endif