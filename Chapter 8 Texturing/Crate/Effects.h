//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);

protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	void SetWorldViewProj(CXMMATRIX M)						{ WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M)								{ World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M)					{ WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M)						{ TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	//void SetEyePosW(const XMFLOAT3& v)					{ EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEyePosW(const XMFLOAT3& v)						{ EyePosW->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetDirLights(const DirectionalLight* lights)		{ DirLights->SetRawValue(lights, 0, 3*sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat)					{ Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	void SetDiffuseMap(ID3D11ShaderResourceView* tex)		{ DiffuseMap->SetResource(tex); }

	void SetFlareMap(ID3D11ShaderResourceView* tex)			{ FlareMap->SetResource(tex); }
	void SetFlareAlphaMap(ID3D11ShaderResourceView* tex)	{ FlareAlphaMap->SetResource(tex); }

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectTechnique* Light1TexTechFireAni;
	ID3DX11EffectTechnique* Light2TexTechFireAni;
	ID3DX11EffectTechnique* Light3TexTechFireAni;

	ID3DX11EffectTechnique* Light1TexTechFireBall;
	ID3DX11EffectTechnique* Light2TexTechFireBall;
	ID3DX11EffectTechnique* Light3TexTechFireBall;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;

	ID3DX11EffectShaderResourceVariable* FlareMap;
	ID3DX11EffectShaderResourceVariable* FlareAlphaMap;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
};
#pragma endregion

#endif // EFFECTS_H