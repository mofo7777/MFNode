//----------------------------------------------------------------------------------------------
// SinkDxva2Renderer_Settings.cpp
// Copyright (C) 2014 Dumonteil David
//
// MFNode is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MFNode is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT CSinkDxva2Renderer::SetBrightness(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetBrightness"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_BRIGHTNESS, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetBrightness(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetBrightness"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_BRIGHTNESS, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetContrast(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetContrast"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_CONTRAST, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetContrast(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetContrast"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_CONTRAST, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetHue(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetHue"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_HUE, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetHue(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetHue"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_HUE, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetSaturation(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetSaturation"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_SATURATION, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetSaturation(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetSaturation"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_SATURATION, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetNoiseReduction(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetNoiseReduction"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_NOISE_REDUCTION, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetNoiseReduction(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetNoiseReduction"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_NOISE_REDUCTION, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetEdgeEnhancement(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetEdgeEnhancement"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_EDGE_ENHANCEMENT, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetEdgeEnhancement(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetEdgeEnhancement"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_EDGE_ENHANCEMENT, bSupported, pDefault, pMin, pMax));

	return hr;
}

HRESULT CSinkDxva2Renderer::SetAnamorphicScaling(BOOL bEnable, INT iValue){

	TRACE_SINK((L"SinkRenderer::SetAnamorphicScaling"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.SetProcAmp(DXVAHD_FILTER_ANAMORPHIC_SCALING, bEnable, iValue));

	return hr;
}

HRESULT CSinkDxva2Renderer::GetAnamorphicScaling(BOOL* bSupported, INT* pDefault, INT* pMin, INT* pMax){

	TRACE_SINK((L"SinkRenderer::GetAnamorphicScaling"));

	HRESULT hr;

	AutoLock lock(m_CriticSection);

	IF_FAILED_RETURN(hr = CheckShutdown());

	IF_FAILED_RETURN(hr = m_cDxva2Manager.GetProcAmp(DXVAHD_FILTER_ANAMORPHIC_SCALING, bSupported, pDefault, pMin, pMax));

	return hr;
}