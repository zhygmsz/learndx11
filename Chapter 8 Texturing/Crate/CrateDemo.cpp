//***************************************************************************************
// CrateDemo.cpp by Frank Luna (C) 2011 All Rights Reserved.
//
// Demonstrates texturing a box.
//
// Controls:
//		Hold the left mouse button down and move the mouse to rotate.
//      Hold the right mouse button down to zoom in and out.
//
//***************************************************************************************

#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"

enum DrawType
{
	Draw_Non			= 0,	//ɶҲ����
	Draw_FireAni		= 1,	//���涯��
	Draw_FireBall		= 2		//������ת
};

class CrateApp : public D3DApp
{
public:
	CrateApp(HINSTANCE hInstance);
	~CrateApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene(); 

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	ID3D11ShaderResourceView* GetNextFrame();

private:
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;
	ID3D11ShaderResourceView** mFireAniMapSRV;

	ID3D11ShaderResourceView* mFlareMapSRV;
	ID3D11ShaderResourceView* mFlareAlphaMapSRV;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	//FireBall����任����
	XMFLOAT4X4 mTexTransFireBall;
	XMFLOAT4X4 mTexTransFireAni;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	const int FireAniMaxFrame;
	int mCurFrame;
	float mfTimer;
	const float FramePerSec;

	DrawType mDrawType;

	static float sAngle;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	CrateApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;
	
	return theApp.Run();
}
 
float CrateApp::sAngle = 0.0f;

CrateApp::CrateApp(HINSTANCE hInstance)
: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f), 
  mTheta(1.3f*MathHelper::Pi), mPhi(0.4f*MathHelper::Pi), mRadius(2.5f), mCurFrame(-1), mfTimer(0.0f), FireAniMaxFrame(120), FramePerSec(30.0f),
  mDrawType(Draw_Non), mFlareMapSRV(0), mFlareAlphaMapSRV(0)
{
	mMainWndCaption = L"Crate Demo";
	
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransFireBall, I);
	XMStoreFloat4x4(&mTexTransFireAni, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	//���������������ž�������ظ�ģʽ
	XMMATRIX texScale = XMMatrixScaling(1.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mTexTransFireBall, texScale);

	//zhy �ʼ�
	//��������Ϊ����ʱ��������������������Ƶ�������ɫ�ϡ�
	//�ƹ��ĳ�����Եķ���ֵҪһ��������ͬ������ֵ�ķ���ֵ���Բ�ͬ
	//ֻ�����������ƺ���������ɫ�Ų����ԭɫ�в�𣬽����Ǹı�������Ч����
	//�����ĳ�����Եķ���ֵ��Ϊ��һ���ģ������տ���������Ч����ԭɫ�в��
	mDirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(1.0f, 1.0f, 0.0f);
 
	mDirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);
	//mDirLights[1].Direction = XMFLOAT3(-1.0f, -1.0f, -1.0f);

	//zhy �ʼ�
	//����ֻ�����ں͹����໥���ã�Ŀ���ǵ��͹������Եķ���ֵ
	//�൱�ڹ��������䵽������֮ǰ������һ����ӣ�����ˣ����㣬ʹ�ù���Ч�����˸���͸���ĵ��ڷ�Χ��
	mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	//zhy �ʼ�
	//��������ӱ����λ��ʱ�̶��ģ��ƹ���ƽ�й⣬��ô���������ܵ��Ĺ�������Ҳ�ǹ̶��ġ�
	//����ʾ�������������������һ��������������ת����ʵ���ڸı��������λ�ã���������Ĺ۲췽�򲻸ı䡣
	//������ķ���ͬ���򿴵��Ļ��治һ��������������������ת����ʵ��Ȼ��

	//���췽����ֻ��ʼ������.
}

CrateApp::~CrateApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	//������������Դ��mFireAniMapSRV���������ע�͵��þ�
	//ReleaseCOM(mDiffuseMapSRV);

	for (int idx = 0; idx < FireAniMaxFrame; ++idx)
	{
		ReleaseCOM(mFireAniMapSRV[idx]);
	}

	ReleaseCOM(mFlareMapSRV);
	ReleaseCOM(mFlareAlphaMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool CrateApp::Init()
{
	if(!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	//Init����������ʼ����Դ�Ĺ���.

	mFireAniMapSRV = new ID3D11ShaderResourceView*[FireAniMaxFrame];
	std::wstring fileName = L"";
	for (int idx = 0; idx < FireAniMaxFrame; ++idx)
	{
		fileName = Str2Wstr(format("../FireAnim/Fire%03d.bmp", idx + 1));
		HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, 
			fileName.c_str(), 0, 0, &mFireAniMapSRV[idx], 0 ));
	}

	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"../flare.dds", 0, 0, &mFlareMapSRV, 0));
	HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,
		L"../flarealpha.dds", 0, 0, &mFlareAlphaMapSRV, 0));

	mDiffuseMapSRV = GetNextFrame();

	BuildGeometryBuffers();

	return true;
}

ID3D11ShaderResourceView* CrateApp::GetNextFrame()
{
	++mCurFrame;
	if (mCurFrame >= FireAniMaxFrame)
	{
		mCurFrame = 0;
	}
	return mFireAniMapSRV[mCurFrame];
}

void CrateApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CrateApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos    = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	mfTimer += dt;
	if (mfTimer >= (1 / FramePerSec))
	{
		mfTimer = 0.0f;
		mDiffuseMapSRV = GetNextFrame();
	}

	if (GetAsyncKeyState('0') & 0x8000)
	{
		mDrawType = Draw_Non;
	}
	if (GetAsyncKeyState('1') & 0x8000)
	{
		mDrawType = Draw_FireAni;
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		mDrawType = Draw_FireBall;
	}

	//FireBall��������תһֱ����
	//���ǽṹ�壬�������أ��ṹ��ռ���ڴ汻ջ���գ����ս�������ڳ�Ա����mTexTransFireBall��
	//����ÿִ֡�У����ڴ沢����Ӱ��
	sAngle += dt * 1;
	XMMATRIX flareTrans = XMMatrixTranslation(0.5f, 0.5f, 0.0f);
	XMMATRIX flareTransInv = XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
	XMMATRIX flareRotation = XMMatrixRotationZ(sAngle);
	XMStoreFloat4x4(&mTexTransFireBall, flareTransInv * flareRotation * flareTrans);

	//zhy �ʼ�
	//�������ת����������У����һֱ���Ի���Ȼ֪����ƽ��+��ת+ƽ�Ƶ���·���������䷨��
	//�������У�����������ǰ�ᣬ������ǰ���������Ⱦ����Ļ�Ͼ������ݵ�ϸ��
	//��һ���������Ѱַģʽ��������õ���WRAP��û����ʶ����������ʱ�򣬷��ְ�������������ƽ�ƺ������ε����������л������������
	//�ڶ����Ƕ����������������Ķ�Ӧ��ϵ���Ͼ���������6���棬����ȴ��Ϊÿ��������ϽǶ��Ƕ�Ӧ���������ԭ�㡣���ʹ�ã�������ô�۶��۲�˳�߼���
	//��ȷ����Ѱַģʽ�Ͷ������������������Ӧ��ϵ�󣬰�������Ĵ���ϸ����ֻʣ��һ���棬ȷ���˸�������ϽǶ�Ӧ��������ԭ�㡣
	//ƽ�ƾ������ת�������ڲ����������꣬����flareTransInvƽ���൱�ڰ��������궼ͳһ��ȥ��0.5
	//���������Ͻ����ڵ���������Ϊ-0.5����ԭ�������ĵ���������������������ԭ�㡣
	//������������Ͳ�����������ϵ��һ�����ϵ��flareTransInvƽ�ƾ����൱���ǰ�ԭ�����������Ͻǵ���������ϵԭ��Ų���������ε����ġ�
	//��flareRotation��ת����ʼ����������������ϵ��Z�ᣨҲ������������ϵԭ�㣩��ת��
	//����֮ǰ��˼·��������ô����������ĵ�Ų�������ε����Ͻǣ��Ǵ���ġ���ȷ��˼·Ӧ���ǰ���ת��������ε����Ͻ�Ų�����ĵ㡣
	//������ת������λ�������ε����ģ���ת���ٰ�����ƽ�ƻ�ȥ���ɡ�
	//��ʵ��Щ������������ı任���󣬶�������VS��ʵ�֡�

	//�����ǵ�һ��Ū���׸�������תʱ�ܽ�ĵıʼǣ����ڶ������Ѿ�����ô˳��������ǰ���߼�Ҳ����˳���������������ʼ�

	//���������������һ�����ϣ����԰�����������һ���ռ䣬����ռ䡣��ԭ����������ͼ�����Ͻǣ�ˮƽ����ΪX�ᣬ��ֱ����ΪY�ᣬ����һ��Z�ᣬ��ֱ��XYƽ���ҳ�����Ļ��/��
	//����ʱ�����������Ҳ����������ռ��ԭ���ϣ�����Z����ת�Ļ���Ҳ�������Ͻǵĵ�Ϊ��ת�㣬Ҫͨ���任�����ʵ���������������ĵ㣬Ҳ��������ռ��(0.5,0.5,Z)����ת
	//Ϊ�˴ﵽЧ������Ҫ����������ĵ�Ų������ռ��ԭ�㣬��Ⱦ���Ժ��ٷ���ƽ�ƻ�ȥ������ʵ��������������ĵ���ת��
}

void CrateApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
 
	UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
 
	XMMATRIX view  = XMLoadFloat4x4(&mView);
	XMMATRIX proj  = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
 
	ID3DX11EffectTechnique* activeTech = NULL;
	if (mDrawType == Draw_FireAni)
	{
		activeTech = Effects::BasicFX->Light2TexTechFireAni;
	}
	else if (mDrawType == Draw_FireBall)
	{
		activeTech = Effects::BasicFX->Light2TexTechFireBall;
	}
	else
	{
		activeTech = Effects::BasicFX->Light2Tech;
	}

    D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
		// Draw the box.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetMaterial(mBoxMat);

		if (mDrawType == Draw_FireAni)
		{
			Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransFireAni));
			Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);
		}
		else if (mDrawType == Draw_FireBall)
		{
			Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransFireBall));
			Effects::BasicFX->SetFlareMap(mFlareMapSRV);
			Effects::BasicFX->SetFlareAlphaMap(mFlareAlphaMapSRV);
		}
		else
		{
			//��ʹ���������
		}

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
    }

	HR(mSwapChain->Present(0, 0));
}

void CrateApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CrateApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CrateApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if( (btnState & MK_LBUTTON) != 0 )
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi   += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi-0.1f);
	}
	else if( (btnState & MK_RBUTTON) != 0 )
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CrateApp::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset      = 0;

	// Cache the index count of each object.
	mBoxIndexCount      = box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset      = 0;
	
	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = mBoxIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for(size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos    = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex    = box.Vertices[i].TexC;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}
 
