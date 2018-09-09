//----------------------------------------------------------------------------------------------
// DllMain.cpp
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
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK;

ClassFactoryData g_ClassFactories[] = { { &CLSID_MFSrKinectCapture, CKinectSchemeHandler::CreateInstance } };
      
DWORD g_numClassFactories = ARRAYSIZE(g_ClassFactories);

const TCHAR* szSchemeHandlerDescription = TEXT("MFNode Kinect Scheme Handler");
const TCHAR* szSchemeExtension = TEXT("kinect:");

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID){
		
		switch(ul_reason_for_call){
		
		  case DLL_PROCESS_ATTACH:
						DisableThreadLibraryCalls((HMODULE)hModule);
						g_hModule = (HMODULE)hModule;
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
		
		HRESULT hr = S_OK;

		if(SUCCEEDED(hr = RegisterObject(g_hModule, CLSID_MFSrKinectCapture, szSchemeHandlerDescription, TEXT("Both")))){

				hr = RegisterSchemeHandler(CLSID_MFSrKinectCapture, szSchemeExtension, szSchemeHandlerDescription);
		}
		
		return hr;
}

STDAPI DllUnregisterServer(){
		
		UnregisterObject(CLSID_MFSrKinectCapture);
		UnregisterSchemeHandler(CLSID_MFSrKinectCapture, szSchemeExtension);

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