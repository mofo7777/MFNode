//----------------------------------------------------------------------------------------------
// MFSourceOperation.h
//----------------------------------------------------------------------------------------------
#ifndef MFSOURCEOPERATION_H
#define MFSOURCEOPERATION_H

class CMFSourceOperation : RefCountedObject, public IUnknown{

  public:

				// IUnknown
    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(){ return RefCountedObject::AddRef(); }
    STDMETHODIMP_(ULONG) Release(){ return RefCountedObject::Release(); }

				enum Operation{OP_START, OP_PAUSE, OP_STOP, OP_REQUEST_DATA, OP_END_OF_STREAM};

				static HRESULT CreateOp(Operation, CMFSourceOperation**);
				static HRESULT CreateStartOp(IMFPresentationDescriptor*, CMFSourceOperation**);

    CMFSourceOperation(Operation op) : m_op(op){ PropVariantInit(&m_data); }
    virtual ~CMFSourceOperation(){ PropVariantClear(&m_data); }

    HRESULT SetData(const PROPVARIANT& var){ return PropVariantCopy(&m_data, &var); }

				Operation Op() const { return m_op; }
				const PROPVARIANT& Data() { return m_data;}

  protected:

				Operation   m_op;
				PROPVARIANT m_data;
};

class CMFStartOperation : public CMFSourceOperation{

		public:
				
				CMFStartOperation(IMFPresentationDescriptor* pPD) : CMFSourceOperation(CMFSourceOperation::OP_START), m_pPD(pPD){ if(m_pPD){ m_pPD->AddRef(); } }
				~CMFStartOperation(){ SAFE_RELEASE(m_pPD); }

				HRESULT GetPresentationDescriptor(IMFPresentationDescriptor**);

  protected:
				
				IMFPresentationDescriptor* m_pPD;
};

inline HRESULT CMFSourceOperation::QueryInterface(REFIID riid, void** ppv){
		
		static const QITAB qit[] = { QITABENT(CMFSourceOperation, IUnknown), {0}};
		
		return QISearch(this, qit, riid, ppv);
}

inline HRESULT CMFSourceOperation::CreateOp(CMFSourceOperation::Operation op, CMFSourceOperation** ppOp){
		
		if(ppOp == NULL){
				return E_POINTER;
		}

		CMFSourceOperation* pOp = new (std::nothrow)CMFSourceOperation(op);
		
		if(pOp  == NULL){
				return E_OUTOFMEMORY;
		}
		
		*ppOp = pOp;

		return S_OK;
}

inline HRESULT CMFSourceOperation::CreateStartOp(IMFPresentationDescriptor* pPD, CMFSourceOperation** ppOp){
		
		if(ppOp == NULL){
				return E_POINTER;
		}

		CMFSourceOperation* pOp = new (std::nothrow)CMFStartOperation(pPD);
		
		if(pOp == NULL){
				return E_OUTOFMEMORY;
		}

		*ppOp = pOp;
		
		return S_OK;
}

inline HRESULT CMFStartOperation::GetPresentationDescriptor(IMFPresentationDescriptor** ppPD){
		
		if(ppPD == NULL){
				return E_POINTER;
		}
		
		if(m_pPD == NULL){
				return MF_E_INVALIDREQUEST;
		}
		
		*ppPD = m_pPD;
		(*ppPD)->AddRef();
		
		return S_OK;
}

#endif