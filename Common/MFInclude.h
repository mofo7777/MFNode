//----------------------------------------------------------------------------------------------
// MFInclude.h
//----------------------------------------------------------------------------------------------
#ifndef MFINCLUDE_H
#define MFINCLUDE_H

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "wmcodecdspuuid")
#pragma comment(lib, "Ws2_32")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "dxva2")
#pragma comment(lib, "Evr")

//----------------------------------------------------------------------------------------------
// STL
#include <queue>
using std::queue;

#include <atlbase.h>
#include <assert.h>
#include <strsafe.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#ifndef RETURN_STRING
#define RETURN_STRING(x) case x: return L#x
#endif

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return L#val
#endif

#ifndef VALUE_NOT_FOUND
#define VALUE_NOT_FOUND(val) return L#val
#endif

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
//----------------------------------------------------------------------------------------------
#include <Shlwapi.h>
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <Mftransform.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>
#include <evr.h>
#include <uuids.h>
#include <wmcodecdsp.h>
#include <ws2tcpip.h>

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
#include <Dxva2api.h>
#include <dxva.h>
#include <dxvahd.h>

//----------------------------------------------------------------------------------------------
// Common Files
//----------------------------------------------------------------------------------------------
#include "MFNodeEnum.h"
#include "MFNodeGuid.h"
#include "MFMpeg2Definition.h"
#include "MFTrace.h"
#include "MFLogging.h"
#include "MFTExternTrace.h"
#include "MFMacro.h"
#include "MFState.h"
#include "MFCriticSection.h"
#include "MFClassFactory.h"
#include "MFLogGuid.h"
#include "MFLogMediaType.h"
#include "MFRegistry.h"
#include "MFTime.h"
#include "MFAsyncCallback.h"
#include "MFBuffer.h"
#include "MFGrowArray.h"
#include "MFLinkList.h"
#include "MFTinyMap.h"
#include "MFOperationQueue.h"
#include "MFAsyncState.h"
#include "MFFile.h"
#include "MFTraceSample.h"
#include "MFReadParam.h"
#include "MFSafeQueue.h"

#endif