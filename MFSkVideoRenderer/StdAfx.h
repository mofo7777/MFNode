//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
#ifdef _DEBUG
  //#pragma comment(lib, "C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9d")
  #pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9d")
#else
  //#pragma comment(lib, "C:\\Program Files\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9")
  #pragma comment(lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9")
#endif

#pragma comment(lib, "winmm")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <mmsystem.h>

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
//#include "C:\Program Files\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"
#include "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\d3dx9.h"

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_SINK
  //#define MF_TRACE_STREAM
#else
  #define MF_USE_LOGGING 0
#endif

#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Project Files
class CSinkVideoRenderer;

extern HMODULE g_hModule;

#include "VideoShaderEffect_h.h"
#include "resource.h"
#include "Quad.h"
#include "DirectX9Manager.h"
#include "StreamVideoRenderer.h"
#include "SinkVideoRenderer.h"

#endif