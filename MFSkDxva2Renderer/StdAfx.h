//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "winmm")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <mmsystem.h>

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
class CSinkDxva2Renderer;

extern HMODULE g_hModule;

#include "Dxva2RendererSettings_h.h"
#include "Dxva2Manager.h"
#include "StreamDxva2Renderer.h"
#include "SinkDxva2Renderer.h"

#endif