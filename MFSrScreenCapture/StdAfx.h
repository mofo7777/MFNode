//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>

#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_SOURCE
  //#define MF_TRACE_EVENTSOURCE
  //#define MF_TRACE_SCHEME
  //#define MF_TRACE_STREAMSOURCE
#else
  #define MF_USE_LOGGING 0
#endif

//----------------------------------------------------------------------------------------------
// Common Project Files
#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "ScreenCaptureSchemeHandler.h"
#include "ScreenCapture_def.h"
#include "ScreenCaptureStream.h"
#include "ScreenCapture.h"

#endif