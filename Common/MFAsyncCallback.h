//----------------------------------------------------------------------------------------------
// MFAsyncCallback.h
//----------------------------------------------------------------------------------------------
#ifndef MFASYNCCALLBACK_H
#define MFASYNCCALLBACK_H

template<class T> class CAsyncCallback : public IMFAsyncCallback{

  public: 
				
				typedef HRESULT (T::*InvokeFn)(IMFAsyncResult*);

				CAsyncCallback(T* pParent, InvokeFn fn) : m_pParent(pParent), m_pInvokeFn(fn){}

				// IUnknown
				STDMETHODIMP QueryInterface(REFIID iid, void** ppv){
						
						if(!ppv){
								return E_POINTER;
						}
						
						if(iid == __uuidof(IUnknown)){
								*ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
						}
						else if(iid == __uuidof(IMFAsyncCallback)){
								*ppv = static_cast<IMFAsyncCallback*>(this);
						}
						else{
								*ppv = NULL;
								return E_NOINTERFACE;
						}
						
						AddRef();
						return S_OK;
				}
				
				STDMETHODIMP_(ULONG) AddRef(){ return m_pParent->AddRef(); }
				STDMETHODIMP_(ULONG) Release(){ return m_pParent->Release(); }

				// IMFAsyncCallback methods
				STDMETHODIMP GetParameters(DWORD*, DWORD*){ return E_NOTIMPL; }
				STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult){ return (m_pParent->*m_pInvokeFn)(pAsyncResult); }

				T* m_pParent;
				InvokeFn m_pInvokeFn;
};

#endif