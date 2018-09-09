//----------------------------------------------------------------------------------------------
// MFOperationQueue.h
//----------------------------------------------------------------------------------------------
#ifndef MFOPERATIONQUEUE_H
#define MFOPERATIONQUEUE_H

template <class OP_TYPE> class CMFOperationQueue : public IUnknown{

  public:

    typedef ComPtrList<OP_TYPE> OpList;

				HRESULT QueueOperation(OP_TYPE* pOp){
						
						HRESULT hr = S_OK;

						AutoLock lock(m_CriticSection);

						hr = m_OpQueue.InsertBack(pOp);
						
						if(SUCCEEDED(hr)){
								hr = ProcessQueue();
						}
						return hr;
				}
				
  protected:

				HRESULT ProcessQueue(){
						
						HRESULT hr = S_OK;
						
						if(m_OpQueue.GetCount() > 0){
								hr = MFPutWorkItem(MFASYNC_CALLBACK_QUEUE_STANDARD, &m_OnProcessQueue, NULL);
						}
						return hr;
				}

				HRESULT ProcessQueueAsync(IMFAsyncResult*){
						
						HRESULT hr = S_OK;

						AutoLock lock(m_CriticSection);

						if(m_OpQueue.GetCount() > 0){
								
								OP_TYPE* pOp = NULL;

								try{

										IF_FAILED_THROW(hr = m_OpQueue.GetFront(&pOp));

										IF_FAILED_THROW(hr = ValidateOperation(pOp));

										IF_FAILED_THROW(hr = m_OpQueue.RemoveFront(NULL));

										(void)DispatchOperation(pOp);
								}
								catch(HRESULT){}

								SAFE_RELEASE(pOp);
						}
						
						return hr;
				}

    virtual HRESULT DispatchOperation(OP_TYPE*) = 0;
    virtual HRESULT ValidateOperation(OP_TYPE*) = 0;

    CMFOperationQueue(CriticSection& critsec) : m_OnProcessQueue(this, &CMFOperationQueue::ProcessQueueAsync), m_CriticSection(critsec){}
				virtual ~CMFOperationQueue(){}
				
  protected:
				
				OpList m_OpQueue;
				CriticSection& m_CriticSection;

				CAsyncCallback<CMFOperationQueue> m_OnProcessQueue;
};

#endif