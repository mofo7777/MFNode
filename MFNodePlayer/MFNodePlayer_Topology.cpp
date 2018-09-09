//----------------------------------------------------------------------------------------------
// MFNodePlayer_Topology.cpp
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

HRESULT CMFNodePlayer::CreateMediaSource(IMFMediaSource** ppSource, PCWSTR wszUrl, const DWORD dwFlags){

		TRACE_PLAYER((L"Player::CreateMediaSource"));

		HRESULT hr = S_OK;
		MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
		IMFSourceResolver* pSourceResolver = NULL;
		IUnknown* pSource = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateSourceResolver(&pSourceResolver));

				IF_FAILED_THROW(hr = pSourceResolver->CreateObjectFromURL(
						wszUrl,
						dwFlags,
						NULL,
						&objectType,
						&pSource
						));

				IF_FAILED_THROW(hr = pSource->QueryInterface(IID_IMFMediaSource, reinterpret_cast<void**>(ppSource)));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSource);
		SAFE_RELEASE(pSourceResolver);

		return hr;
}

HRESULT CMFNodePlayer::CreateScreenShotTopology(IMFTopology** ppTopology){

		TRACE_PLAYER((L"Player::CreateScreenShotTopology"));

		assert(m_pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = m_pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildScreenShotTopology(pTopology, pSourcePD, 0));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateHttpTopology(IMFTopology** ppTopology){

		TRACE_PLAYER((L"Player::CreateHttpTopology"));

		assert(m_pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = m_pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildHttpTopology(pTopology, pSourcePD, 0));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateTopology(IMFTopology** ppTopology, IMFMediaSource* pSource){

		TRACE_PLAYER((L"Player::CreateTopology"));

		assert(pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildTopology(pTopology, pSourcePD, pSource));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateRendererTopology(IMFTopology** ppTopology, IMFMediaSource* pSource, const CLSID clsidTransform, const CLSID clsidSink){

		TRACE_PLAYER((L"Player::CreateRendererTopology"));

		assert(pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildRendererTopology(pTopology, pSource, pSourcePD, clsidTransform, clsidSink));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::BuildScreenShotTopology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, DWORD iStream){
		
		TRACE_PLAYER((L"Player::BuildScreenShotTopology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		BOOL bSelected = FALSE;

		try{
				
				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(iStream, &bSelected, &pSourceSD));

				if(bSelected){
						
						IF_FAILED_THROW(hr = CreateSourceStreamNode(m_pSource, pSourcePD, pSourceSD, &pSourceNode));

						IF_FAILED_THROW(hr = CreateVideoOutputNode(pSourceSD, &pOutputNode));

						IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

						IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
				}
				else{
						IF_FAILED_THROW(hr = E_UNEXPECTED);
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pSourceSD);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pOutputNode);
		
		return hr;
}

HRESULT CMFNodePlayer::BuildHttpTopology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, DWORD iStream){
		
		TRACE_PLAYER((L"Player::BuildHttpTopology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pTransformNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		BOOL bSelected = FALSE;

		try{

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(iStream, &bSelected, &pSourceSD));

				if(bSelected){

						IF_FAILED_THROW(hr = CreateSourceStreamNode(m_pSource, pSourcePD, pSourceSD, &pSourceNode));

						IF_FAILED_THROW(hr = CreateTransformNode(CLSID_MFTJpegEncoder, &pTransformNode));

						IF_FAILED_THROW(hr = CreateClsidOutputNode(CLSID_MFSkJpegHttpStreamer, &pOutputNode));

						IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						IF_FAILED_THROW(hr = pTopology->AddNode(pTransformNode));
						IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

						IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pTransformNode, 0));
						IF_FAILED_THROW(hr = pTransformNode->ConnectOutput(0, pOutputNode, 0));
				}
				else{
						IF_FAILED_THROW(hr = E_UNEXPECTED);
				}
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pSourceSD);

		return hr;
}

HRESULT CMFNodePlayer::BuildTopology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, IMFMediaSource* pSource){
		
		TRACE_PLAYER((L"Player::BuildTopology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		BOOL bSelected = FALSE;
		DWORD dwStreamCount;

		try{

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD i = 0; i < dwStreamCount; i++){

						IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD));

						if(bSelected){

								IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));

						  IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode));

						  IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						  IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

						  IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));

								SAFE_RELEASE(pOutputNode);
								SAFE_RELEASE(pSourceNode);
						}

						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pSourceSD);

		return hr;
}

