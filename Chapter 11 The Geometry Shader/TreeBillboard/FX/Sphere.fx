//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldViewProj;
	Material gMaterial;
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL		: POSITION;
	float3 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
};

struct VertexOut
{
	float3 PosL		: POSITION;
	float3 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
};

struct GeoOut
{
	float4 PosH		: SV_POSITION;
	float3 PosW		: POSITION;
	float3 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
	uint   PrimID	: SV_PrimitiveID;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosL = vin.PosL;
	vout.Normal = vin.Normal;
	vout.Tex = vin.Tex;

	return vout;
}

[maxvertexcount(24)]
void GS(triangle VertexOut gin[3], inout TriangleStream<GeoOut> triStream)
{
	VertexOut m[3];
	m[0].PosL = 0.5f * (gin[0].PosL + gin[1].PosL);
	m[1].PosL = 0.5f * (gin[1].PosL + gin[2].PosL);
	m[2].PosL = 0.5f * (gin[2].PosL + gin[0].PosL);

	
	m[0].PosL = normalize(m[0].PosL);
	m[1].PosL = normalize(m[1].PosL);
	m[2].PosL = normalize(m[2].PosL);
	

	m[0].Normal = 0.5f * (gin[0].Normal + gin[1].Normal);
	m[1].Normal = 0.5f * (gin[1].Normal + gin[2].Normal);
	m[2].Normal = 0.5f * (gin[2].Normal + gin[0].Normal);

	m[0].Tex = 0.5f * (gin[0].Tex + gin[1].Tex);
	m[1].Tex = 0.5f * (gin[1].Tex + gin[2].Tex);
	m[2].Tex = 0.5f * (gin[2].Tex + gin[0].Tex);

	VertexOut outVerts[6];
    outVerts[0] = gin[0];
    outVerts[1] = m[0];
    outVerts[2] = m[2];
    outVerts[3] = m[1];
    outVerts[4] = gin[2];
    outVerts[5] = gin[1];

	GeoOut gout[6];
	[unroll]
	for (int i = 0; i < 6; ++i)
	{
		gout[i].PosW = mul(float4(outVerts[i].PosL, 1.0f), gWorld).xyz;
		gout[i].PosH = mul(float4(outVerts[i].PosL, 1.0f), gWorldViewProj);
		gout[i].Normal = outVerts[i].Normal;
		gout[i].Tex = outVerts[i].Tex;
	}

	[unroll]
	for (int j = 0; j < 5; ++j)
	{
		triStream.Append(gout[j]);
	}
	triStream.RestartStrip();
	triStream.Append(gout[1]);
    triStream.Append(gout[5]);
    triStream.Append(gout[3]);
}
 
float4 PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
	float4 litColor = float4(1, 1, 1, 1);

    return litColor;
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
		//SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false) ) );
    }
}

technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true) ) );
    }
}