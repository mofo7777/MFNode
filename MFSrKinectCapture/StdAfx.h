//-----------------------------------------------------------------------------------------------
// StdAfx.h
//-----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#pragma comment(lib, "Dmoguids")
#pragma comment(lib, "Msdmo")

//----------------------------------------------------------------------------------------------
// Microsoft Kinect SDK
// "C:\Program Files\Microsoft SDKs\Kinect\v1.7\lib\x86\"
#pragma comment(lib, "Kinect10")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <cguid.h>
#include <dmo.h>

//----------------------------------------------------------------------------------------------
// Microsoft Kinect SDK
// "C:\Program Files\Microsoft SDKs\Kinect\v1.7\inc\"
#include "NuiApi.h"

#ifdef _DEBUG
#define MF_USE_LOGGING 1
//#define MF_USE_LOGREFCOUNT
#else
#define MF_USE_LOGGING 0
#endif

//----------------------------------------------------------------------------------------------
// Common Project Files
#include "../Common/MFInclude.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "KinectSource_Def.h"
#include "KinectDevice.h"
#include "KinectStream.h"
#include "KinectSource.h"
#include "KinectSchemeHandler.h"

#endif