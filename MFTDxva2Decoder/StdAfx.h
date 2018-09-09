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

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_TRANSFORM
  //#define TRACE_TIME_REFERENCE
  //#define TRACE_INPUT_REFERENCE
  //#define TRACE_SET_PTS
  //#define TRACE_MPEG_PTS
  #define TRACE_DXVA2_DECODER_CAPS
  //#define TRACE_DXVA2
#else
  #define MF_USE_LOGGING 0
#endif

#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "MFDxva2Definition.h"
#include "TSScheduler.h"
#include "SurfaceParam.h"
#include "Dxva2Decoder.h"

#endif