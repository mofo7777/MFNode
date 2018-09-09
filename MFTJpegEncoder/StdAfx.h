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
#pragma comment(lib, "Gdiplus")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <mmintrin.h>
#include <Mfmp2dlna.h>
#include <Gdiplus.h>

using namespace Gdiplus;

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_TRANSFORM
#else
  #define MF_USE_LOGGING 0
#endif

#include "../Common/MFInclude.h"

// Project Files
#include "JpegStream.h"
#include "MFTJpegEncoder.h"

#endif