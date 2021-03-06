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
	Draw_Non			= 0,	//啥也不画
	Draw_FireAni		= 1,	//火焰动画
	Draw_FireBall		= 2		//火球旋转
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

	//FireBall纹理变换矩阵
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

	//设置纹理坐标缩放矩阵，配合重复模式
	XMMATRIX texScale = XMMatrixScaling(1.0f, 1.0f, 0.0f);
	XMStoreFloat4x4(&mTexTransFireBall, texScale);

	//zhy 笔记
	//把纹理作为材质时，将环境光和漫反射光调制到纹理颜色上。
	//灯光的某个属性的分量值要一样，但不同的属性值的分量值可以不同
	//只有这样被调制后的纹理的颜色才不会和原色有差别，仅仅是改变了明暗效果。
	//如果把某个属性的分量值改为不一样的，那最终看到的纹理效果和原色有差别。
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

	//zhy 笔记
	//材质只是用于和光照相互作用，目的是调和光照属性的分量值
	//相当于光照在照射到物体上之前，多了一层叠加（或过滤）计算，使得光照效果有了更大和更广的调节范围。
	mBoxMat.Ambient  = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	//zhy 笔记
	//立方体盒子本身的位置时固定的，灯光是平行光，那么立方体能受到的光照区域也是固定的。
	//在演示程序里，看似整个场景在一个球面上来回旋转，其实是在改变摄像机的位置，而摄像机的观察方向不改变。
	//摄像机的方向不同，则看到的画面不一样，看似是立方体在旋转，其实不然。

	//构造方法里只初始化数据.
}

CrateApp::~CrateApp()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	//真正的纹理资源在mFireAniMapSRV数组里，所以注释掉该句
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

	//Init方法里做初始化资源的工作.

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

	//FireBall的纹理旋转一直持续
	//都是结构体，方法返回，结构体占用内存被栈回收，最终结果保存在成员变量mTexTransFireBall里
	//方法每帧执行，但内存并不受影响
	sAngle += dt * 1;
	XMMATRIX flareTrans = XMMatrixTranslation(0.5f, 0.5f, 0.0f);
	XMMATRIX flareTransInv = XMMatrixTranslation(-0.5f, -0.5f, 0.0f);
	XMMATRIX flareRotation = XMMatrixRotationZ(sAngle);
	XMStoreFloat4x4(&mTexTransFireBall, flareTransInv * flareRotation * flareTrans);

	//zhy 笔记
	//在这个旋转火球的例子中，最初一直很迷惑，虽然知道是平移+旋转+平移的套路，但不得其法。
	//在这其中，忽略了两个前提，这两个前提决定了渲染到屏幕上具体内容的细节
	//第一个是纹理的寻址模式，最初设置的是WRAP，没有意识到这个问题的时候，发现把纹理坐标做了平移后，正方形的其他区域还有火球的其他轮廓
	//第二个是顶点坐标和纹理坐标的对应关系，毕竟立方体有6个面，而我却以为每个面的左上角都是对应纹理坐标的原点。这才使得，无论怎么捋都捋不顺逻辑。
	//在确定了寻址模式和顶点坐标与纹理坐标对应关系后，把立方体的创建细节里只剩余一个面，确定了该面的左上角对应纹理坐标原点。
	//平移矩阵和旋转矩阵都是在操作纹理坐标，比如flareTransInv平移相当于把纹理坐标都统一减去了0.5
	//正方形左上角现在的纹理坐标为-0.5，而原来的中心点纹理坐标变成了纹理坐标原点。
	//操作纹理坐标和操作纹理坐标系是一个逆关系，flareTransInv平移矩阵相当于是把原来正方形左上角的纹理坐标系原点挪到了正方形的中心。
	//而flareRotation旋转矩阵始终是沿着纹理坐标系的Z轴（也就是纹理坐标系原点）旋转。
	//所以之前的思路（想着怎么把纹理的中心点挪到正方形的左上角）是错误的。正确的思路应该是把旋转轴从正方形的左上角挪到中心点。
	//现在旋转中心轴位于正方形的中心，旋转后，再把纹理平移回去即可。
	//其实这些操作纹理坐标的变换矩阵，都可以在VS里实现。

	//以上是第一次弄明白该纹理旋转时总结的的笔记，现在读起来已经不那么顺畅，而且前后逻辑也难捋顺，下面是最新理解笔记

	//纹理贴在立方体的一个面上，可以把纹理单独理解成一个空间，纹理空间。其原点在纹理贴图的左上角，水平向右为X轴，垂直向下为Y轴，还有一个Z轴，垂直于XY平面且朝向屏幕外/内
	//而此时该纹理的坐标也正是在纹理空间的原点上，绕着Z轴旋转的话，也是以左上角的点为旋转点，要通过变换，最后实现纹理绕着其中心点，也就是纹理空间的(0.5,0.5,Z)轴旋转
	//为了达到效果，需要将纹理的中心点挪到纹理空间的原点，渲染了以后，再反向平移回去，即可实现绕着纹理的中心点旋转。
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
			//不使用纹理采样
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
 
