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
//#define MF_TRACE_HANDLER
//#define MF_TRACE_SOURCE
//#define MF_TRACE_STREAM
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
// Project Files
class CMFMpeg12Source;

#include "MFMpeg12Definition.h"
#include "MFByteStreamHandler.h"
#include "MFMpeg12Stream.h"
#include "MFMpeg12Source.h"

#endif