//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Pragma
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#pragma comment(lib, "comctl32")
#pragma comment(lib, "Propsys")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <Shellapi.h>

//----------------------------------------------------------------------------------------------
// STL
#include <string>
using std::wstring;

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_PLAYER
  //#define MF_TRACE_SESSION
  //#define MF_TRACE_EVENT
  //#define MF_TRACE_PIPELINE_EVENT
  //#define MF_TRACE_PLAYER_EVENT
#else
  #define MF_USE_LOGGING 0
#endif

#include "../Common/MFInclude.h"

#include "../MFSkVideoRenderer/VideoShaderEffect_h.h"
#include "../MFSkDxva2Renderer/Dxva2RendererSettings_h.h"

//----------------------------------------------------------------------------------------------
// Capture definition
#define VIDEO_CAPTURE_INDEX          0
#define AUDIO_CAPTURE_INDEX          2
#define VIDEO_CAPTURE_NAME           L"Microsoft LifeCam HD-5000"
#define AUDIO_CAPTURE_NAME           L"Microphone (SB Audigy)"
#define VIDEO_CAPTURE_TYPE_INDEX     2
#define AUDIO_CAPTURE_TYPE_INDEX     0

//----------------------------------------------------------------------------------------------
// Test
//#define TEST_CUDA_DECODER
#define TEST_DXVA2_DECODER

//----------------------------------------------------------------------------------------------
// Project Files
#include "resource.h"
#include "MFNodePlayer_definition.h"
#include "WaveMixerSession.h"
#include "MFNodePlayer.h"
#include "MFControlManager.h"
#include "MFNodeForm.h"

#endif