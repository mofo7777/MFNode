//----------------------------------------------------------------------------------------------
// MFMacro.h
//----------------------------------------------------------------------------------------------
#ifndef MFMACRO_H
#define MFMACRO_H

#ifndef MF_SAFE_RELEASE
#define MF_SAFE_RELEASE
template <class T> inline void SAFE_RELEASE(T*& p){
		
		if(p){
				p->Release();
				p = NULL;
		}
}
#endif

#ifndef MF_SAFE_DELETE
#define MF_SAFE_DELETE
template<class T> inline void SAFE_DELETE(T*& p){
		
		if(p){
				delete p;
				p = NULL;
		}
}
#endif

#ifndef MF_SAFE_DELETE_ARRAY
#define MF_SAFE_DELETE_ARRAY
template<class T> inline void SAFE_DELETE_ARRAY(T*& p){
		
		if(p){
				delete[] p;
				p = NULL;
		}
}
#endif

// Need to use other macro. When error, multiple call.
#ifndef IF_FAILED_RETURN
#if (_DEBUG && MF_USE_LOGGING)
  #define IF_FAILED_RETURN(hr) if(FAILED(hr)){ LOG_HRESULT(hr); return hr; }
#else
  #define IF_FAILED_RETURN(hr) if(FAILED(hr)){ return hr; }
#endif
#endif

#ifndef IF_FAILED_THROW
#if (_DEBUG && MF_USE_LOGGING)
  #define IF_FAILED_THROW(hr) if(FAILED(hr)){ LOG_HRESULT(hr); throw hr; }
#else
  #define IF_FAILED_THROW(hr) if(FAILED(hr)){ throw hr; }
#endif
#endif

#ifndef IF_ERROR_RETURN
#if (_DEBUG && MF_USE_LOGGING)
  #define IF_ERROR_RETURN(b) if(b == FALSE){ LOG_LAST_ERROR(); return b; }
#else
  #define IF_ERROR_RETURN(b) if(b == FALSE){ return b; }
#endif
#endif

#ifndef MF_THROW_LAST_ERROR
#define MF_THROW_LAST_ERROR
template<class T> inline void IF_FAILED_THROW_LAST_ERROR(T*& p, HRESULT hr){
		
		if(p == NULL){

				#if (_DEBUG && MF_USE_LOGGING)
				  LOG_LAST_ERROR();
				#endif

				throw hr;
		}
}
#endif

#ifndef CLOSE_HANDLE_IF
#if (_DEBUG && MF_USE_LOGGING)
  #define CLOSE_HANDLE_IF(h) if(h != INVALID_HANDLE_VALUE){ if(CloseHandle(h) == FALSE){ LOG_LAST_ERROR(); } h = INVALID_HANDLE_VALUE; }
#else
  #define CLOSE_HANDLE_IF(h) if(h != INVALID_HANDLE_VALUE){ CloseHandle(h); h = INVALID_HANDLE_VALUE; }
#endif
#endif

#ifndef CLOSE_EVENT_IF
#if (_DEBUG && MF_USE_LOGGING)
  #define CLOSE_EVENT_IF(h) if(h != NULL){ if(CloseHandle(h) == FALSE){ LOG_LAST_ERROR(); } h = NULL; }
#else
  #define CLOSE_EVENT_IF(h) if(h != NULL){ CloseHandle(h); h = NULL; }
#endif
#endif

#ifndef CLOSE_SOCKET_IF
#if (_DEBUG && MF_USE_LOGGING)
  #define CLOSE_SOCKET_IF(s) if(s != INVALID_SOCKET){ if(closesocket(s) == SOCKET_ERROR){ LOG_LAST_WSA_ERROR(); } s = INVALID_SOCKET; }
#else
  #define CLOSE_SOCKET_IF(s) if(s != INVALID_SOCKET){ closesocket(s); s = INVALID_SOCKET; }
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]) )
#endif

#endif