HRESULT CMFNodePlayer::BuildRendererTopology(IMFTopology* pTopology, IMFMediaSource* pSource, IMFPresentationDescriptor* pSourcePD,
		const CLSID clsidTransform, const CLSID clsidSink){
		
		TRACE_PLAYER((L"Player::BuildRendererTopology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pTransformNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;

		GUID guidMajorType = GUID_NULL;
		BOOL bSelected = FALSE;
		DWORD dwStreamCount;

		try{

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD i = 0; i < dwStreamCount; i++){

						IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD));

						if(bSelected){

								IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
								IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

								if(guidMajorType == MFMediaType_Video){

										IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));

										IF_FAILED_THROW(hr = CreateClsidOutputNode(clsidSink, &pOutputNode));

						    IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						    IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

										if(clsidTransform != CLSID_NULL){

												IF_FAILED_THROW(hr = CreateTransformNode(clsidTransform, &pTransformNode));
												IF_FAILED_THROW(hr = pTopology->AddNode(pTransformNode));

												IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pTransformNode, 0));
						      IF_FAILED_THROW(hr = pTransformNode->ConnectOutput(0, pOutputNode, 0));
										}
										else{

												IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
										}

										break;
								}
						}

						SAFE_RELEASE(pHandler);
						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pSourceSD);

		return hr;
}

HRESULT CMFNodePlayer::CreateSourceStreamNode(IMFMediaSource* pSource, IMFPresentationDescriptor* pSourcePD,
		IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode){
				
  TRACE_PLAYER((L"Player::CreateSourceStreamNode"));

		if(!pSource || !pSourcePD || !pSourceSD || !ppNode){
				return E_POINTER;
		}

		IMFTopologyNode* pNode = NULL;
		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pSourcePD));

				IF_FAILED_THROW(hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSourceSD));

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pNode);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateTransformNode(const CLSID& clsid, IMFTopologyNode** ppNode){

		TRACE_PLAYER((L"Player::CreateTransformNode"));

		IMFTopologyNode* pNode = NULL;

		HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE, &pNode);

		if(SUCCEEDED(hr)){
				hr = pNode->SetGUID(MF_TOPONODE_TRANSFORM_OBJECTID, clsid);
		}

		if(SUCCEEDED(hr)){
				
				*ppNode = pNode;
				(*ppNode)->AddRef();
		}

		SAFE_RELEASE(pNode);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateClsidOutputNode(const CLSID& clsid, IMFTopologyNode** ppNode){

		TRACE_PLAYER((L"Player::CreateClsidOutputNode"));

		HRESULT hr = S_OK;
		IMFMediaSink* pSink = NULL;
		IMFStreamSink* pStreamSink = NULL;
		IMFTopologyNode* pNode = NULL;
		DWORD dwStreamSinkCount;

		try{

				IF_FAILED_THROW(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IMFMediaSink, reinterpret_cast<void**>(&pSink)));

				IF_FAILED_THROW(hr = pSink->GetStreamSinkCount(&dwStreamSinkCount));

				IF_FAILED_THROW(hr = pSink->GetStreamSinkByIndex(0, &pStreamSink));

				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));

				IF_FAILED_THROW(hr = pNode->SetObject(pStreamSink));

				m_pSink = pSink;
				m_pSink->AddRef();

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pNode);
		SAFE_RELEASE(pStreamSink);
		SAFE_RELEASE(pSink);

		return hr;
}

HRESULT CMFNodePlayer::CreateVideoOutputNode(IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode){

		TRACE_PLAYER((L"Player::CreateVideoOutputNode"));

		IMFTopologyNode* pNode = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFActivate* pRendererActivate = NULL;

		GUID guidMajorType = GUID_NULL;
		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
    
    IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));

				if(MFMediaType_Video == guidMajorType){
						
					 IF_FAILED_THROW(hr = MFCreateVideoRendererActivate(m_hWindow, &pRendererActivate));
				}
				else{
						
						IF_FAILED_THROW(hr = E_FAIL);
				}

				IF_FAILED_THROW(hr = pNode->SetObject(pRendererActivate));

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pRendererActivate);
		
		return hr;
}

