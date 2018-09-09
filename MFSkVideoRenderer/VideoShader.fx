//----------------------------------------------------------------------------------------------
// VideoShader.fx
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
float4x4 g_mWorldViewProjection;

bool g_bReverseX = false;
bool g_bReverseY = false;
bool g_bNegatif = false;
bool g_bGray = false;
bool g_bGrayScale = false;
int g_iColorMode = 0;
float g_fContrast = 1.0f;
float g_fSaturation = 1.0f;

texture g_TextureY;
texture g_TextureU;
texture g_TextureV;

sampler SamplerY = sampler_state{

		Texture = <g_TextureY>;
};

sampler SamplerU = sampler_state{

		Texture = <g_TextureU>;
};

sampler SamplerV = sampler_state{

		Texture = <g_TextureV>;
};

struct VS_OUTPUT{

		float4 Position  : POSITION;
  float2 TextureUV : TEXCOORD0;
};

VS_OUTPUT RenderVideoVS(float4 vPos : POSITION, float2 vTexCoord0 : TEXCOORD0){

		VS_OUTPUT Output;

		Output.Position = mul(vPos, g_mWorldViewProjection);
		Output.TextureUV = vTexCoord0;

		return Output;    
}

float4 RenderVideoPS(VS_OUTPUT In) : COLOR{

		float4 color;

		if(g_bReverseX){
				In.TextureUV.x = 1.0f - In.TextureUV.x;
		}

		if(g_bReverseY){
		  In.TextureUV.y = 1.0f - In.TextureUV.y;
		}

		float3 YUV = float3(tex2D(SamplerY, In.TextureUV).x - (16.0f / 256.0f),
				tex2D(SamplerU, In.TextureUV).x - (128.0f / 256.0f),
				tex2D(SamplerV, In.TextureUV).x - (128.0f / 256.0f));

		color.r = clamp((1.164f * YUV.x + 1.596f * YUV.y), 0.0f, 255.0f);
		color.g = clamp((1.164f * YUV.x - 0.813f * YUV.z - 0.391f * YUV.y), 0.0f, 255.0f);
		color.b = clamp((1.164f * YUV.x + 2.018f * YUV.z), 0.0f, 255.0f);
		color.a = 1.0f;

		if(g_bNegatif){
				color.rgb = 1.0f - color.rgb;
		}

		if(g_iColorMode){

				if(g_iColorMode == 1)
						color.rbga = color;
				else if(g_iColorMode == 2)
						color.gbra = color;
				else
						color.bgra = color;
		}

		if(g_bGray){
				color = (color.r + color.g + color.b) / 3.0f;
		}

		if(g_bGrayScale){
		  color.rgb = dot(color.rgb, float3(0.3f, 0.59f, 0.11f));
		}

		color = pow(color, g_fContrast);

		float fGrey = dot(color, float3(0.3f, 0.59f, 0.11f));
  color.rgb = lerp(fGrey, color, g_fSaturation).rgb;

		return color;
}

technique RenderVideo{

		pass P0{

				VertexShader = compile vs_2_0 RenderVideoVS();
    PixelShader  = compile ps_2_0 RenderVideoPS();
  }
}