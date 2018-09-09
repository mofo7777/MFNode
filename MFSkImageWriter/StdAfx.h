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
// STL
#include <string>
#include <sstream>
using std::wstring;
using std::wostringstream;

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
class CSkinkImageWriter;

#include "StreamImage.h"
#include "SinkImageWriter.h"

#endif