HRESULT CMFNodePlayer::CreateOutputNode(IMFStreamDescriptor* pSourceSD, IMFTopologyNode** ppNode){

		TRACE_PLAYER((L"Player::CreateOutputNode"));

		IMFTopologyNode* pNode = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFActivate* pActivate = NULL;

		GUID guidMajorType = GUID_NULL;
		HRESULT hr = S_OK;

		try{
				
				IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
    
    IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

				IF_FAILED_THROW(hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode));

				if(MFMediaType_Video == guidMajorType){
						
					 IF_FAILED_THROW(hr = MFCreateVideoRendererActivate(m_hWindow, &pActivate));
				}
				else if(MFMediaType_Audio == guidMajorType){
						
					 IF_FAILED_THROW(hr = MFCreateAudioRendererActivate(&pActivate));
				}
				else{
						
						IF_FAILED_THROW(hr = E_FAIL);
				}

				IF_FAILED_THROW(hr = pNode->SetObject(pActivate));

				*ppNode = pNode;
				(*ppNode)->AddRef();
		}
		catch(HRESULT){}

		SAFE_RELEASE(pNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pActivate);
		
		return hr;
}

#ifdef TEST_CUDA_DECODER
HRESULT CMFNodePlayer::CreateCudaTopology(IMFTopology** ppTopology, IMFMediaSource* pSource){

		TRACE_PLAYER((L"Player::CreateCudaTopology"));

		assert(pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildCudaTopology(pTopology, pSourcePD, pSource));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();

				UINT64 ui64Duration;

				if(SUCCEEDED(pSourcePD->GetUINT64(MF_PD_DURATION, &ui64Duration))){

						m_llDuration = ui64Duration;
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::BuildCudaTopology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, IMFMediaSource* pSource){
		
		TRACE_PLAYER((L"Player::BuildCudaTopology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pTransformNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		BOOL bSelected = FALSE;
		DWORD dwStreamCount;
		GUID guidMajorType = GUID_NULL;

		try{

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD i = 0; i < dwStreamCount; i++){

						IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD));

						if(bSelected){

								IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
								IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

								IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));

						  IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode));

								IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						  IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

								if(guidMajorType == MFMediaType_Video){

										IF_FAILED_THROW(hr = CreateTransformNode(CLSID_MFTCudaDecoder, &pTransformNode));
										IF_FAILED_THROW(hr = pTopology->AddNode(pTransformNode));

										IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pTransformNode, 0));
						    IF_FAILED_THROW(hr = pTransformNode->ConnectOutput(0, pOutputNode, 0));
								}
								else{

						    IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pOutputNode, 0));
								}

								SAFE_RELEASE(pTransformNode);
								SAFE_RELEASE(pHandler);
								SAFE_RELEASE(pOutputNode);
								SAFE_RELEASE(pSourceNode);
						}

						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pSourceSD);

		return hr;
}
#elif defined TEST_DXVA2_DECODER
HRESULT CMFNodePlayer::CreateDxva2Topology(IMFTopology** ppTopology, IMFMediaSource* pSource){

		TRACE_PLAYER((L"Player::CreateDxva2Topology"));

		assert(pSource != NULL);

		HRESULT hr = S_OK;

		IMFTopology* pTopology = NULL;
		IMFPresentationDescriptor* pSourcePD = NULL;

		try{

				IF_FAILED_THROW(hr = MFCreateTopology(&pTopology));

				IF_FAILED_THROW(hr = pSource->CreatePresentationDescriptor(&pSourcePD));

				IF_FAILED_THROW(hr = BuildDxva2Topology(pTopology, pSourcePD, pSource));
				
				*ppTopology = pTopology;
				(*ppTopology)->AddRef();

				UINT64 ui64Duration;

				if(SUCCEEDED(pSourcePD->GetUINT64(MF_PD_DURATION, &ui64Duration))){

						m_llDuration = ui64Duration;
				}
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTopology);
		SAFE_RELEASE(pSourcePD);
		
		return hr;
}

