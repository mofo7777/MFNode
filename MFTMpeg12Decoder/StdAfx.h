//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Mpeg Audio Library
#ifdef _DEBUG
  #pragma comment(lib, "..\\Debug\\LibMpegAudio")
#else
  #pragma comment(lib, "..\\Release\\LibMpegAudio")
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
// Mpeg Audio Library
#include "..\LibMpegAudio\Mpeg12LibDefinition.h"
#include "..\LibMpegAudio\Mpeg12Decoder.h"

//----------------------------------------------------------------------------------------------
// Test
//#define TRACE_REFERENCE_TIME

//----------------------------------------------------------------------------------------------
// Project Files
#include "MFTMpegAudioDecoder.h"

#endif