//-------------------------------------------------------------------------
// Quad.h
//-------------------------------------------------------------------------
#ifndef QUAD_H
#define QUAD_H

#define D3DFVF_SCREENVERTEX  (D3DFVF_XYZRHW | D3DFVF_TEX1)

struct SCREENVERTEX{

	struct Position{

		Position() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {};
		Position(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_){};
		float x, y, z, w;
	};

	Position Pos;
	float Tu, Tv;
};

class CQuad{

public:

	CQuad() : m_QuadVB(NULL){}
	virtual ~CQuad(){ OnDelete(); }

	void OnCreate(const UINT32 uiWidth, const UINT32 uiHeight){

		m_QuadVertex[0].Pos = SCREENVERTEX::Position(0.0f, 0.0f, 0.0f, 1.0f);
		m_QuadVertex[1].Pos = SCREENVERTEX::Position(0.0f, (float)uiHeight, 0.0f, 1.0f);
		m_QuadVertex[2].Pos = SCREENVERTEX::Position((float)uiWidth, 0.0f, 0.0f, 1.0f);
		m_QuadVertex[3].Pos = SCREENVERTEX::Position((float)uiWidth, (float)uiHeight, 0.0f, 1.0f);

		m_QuadVertex[0].Tu = 0.0f; m_QuadVertex[0].Tv = 0.0f;
		m_QuadVertex[1].Tu = 0.0f; m_QuadVertex[1].Tv = 1.0f;
		m_QuadVertex[2].Tu = 1.0f; m_QuadVertex[2].Tv = 0.0f;
		m_QuadVertex[3].Tu = 1.0f; m_QuadVertex[3].Tv = 1.0f;
	}

	HRESULT OnRestore(IDirect3DDevice9* pDevice){

		HRESULT hr;
		OnDelete();

		IF_FAILED_RETURN(hr = (pDevice == NULL ? E_POINTER : S_OK));

		IF_FAILED_RETURN(pDevice->CreateVertexBuffer(sizeof(m_QuadVertex), D3DUSAGE_WRITEONLY, D3DFVF_SCREENVERTEX, D3DPOOL_DEFAULT, &m_QuadVB, NULL));

		void* pData;

		IF_FAILED_RETURN(m_QuadVB->Lock(0, sizeof(pData), &pData, 0));
		memcpy(pData, m_QuadVertex, sizeof(m_QuadVertex));
		IF_FAILED_RETURN(m_QuadVB->Unlock());

		return hr;
	}

	HRESULT OnRender(IDirect3DDevice9* pDevice){

		HRESULT hr;

		IF_FAILED_RETURN(hr = (pDevice == NULL ? E_POINTER : S_OK));

		if(m_QuadVB){

			IF_FAILED_RETURN(pDevice->SetStreamSource(0, m_QuadVB, 0, sizeof(SCREENVERTEX)));// Do it once...
			IF_FAILED_RETURN(pDevice->SetFVF(D3DFVF_SCREENVERTEX));// Do it once...
			IF_FAILED_RETURN(pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2));
		}

		return hr;
	}

	void OnDelete(){ SAFE_RELEASE(m_QuadVB); }

private:

	SCREENVERTEX m_QuadVertex[4];
	IDirect3DVertexBuffer9* m_QuadVB;
};

#endif