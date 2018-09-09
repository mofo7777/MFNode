//----------------------------------------------------------------------------------------------
// MFLogging.h
//----------------------------------------------------------------------------------------------
#ifndef MFLOGGING_H
#define MFLOGGING_H

#ifdef _DEBUG

class DebugLog{

public:

	static void Initialize(){
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
		//_CrtSetBreakAlloc(203);
	}

	static void Trace(const WCHAR* sFormatString, ...){

		HRESULT hr = S_OK;
		va_list va;

		const DWORD TRACE_STRING_LEN = 512;

		WCHAR message[TRACE_STRING_LEN];

		va_start(va, sFormatString);
		hr = StringCchVPrintf(message, TRACE_STRING_LEN, sFormatString, va);
		va_end(va);

		if(SUCCEEDED(hr)){

			size_t size = _tcslen(message);

			if(size != 0 && size < TRACE_STRING_LEN){
				message[size] = '\n';
				message[size + 1] = '\0';
			}

			_CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "%S", message);
		}
	}

	static void TraceNoEndLine(const WCHAR* sFormatString, ...){

		HRESULT hr = S_OK;
		va_list va;

		const DWORD TRACE_STRING_LEN = 512;

		WCHAR message[TRACE_STRING_LEN];

		va_start(va, sFormatString);
		hr = StringCchVPrintf(message, TRACE_STRING_LEN, sFormatString, va);
		va_end(va);

		if(SUCCEEDED(hr)){

			_CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, "%S", message);
		}
	}

	static void Close(){
		int bLeak = _CrtDumpMemoryLeaks();
		assert(bLeak == FALSE);
	}
};

#define TRACE_INIT() DebugLog::Initialize()
#define TRACE(x) DebugLog::Trace x
#define TRACE_NO_END_LINE(x) DebugLog::TraceNoEndLine x
#define TRACE_CLOSE() DebugLog::Close()

inline HRESULT _LOG_HRESULT(HRESULT hr, const char* sFileName, long lLineNo){

	if(FAILED(hr)){
		TRACE((L"\n%S - Line: %d hr = %s\n", sFileName, lLineNo, MFErrorString(hr)));
	}

	return hr;
}

#define LOG_HRESULT(hr)      _LOG_HRESULT(hr, __FILE__, __LINE__)
#define LOG_LAST_ERROR()     _LOG_HRESULT(HRESULT_FROM_WIN32(GetLastError()), __FILE__, __LINE__)

// Todo WSAGetLastError()
#define LOG_LAST_WSA_ERROR() _LOG_HRESULT(E_FAIL, __FILE__, __LINE__)

#ifdef MF_USE_LOGREFCOUNT
#define TRACE_REFCOUNT(x) DebugLog::Trace x
#else
#define TRACE_REFCOUNT(x)
#endif

#ifdef MF_TRACE_HANDLER
#define TRACE_HANDLER(x) DebugLog::Trace x
#else
#define TRACE_HANDLER(x)
#endif

#ifdef MF_TRACE_SOURCE
#define TRACE_SOURCE(x) DebugLog::Trace x
#else
#define TRACE_SOURCE(x)
#endif

#ifdef MF_TRACE_EVENTSOURCE
#define TRACE_EVENTSOURCE(x) DebugLog::Trace x
#else
#define TRACE_EVENTSOURCE(x)
#endif

#ifdef MF_TRACE_OPSOURCE
#define TRACE_OPSOURCE(x) DebugLog::Trace x
#else
#define TRACE_OPSOURCE(x)
#endif

#ifdef MF_TRACE_STREAMSOURCE
#define TRACE_STREAMSOURCE(x) DebugLog::Trace x
#else
#define TRACE_STREAMSOURCE(x)
#endif

#ifdef MF_TRACE_MEDIATYPESOURCE
#define TRACE_MEDIATYPESOURCE(x) DebugLog::Trace x
#else
#define TRACE_MEDIATYPESOURCE(x)
#endif

#ifdef MF_TRACE_STREAM
#define TRACE_STREAM(x) DebugLog::Trace x
#else
#define TRACE_STREAM(x)
#endif

#ifdef MF_TRACE_EVENTSTREAM
#define TRACE_EVENTSTREAM(x) DebugLog::Trace x
#else
#define TRACE_EVENTSTREAM(x)
#endif

#ifdef MF_TRACE_PARSER
#define TRACE_PARSER(x) DebugLog::Trace x
#else
#define TRACE_PARSER(x)
#endif

#ifdef MF_TRACE_BYTESTREAM
#define TRACE_BYTESTREAM(x) DebugLog::Trace x
#else
#define TRACE_BYTESTREAM(x)
#endif

#ifdef MF_TRACE_TRANSFORM
#define TRACE_TRANSFORM(x) DebugLog::Trace x
#else
#define TRACE_TRANSFORM(x)
#endif

#ifdef MF_TRACE_EVENT
#define TRACE_EVENT(x) DebugLog::Trace x
#else
#define TRACE_EVENT(x)
#endif

#ifdef MF_TRACE_TIME
#define TRACE_TIME(x) DebugLog::Trace x
#else
#define TRACE_TIME(x)
#endif

#ifdef MF_TRACE_SINK
#define TRACE_SINK(x) DebugLog::Trace x
#else
#define TRACE_SINK(x)
#endif

#ifdef MF_TRACE_SCHEME
#define TRACE_SCHEME(x) DebugLog::Trace x
#else
#define TRACE_SCHEME(x)
#endif

#ifdef MF_TRACE_SESSION
#define TRACE_SESSION(x) DebugLog::Trace x
#else
#define TRACE_SESSION(x)
#endif

#ifdef MF_TRACE_PLAYER
#define TRACE_PLAYER(x) DebugLog::Trace x
#else
#define TRACE_PLAYER(x)
#endif

#ifdef MF_TRACE_PIPELINE_EVENT
#define TRACE_PIPELINE_EVENT(x) DebugLog::Trace x
#else
#define TRACE_PIPELINE_EVENT(x)
#endif

#ifdef MF_TRACE_PLAYER_EVENT
#define TRACE_PLAYER_EVENT(x) DebugLog::Trace x
#else
#define TRACE_PLAYER_EVENT(x)
#endif

#else
#define TRACE_INIT()
#define TRACE(x)
#define TRACE_NO_END_LINE(x)
#define TRACE_CLOSE()
#define LOG_HRESULT(hr) hr
#define LOG_LAST_ERROR()
#define LOG_LAST_WSA_ERROR()
#define TRACE_REFCOUNT(x)
#define TRACE_HANDLER(x)
#define TRACE_SOURCE(x)
#define TRACE_EVENTSOURCE(x)
#define TRACE_OPSOURCE(x)
#define TRACE_STREAMSOURCE(x)
#define TRACE_MEDIATYPESOURCE(x)
#define TRACE_STREAM(x)
#define TRACE_EVENTSTREAM(x)
#define TRACE_PARSER(x)
#define TRACE_BYTESTREAM(x)
#define TRACE_TRANSFORM(x)
#define TRACE_EVENT(x)
#define TRACE_TIME(x)
#define TRACE_SINK(x)
#define TRACE_SCHEME(x)
#define TRACE_SESSION(x)
#define TRACE_PLAYER(x)
#define TRACE_PIPELINE_EVENT(x)
#define TRACE_PLAYER_EVENT(x)
#endif

#endif