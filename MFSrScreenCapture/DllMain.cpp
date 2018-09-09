//----------------------------------------------------------------------------------------------
// DllMain.cpp
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HMODULE g_hModule;

DEFINE_CLASSFACTORY_SERVER_LOCK;

ClassFactoryData g_ClassFactories[] = { { &CLSID_MFSrScreenCapture, CScreenCaptureSchemeHandler::CreateInstance } };
      
DWORD g_numClassFactories = ARRAYSIZE(g_ClassFactories);

const TCHAR* szSchemeHandlerDescription = TEXT("MFNode Screen Capture Scheme Handler");
const TCHAR* szSchemeExtension = TEXT("screen:");

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID){
		
		switch(ul_reason_for_call){
		
		  case DLL_PROCESS_ATTACH:
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

		if(SUCCEEDED(hr = RegisterObject(g_hModule, CLSID_MFSrScreenCapture, szSchemeHandlerDescription, TEXT("Both")))){

				hr = RegisterSchemeHandler(CLSID_MFSrScreenCapture, szSchemeExtension, szSchemeHandlerDescription);
		}
		
		return hr;
}

STDAPI DllUnregisterServer(){
		
		UnregisterObject(CLSID_MFSrScreenCapture);

		UnregisterSchemeHandler(CLSID_MFSrScreenCapture, szSchemeExtension);

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