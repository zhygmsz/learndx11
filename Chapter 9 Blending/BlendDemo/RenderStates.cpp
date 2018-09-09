//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState*	RenderStates::WireframeRS = 0;
ID3D11RasterizerState*	RenderStates::NoCullRS    = 0;
	 
ID3D11BlendState*		RenderStates::AlphaToCoverageBS		= 0;
ID3D11BlendState*		RenderStates::TransparentBS			= 0;
ID3D11BlendState*		RenderStates::DisableRedAndGreenBS	= 0;
ID3D11BlendState*		RenderStates::AdditiveBS			= 0;

ID3D11DepthStencilState* RenderStates::DisableDepthWriteDSS = 0;

void RenderStates::InitAll(ID3D11Device* device)
{
	//
	// WireframeRS
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));

	//
	// NoCullRS
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = {0};
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	//
	// TransparentBS
	//

	D3D11_BLEND_DESC transparentDesc = {0};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;
	transparentDesc.RenderTarget[0].BlendEnable = true;
	//������Щϵ������������ڻ��ϵ���ĺͻ�Ϸ�ʽ��
	transparentDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	D3D11_BLEND_DESC disableRedAndGreenDesc = {0};
	disableRedAndGreenDesc.AlphaToCoverageEnable = false;
	disableRedAndGreenDesc.IndependentBlendEnable = false;
	disableRedAndGreenDesc.RenderTarget[0].BlendEnable = true;
	disableRedAndGreenDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	disableRedAndGreenDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	disableRedAndGreenDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	disableRedAndGreenDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	disableRedAndGreenDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	disableRedAndGreenDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	disableRedAndGreenDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_BLUE | D3D11_COLOR_WRITE_ENABLE_ALPHA;

	HR(device->CreateBlendState(&disableRedAndGreenDesc, &DisableRedAndGreenBS));

	// AdditiveBS
	D3D11_BLEND_DESC additiveDesc = {0};
	//zhy �ʼ�
	//ֻ�иýṹ���Ĭ��ֵ�����Ϊ0����falseʱ���ſ���ʹ��{0}��ʼ����Ȼ���ֶ�ָ����Щ��0�ͷ�false��
	//���һ���ṹ��ķ�0���false�ֶα���ʼ��Ϊ��0���������ʱ����
	additiveDesc.AlphaToCoverageEnable = false;
	additiveDesc.IndependentBlendEnable = false;
	additiveDesc.RenderTarget[0].BlendEnable = true;
	//������Щϵ������������ڻ��ϵ���ĺͻ�Ϸ�ʽ��
	additiveDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
	additiveDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ZERO;
	additiveDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	additiveDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	additiveDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// DisableDepthWriteDSS
	D3D11_DEPTH_STENCIL_DESC disableDepthWriteDesc = {0};
	disableDepthWriteDesc.DepthEnable = true;
	disableDepthWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	disableDepthWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;

	/*
	disableDepthWriteDesc.StencilEnable = true;
	disableDepthWriteDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	disableDepthWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	disableDepthWriteDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
	disableDepthWriteDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	disableDepthWriteDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	disableDepthWriteDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
	disableDepthWriteDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
	disableDepthWriteDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
	*/

	HR(device->CreateDepthStencilState(&disableDepthWriteDesc, &DisableDepthWriteDSS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(DisableRedAndGreenBS);
}