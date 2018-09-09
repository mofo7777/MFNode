//----------------------------------------------------------------------------------------------
// MFAsyncState.h
//----------------------------------------------------------------------------------------------
#ifndef MFASYNCSTATE_H
#define MFASYNCSTATE_H

class IMFAsyncState : public IUnknown{

  public:
				
				virtual MFNodeMediaEvent EventType() = 0;
};

class CMFAsyncState : public IMFAsyncState{

  public:
				
				CMFAsyncState(MFNodeMediaEvent type) : m_nRefCount(1), m_eventType(type){}
				virtual ~CMFAsyncState(){}
				
				STDMETHODIMP QueryInterface(REFIID riid, void** ppv){

						HRESULT hr = S_OK;

						if(ppv == NULL){
								return E_POINTER;
						}

						if(riid == IID_IUnknown){
								*ppv = static_cast<IUnknown*>(this);
						}
						else if(riid == IID_IMFAsyncState){
								*ppv = static_cast<IMFAsyncState*>(this);
						}
						else{
								*ppv = NULL;
								hr = E_NOINTERFACE;
						}

						if(SUCCEEDED(hr))
								AddRef();

						return hr;
				}

    STDMETHODIMP_(ULONG) AddRef(){

								return InterlockedIncrement(&m_nRefCount);
				}
				
				STDMETHODIMP_(ULONG) Release(){

						ULONG refCount = InterlockedDecrement(&m_nRefCount);

						if(refCount == 0){
								delete this;
						}

						return refCount;
				}

				virtual MFNodeMediaEvent EventType(){ return m_eventType; }

  private:
				
				volatile long m_nRefCount;
				MFNodeMediaEvent m_eventType;
};

#endif