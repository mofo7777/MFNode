//----------------------------------------------------------------------------------------------
// DllMain.cpp
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

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK;

ClassFactoryData g_ClassFactories[] = {{ &CLSID_MFTA52Decoder, CA52Decoder::CreateInstance }};

const DWORD g_numClassFactories = ARRAY_SIZE(g_ClassFactories);

WCHAR* g_sFriendlyName = L"MFNode A52 Decoder";

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID /*lpReserved*/){

	switch(ul_reason_for_call){

	case DLL_PROCESS_ATTACH:
		g_hModule = (HMODULE)hModule;
		//DisableThreadLibraryCalls(hModule);
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

STDAPI DllCanUnloadNow(){

	if(!ClassFactory::IsLocked()){
		return S_OK;
	}
	else{
		return S_FALSE;
	}
}

STDAPI DllRegisterServer(){

	HRESULT hr;

	hr = RegisterObject(g_hModule, CLSID_MFTA52Decoder, g_sFriendlyName, TEXT("Both"));

	if(SUCCEEDED(hr)){

		MFT_REGISTER_TYPE_INFO tInInfo[] = {{ MFMediaType_Audio, MFAudioFormat_Dolby_AC3 }};
		MFT_REGISTER_TYPE_INFO tOutInfo[] = {{ MFMediaType_Audio, MFAudioFormat_Float }};

		hr = MFTRegister(CLSID_MFTA52Decoder, MFT_CATEGORY_AUDIO_DECODER, g_sFriendlyName, 0, ARRAY_SIZE(tInInfo), tInInfo, ARRAY_SIZE(tOutInfo), tOutInfo, NULL);
	}
	return hr;
}

STDAPI DllUnregisterServer(){

	UnregisterObject(CLSID_MFTA52Decoder);
	MFTUnregister(CLSID_MFTA52Decoder);
	return S_OK;
}

STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppv){

	ClassFactory* pFactory = NULL;

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	for(DWORD index = 0; index < g_numClassFactories; index++){

		if(*g_ClassFactories[index].pclsid == clsid){

			pFactory = new (std::nothrow)ClassFactory(g_ClassFactories[index].pfnCreate);

			if(pFactory){
				hr = S_OK;
			}
			else{
				hr = E_OUTOFMEMORY;
			}
			break;
		}
	}

	if(SUCCEEDED(hr)){
		hr = pFactory->QueryInterface(riid, ppv);
	}
	SAFE_RELEASE(pFactory);

	return hr;
}