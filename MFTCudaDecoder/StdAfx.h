//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Cuda SDK v7.0
// "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v7.0\lib\Win32\"
#pragma comment(lib, "cuda")
#pragma comment(lib, "nvcuvid")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <mmintrin.h>

//----------------------------------------------------------------------------------------------
// STL
#include <string>
using std::wstring;

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_TRANSFORM
  //#define MF_TRACE_TIME
#else
  #define MF_USE_LOGGING 0
#endif

#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Cuda SDK v7.0
// "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v7.0\include\"
#include <cuda.h>
#include <nvcuvid.h>
#include <cuda_runtime_api.h>

//----------------------------------------------------------------------------------------------
// Private trace
//#define TRACE_PICTURE_PARAM
//#define TRACE_SLICE
//#define TRACE_PICTURE_OUTPUT
//#define TRACE_TIME_REFERENCE
//#define TRACE_SET_PTS
//#define TRACE_CUDA_PTS
//#define TRACE_INPUT_REFERENCE
//#define TRACE_OUTPUT_PTS

// {3584CA5C-947A-46B2-AC8E-48B0DB713C58}
DEFINE_GUID(MFSampleExtension_PictureType, 0x3584ca5c, 0x947a, 0x46b2, 0xac, 0x8e, 0x48, 0xb0, 0xdb, 0x71, 0x3c, 0x58);

//----------------------------------------------------------------------------------------------
// Project Files
#include "MFTCudaDecoder_Def.h"
#include "CudaManager.h"
#include "CudaDecoder.h"
#include "CudaFrame.h"
#include "MFTCudaDecoder.h"

#endif