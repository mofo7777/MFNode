
//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Mpeg2 Lib
#ifdef _DEBUG
#pragma comment(lib, "..\\Debug\\LibMpeg2")
#else
#pragma comment(lib, "..\\Release\\LibMpeg2")
#endif

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>

#ifdef _DEBUG
#define MF_USE_LOGGING 1
//#define MF_USE_LOGREFCOUNT
//#define MF_TRACE_TIME
#else
#define MF_USE_LOGGING 0
#endif

//----------------------------------------------------------------------------------------------
// Common Project Files
#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "Mpeg2Decoder_LibDef.h"
#include "Mpeg2Decoder.h"
#include "MFTVideoMpeg2.h"

#endif