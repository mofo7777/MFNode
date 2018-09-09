//----------------------------------------------------------------------------------------------
// MFLinkList.h
//----------------------------------------------------------------------------------------------
#ifndef MFLINKLIST_H
#define MFLINKLIST_H

#pragma warning(push)
#pragma warning(disable : 4239)

template <class T> struct NoOp{

	void operator()(T&){}
};

template <class T> class List{

protected:

	struct Node{

		Node* prev;
		Node* next;
		T item;

		Node() : prev(NULL), next(NULL){}
		Node(T item) : prev(NULL), next(NULL){ this->item = item; }
		T Item() const { return item; }
	};

public:

	class POSITION{

		friend class List<T>;

	public:

		POSITION() : pNode(NULL){}
		bool operator==(const POSITION &p) const{ return pNode == p.pNode; }
		bool operator!=(const POSITION &p) const{ return pNode != p.pNode; }

	private:

		const Node* pNode;

		POSITION(Node* p) : pNode(p){}
	};

protected:

	Node m_anchor;
	DWORD m_count;

	Node* Front() const{ return m_anchor.next; }
	Node* Back() const{ return m_anchor.prev; }

	virtual HRESULT InsertAfter(T item, Node* pBefore){

		if(pBefore == NULL){
			return E_POINTER;
		}

		Node* pNode = new (std::nothrow)Node(item);

		if(pNode == NULL){
			return E_OUTOFMEMORY;
		}

		Node* pAfter = pBefore->next;

		pBefore->next = pNode;
		pAfter->prev = pNode;

		pNode->prev = pBefore;
		pNode->next = pAfter;

		m_count++;

		return S_OK;
	}

	virtual HRESULT GetItem(const Node* pNode, T* ppItem){

		if(pNode == NULL || ppItem == NULL){
			return E_POINTER;
		}

		*ppItem = pNode->item;
		return S_OK;
	}

	virtual HRESULT RemoveItem(Node* pNode, T* ppItem){

		if(pNode == NULL){
			return E_POINTER;
		}

		assert(pNode != &m_anchor);

		if(pNode == &m_anchor){
			return E_INVALIDARG;
		}

		T item;

		pNode->next->prev = pNode->prev;

		pNode->prev->next = pNode->next;

		item = pNode->item;
		delete pNode;

		m_count--;

		if(ppItem){
			*ppItem = item;
		}

		return S_OK;
	}

public:

	List(){ m_anchor.next = &m_anchor; m_anchor.prev = &m_anchor; m_count = 0; }
	virtual ~List(){ Clear(); }

	HRESULT InsertBack(T item){ return InsertAfter(item, m_anchor.prev); }
	HRESULT InsertFront(T item){ return InsertAfter(item, &m_anchor); }

	HRESULT RemoveBack(T* ppItem){

		if(IsEmpty()){
			return E_FAIL;
		}
		else{
			return RemoveItem(Back(), ppItem);
		}
	}

	HRESULT RemoveFront(T* ppItem){

		if(IsEmpty()){
			return E_FAIL;
		}
		else{
			return RemoveItem(Front(), ppItem);
		}
	}

	HRESULT GetBack(T* ppItem){

		if(IsEmpty()){
			return E_FAIL;
		}
		else{
			return GetItem(Back(), ppItem);
		}
	}

	HRESULT GetFront(T* ppItem){

		if(IsEmpty()){
			return E_FAIL;
		}
		else{
			return GetItem(Front(), ppItem);
		}
	}

	DWORD GetCount() const { return m_count; }

	bool IsEmpty() const{ return (GetCount() == 0); }

	template <class FN> void Clear(FN& clear_fn){

		Node* n = m_anchor.next;

		while(n != &m_anchor){

			clear_fn(n->item);

			Node* tmp = n->next;
			delete n;
			n = tmp;
		}

		m_anchor.next = &m_anchor;
		m_anchor.prev = &m_anchor;

		m_count = 0;
	}

	virtual void Clear(){ Clear<NoOp<T>>(NoOp<T>()); }

	POSITION FrontPosition(){

		if(IsEmpty()){
			return POSITION(NULL);
		}
		else{
			return POSITION(Front());
		}
	}

	POSITION EndPosition() const{ return POSITION(); }

	HRESULT GetItemPos(POSITION pos, T* ppItem){

		if(pos.pNode){
			return GetItem(pos.pNode, ppItem);
		}
		else{
			return E_FAIL;
		}
	}

	POSITION Next(const POSITION pos){

		if(pos.pNode && (pos.pNode->next != &m_anchor)){
			return POSITION(pos.pNode->next);
		}
		else{
			return POSITION(NULL);
		}
	}

	HRESULT Remove(POSITION& pos, T* ppItem){

		if(pos.pNode){

			Node* pNode = const_cast<Node*>(pos.pNode);

			pos = POSITION();

			return RemoveItem(pNode, ppItem);
		}
		else{
			return E_INVALIDARG;
		}
	}
};

class ComAutoRelease{

public:

	void operator()(IUnknown* p){

		if(p){
			p->Release();
		}
	}
};

class MemDelete{

public:

	void operator()(void* p){

		if(p){
			delete p;
		}
	}
};

template <class T, bool NULLABLE = FALSE> class ComPtrList : public List<T*>{

public:

	typedef T* Ptr;

	void Clear(){ List<Ptr>::Clear(ComAutoRelease()); }
	~ComPtrList(){ Clear(); }

protected:

	HRESULT InsertAfter(Ptr item, Node* pBefore){

		if(!item && !NULLABLE){
			return E_POINTER;
		}

		if(item){
			item->AddRef();
		}

		HRESULT hr = List<Ptr>::InsertAfter(item, pBefore);

		if(FAILED(hr)){
			SAFE_RELEASE(item);
		}
		return hr;
	}

	HRESULT GetItem(const Node* pNode, Ptr* ppItem){

		Ptr pItem = NULL;

		HRESULT hr = List<Ptr>::GetItem(pNode, &pItem);

		if(SUCCEEDED(hr)){

			assert(pItem || NULLABLE);

			if(pItem){
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}
		}
		return hr;
	}

	HRESULT RemoveItem(Node* pNode, Ptr* ppItem){

		Ptr pItem = NULL;

		HRESULT hr = List<Ptr>::RemoveItem(pNode, &pItem);

		if(SUCCEEDED(hr)){

			assert(pItem || NULLABLE);

			if(ppItem && pItem){
				*ppItem = pItem;
				(*ppItem)->AddRef();
			}

			SAFE_RELEASE(pItem);
		}

		return hr;
	}
};

#pragma warning(pop)

#endif