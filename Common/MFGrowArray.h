//----------------------------------------------------------------------------------------------
// MFGrowArray.h
//----------------------------------------------------------------------------------------------
#ifndef MFGROWARRAY_H
#define MFGROWARRAY_H

template <class T> class CGrowableArray{

public:

	CGrowableArray() : m_count(0), m_allocated(0), m_pArray(NULL){}
	virtual ~CGrowableArray(){ SAFE_DELETE_ARRAY(m_pArray); }

	HRESULT Allocate(DWORD alloc){

		HRESULT hr = S_OK;

		if(alloc > m_allocated){

			T* pTmp = new T[alloc];

			if(pTmp){

				ZeroMemory(pTmp, alloc * sizeof(T));

				assert(m_count <= m_allocated);

				for(DWORD i = 0; i < m_count; i++){
					pTmp[i] = m_pArray[i];
				}

				delete[] m_pArray;

				m_pArray = pTmp;
				m_allocated = alloc;
			}
			else{
				hr = E_OUTOFMEMORY;
			}
		}
		return hr;
	}

	HRESULT SetSize(DWORD count){

		assert(m_count <= m_allocated);

		HRESULT hr = S_OK;

		if(count > m_allocated){
			hr = Allocate(count);
		}

		if(SUCCEEDED(hr)){
			m_count = count;
		}

		return hr;
	}

	DWORD GetCount() const { return m_count; }

	T& operator[](DWORD index){

		assert(index < m_count);
		return m_pArray[index];
	}

	const T& operator[](DWORD index) const{

		assert(index < m_count);
		return m_pArray[index];
	}

	T* Ptr() { return m_pArray; }

protected:

	CGrowableArray & operator=(const CGrowableArray&);
	CGrowableArray(const CGrowableArray&);

	T* m_pArray;
	DWORD m_count;
	DWORD m_allocated;
};

#endif