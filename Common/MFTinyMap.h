//----------------------------------------------------------------------------------------------
// MFTinyMap.h
//----------------------------------------------------------------------------------------------
#ifndef MFTINYMAP_H
#define MFTINYMAP_H

template <class Key, class Value> struct Pair{
		
		Key key;
		Value value;

		Pair(){}
		Pair(Key k, Value v){ key = k; value = v; }
};

template <class Key, class Value> class CTinyMap : List< Pair<Key, Value> >{

  protected:

				typedef Pair<Key, Value> pair_type;

  public:

				CTinyMap(){ Clear(); }
				virtual ~CTinyMap(){}

				HRESULT Insert(Key k, Value v){
						
						HRESULT hr = S_OK;

						Node* pNode = Front();
						
						while(TRUE){
								
								if(pNode == &m_anchor){
										
										hr = InsertBack(pair_type(k, v));
										break;
								}
								else if(pNode->item.key == k){
										
										hr = MF_E_INVALID_KEY; 
										break;
								}
								else if(pNode->item.key > k){
										
										hr = InsertAfter(pair_type(k, v), pNode->prev);
										break;
								}
								
								pNode = pNode->next;
						}
						
						return hr;
				}

				HRESULT Remove(Key k){
						
						HRESULT hr = E_FAIL;

						Node* pNode = Front();
						Node* pToRemove = NULL;

						while(TRUE){
								
								if(pNode == &m_anchor){										
										break;
								}
								else if(pNode->item.key == k){
										
										pToRemove = pNode; 
										break;
								}
								else if(pNode->item.key > k){
										
										hr = MF_E_INVALID_KEY;
										break;
								}
								
								pNode = pNode->next;
						}

						if(pToRemove){
								
								hr = RemoveItem(pToRemove, NULL);
						}

						return hr;
				}

				HRESULT Find(Key k, Value* pv){
						
						HRESULT hr = S_OK;
						BOOL bFound = FALSE;

						pair_type pair;

						POSITION pos = List<pair_type>::FrontPosition();

						while(pos != List<pair_type>::EndPosition()){
								
								hr = GetItemPos(pos, &pair);

								if(FAILED(hr)){
										break;
								}

								if(pair.key == k){
										
										if(pv){
												*pv = pair.value; 
										}
										
										bFound = TRUE;
										break;
								}

								if(pair.key > k){										
										break;
								}

								pos = List<pair_type>::Next(pos);
						}
						
						return (bFound ? S_OK : MF_E_INVALID_KEY);
				}

				void Clear(){ List<pair_type>::Clear(); }

				template <class FN> void ClearValues(FN& clear_fn){
						
						Node* n = m_anchor.next;

						while(n != &m_anchor){
								
								clear_fn(n->item.value);

								Node* tmp = n->next;
								delete n;
								n = tmp;
						}

						m_anchor.next = &m_anchor;
						m_anchor.prev = &m_anchor;

						m_count = 0;
				}

				DWORD GetCount() const{ return List<pair_type>::GetCount(); }

				class MAPPOS{
						
			  			friend class CTinyMap;

						  typedef List<pair_type>::POSITION LISTPOS;

				  public:
								
								MAPPOS(){}
								bool operator==(const MAPPOS &p) const{ return pos == p.pos; }
								bool operator!=(const MAPPOS &p) const{ return pos != p.pos; }

				  private:
								
								LISTPOS pos;

								MAPPOS(LISTPOS p) : pos(p){}
				};

				MAPPOS FrontPosition(){ return MAPPOS( List<pair_type>::FrontPosition() ); }

				MAPPOS EndPosition() const{ return MAPPOS( List<pair_type>::EndPosition() ); }

				HRESULT GetValue(MAPPOS vals, Value* ppItem){

						HRESULT hr = S_OK;

						pair_type pair;

						hr = List<pair_type>::GetItemPos(vals.pos, &pair);

						if(SUCCEEDED(hr)){
								*ppItem = pair.value;
						}

						return hr;
				}

				HRESULT GetKey(MAPPOS vals, Key* ppItem){

						HRESULT hr = S_OK;

						pair_type pair;

						hr = List<pair_type>::GetItemPos(vals.pos, &pair);

						if(SUCCEEDED(hr)){
								*ppItem = pair.key;
						}

						return hr;
				}

				MAPPOS Next(const MAPPOS vals){ return MAPPOS( List<pair_type>::Next( vals.pos ) ); }
};

#endif