HRESULT CMFNodePlayer::BuildDxva2Topology(IMFTopology* pTopology, IMFPresentationDescriptor* pSourcePD, IMFMediaSource* pSource){
		
		TRACE_PLAYER((L"Player::BuildDxva2Topology"));

		assert(pTopology != NULL);

		HRESULT hr = S_OK;

		IMFStreamDescriptor* pSourceSD = NULL;
		IMFMediaTypeHandler* pHandler = NULL;
		IMFTopologyNode* pSourceNode = NULL;
		IMFTopologyNode* pTransformNode = NULL;
		IMFTopologyNode* pOutputNode = NULL;
		BOOL bSelected = FALSE;
		DWORD dwStreamCount;
		GUID guidMajorType = GUID_NULL;

		try{

				IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorCount(&dwStreamCount));

				for(DWORD i = 0; i < dwStreamCount; i++){

						IF_FAILED_THROW(hr = pSourcePD->GetStreamDescriptorByIndex(i, &bSelected, &pSourceSD));

						if(bSelected){

								IF_FAILED_THROW(hr = pSourceSD->GetMediaTypeHandler(&pHandler));
								IF_FAILED_THROW(hr = pHandler->GetMajorType(&guidMajorType));

								if(guidMajorType == MFMediaType_Video){

								  IF_FAILED_THROW(hr = CreateSourceStreamNode(pSource, pSourcePD, pSourceSD, &pSourceNode));

						    IF_FAILED_THROW(hr = CreateOutputNode(pSourceSD, &pOutputNode));

								  IF_FAILED_THROW(hr = pTopology->AddNode(pSourceNode));
						    IF_FAILED_THROW(hr = pTopology->AddNode(pOutputNode));

										IF_FAILED_THROW(hr = CreateTransformNode(CLSID_MFTDxva2Decoder, &pTransformNode));
										IF_FAILED_THROW(hr = pTopology->AddNode(pTransformNode));

										IF_FAILED_THROW(hr = pSourceNode->ConnectOutput(0, pTransformNode, 0));
						    IF_FAILED_THROW(hr = pTransformNode->ConnectOutput(0, pOutputNode, 0));

										IF_FAILED_THROW(hr = ConnectD3DManager(pTransformNode, pOutputNode));
								}

								SAFE_RELEASE(pTransformNode);
								SAFE_RELEASE(pHandler);
								SAFE_RELEASE(pOutputNode);
								SAFE_RELEASE(pSourceNode);
						}

						SAFE_RELEASE(pSourceSD);
				}
		}
		catch(HRESULT){}
		
		SAFE_RELEASE(pOutputNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pSourceNode);
		SAFE_RELEASE(pTransformNode);
		SAFE_RELEASE(pHandler);
		SAFE_RELEASE(pSourceSD);

		return hr;
}

HRESULT CMFNodePlayer::ConnectD3DManager(IMFTopologyNode* pTransformNode, IMFTopologyNode* pOutputNode){

		HRESULT hr = S_OK;
		IUnknown* pUnk = NULL;

		IMFMediaSink* pMediaSink = NULL;
		IMFStreamSink* pStreamSink = NULL;
		IMFActivate* pActivate = NULL;
		IDirect3DDeviceManager9* pManager = NULL;

		IMFTransform* pTransform = NULL;

		try{

				IF_FAILED_THROW(hr = pOutputNode->SetUINT32(MF_TOPONODE_STREAMID, 0));
				IF_FAILED_THROW(hr = pOutputNode->GetObject(&pUnk));
				IF_FAILED_THROW(hr = pUnk->QueryInterface(IID_PPV_ARGS(&pActivate)));
				SAFE_RELEASE(pUnk);

				IF_FAILED_THROW(hr = pActivate->ActivateObject(IID_PPV_ARGS(&pMediaSink)));
				SAFE_RELEASE(pActivate);
				IF_FAILED_THROW(hr = pMediaSink->GetStreamSinkById(0, &pStreamSink));

				IF_FAILED_THROW(hr = MFGetService(pStreamSink, MR_VIDEO_ACCELERATION_SERVICE, IID_IDirect3DDeviceManager9, reinterpret_cast<void**>(&pManager)));

				IF_FAILED_THROW(hr = CoCreateInstance(CLSID_MFTDxva2Decoder, NULL, CLSCTX_INPROC_SERVER, IID_IMFTransform, reinterpret_cast<void**>(&pTransform)));

				IF_FAILED_THROW(hr = pTransform->ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER, (ULONG_PTR)pManager));

				IF_FAILED_THROW(hr = pTransformNode->SetObject(pTransform));
		}
		catch(HRESULT){}

		SAFE_RELEASE(pTransform);
		SAFE_RELEASE(pManager);
		SAFE_RELEASE(pStreamSink);
		SAFE_RELEASE(pMediaSink);

		SAFE_RELEASE(pActivate);
		SAFE_RELEASE(pUnk);

		return hr;
}

#endif