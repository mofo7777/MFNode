//-----------------------------------------------------------------------------------------------
// StdAfx.h
//-----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// A52 Audio Library
#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\LibA52Audio")
#else
#pragma comment(lib, "..\\Release\\LibA52Audio")
#endif

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>

#ifdef _DEBUG
#define MF_USE_LOGGING 1
//#define MF_USE_LOGREFCOUNT
//#define MF_TRACE_TRANSFORM
#else
#define MF_USE_LOGGING 0
#endif

//----------------------------------------------------------------------------------------------
// Common Project Files
#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// A52 Audio Library
#include "../LibA52Audio/inttypes.h"
#include "../LibA52Audio/a52.h"
#include "../LibA52Audio/a52_internal.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "A52Decoder.h"

#endif