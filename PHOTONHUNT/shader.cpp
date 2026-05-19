/*==============================================================================

   シェーダー [shader.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#include "shader.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include "debug_ostream.h"
#include <fstream>
#include "main.h"
#include <wrl.h>
#include <d3dcompiler.h>

Camera_3D WinCamera_3D;
Camera_3D WinLight_3D;
Camera WinCamera;
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11InputLayout> g_pInputLayout = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_pVSConstantBuffer = nullptr;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader = nullptr;

Microsoft::WRL::ComPtr<ID3D11Buffer> g_pVertexBuffer;

// 状态
Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState; // 光栅化状态
// 视口
std::optional<D3D11_VIEWPORT> viewport; // 使用可选视口

Microsoft::WRL::ComPtr<ID3D11Buffer> g_pConstantBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> g_pIndexBuffer;


/////////////////////////////////////////
Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader_uv;
Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader_uv;
Microsoft::WRL::ComPtr<ID3D11InputLayout> g_pInputLayout_uv;//输入布局
Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState_uv; // 光栅化状态
////////////////////////////////

bool Shader_Initialize(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext)
{
	HRESULT hr; // 用于保存 DirectX 函数返回值

	CreateVertexBuffer(pDevice);
	CreateIndexBuffer(pDevice);
	CompileAndUploadShader(pDevice);

	CreateConstantBuffer(pDevice, pContext);

	return true;
}


void Shader_Finalize()
{
	SAFE_RELEASE(g_pPixelShader);
	SAFE_RELEASE(g_pVSConstantBuffer);
	SAFE_RELEASE(g_pInputLayout);
	SAFE_RELEASE(g_pVertexShader);
}



// 光栅化状态
bool CreateRasterizerState(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext
	)
{
	D3D11_RASTERIZER_DESC rsDesc = {};

	rsDesc.FillMode = D3D11_FILL_SOLID;             // 设为实心填充
	// FillMode: 决定三角形图元的填充方式。
	// D3D11_FILL_WIREFRAME: 线框模式，只绘制图元的边框。
	// D3D11_FILL_SOLID: 实心填充模式，绘制完整的三角形（默认）。

	rsDesc.CullMode = D3D11_CULL_NONE;              // 剔除背面
	// CullMode: 决定如何剔除图元的背面。
	// D3D11_CULL_BACK: 剔除背面（默认），不渲染背面图元。
	// D3D11_CULL_FRONT: 剔除正面，只渲染背面。
	// D3D11_CULL_NONE: 不进行剔除，渲染正面和背面。

	rsDesc.FrontCounterClockwise = FALSE;           // 顺时针顶点为正面
	// FrontCounterClockwise: 指定三角形顶点的顺序来定义正面方向。
	// TRUE: 逆时针顶点顺序为正面。
	// FALSE: 顺时针顶点顺序为正面（默认）。

	rsDesc.DepthBias = 0;                           // 没有恒定的深度偏移
	// DepthBias: 在渲染深度缓冲区时应用一个恒定的深度偏移，以防止 z-fighting。
	// 取整数值，通常用于调整深度值，避免图元深度过于接近导致深度冲突。

	rsDesc.DepthBiasClamp = 0.0f;                   // 不限制深度偏移
	// DepthBiasClamp: 深度偏移的最大值，确保 DepthBias 不会过大。
	// 取浮点值，设定深度偏移的最大允许值。

	rsDesc.SlopeScaledDepthBias = 0.0f;             // 没有斜率缩放偏移
	// SlopeScaledDepthBias: 基于图元的斜率对深度进行缩放偏移。
	// 取浮点值，基于图元斜率的深度偏移量，通常用于避免锯齿现象。

	rsDesc.DepthClipEnable = TRUE;                  // 禁用深度裁剪
	// DepthClipEnable: 启用或禁用深度裁剪。
	// TRUE: 启用深度裁剪，在深度缓冲区范围外的图元将被裁剪。
	// FALSE: 禁用深度裁剪（默认），所有图元都会被绘制，不论其深度。

	rsDesc.ScissorEnable = FALSE;                   // 禁用剪刀矩形
	// ScissorEnable: 启用或禁用剪刀矩形测试。
	// TRUE: 启用剪刀矩形测试，仅绘制在剪刀矩形内的部分。
	// FALSE: 禁用剪刀矩形测试（默认），不限制绘制区域。

	rsDesc.MultisampleEnable = FALSE;               // 禁用多重采样
	// MultisampleEnable: 启用或禁用多重采样抗锯齿。
	// TRUE: 启用多重采样抗锯齿，用于提升渲染质量。
	// FALSE: 禁用多重采样（默认）。

	rsDesc.AntialiasedLineEnable = FALSE;           // 禁用线条抗锯齿
	// AntialiasedLineEnable: 启用或禁用线条抗锯齿。
	// TRUE: 启用线条抗锯齿，但只有在 FillMode 为 D3D11_FILL_WIREFRAME 时有效。
	// FALSE: 禁用线条抗锯齿（默认）。



	//D3D11_RASTERIZER_DESC rsDesc = {};
	//rsDesc.CullMode = D3D11_CULL_BACK;  // 禁用剔除
	//rsDesc.FillMode = D3D11_FILL_SOLID;
	//CullMode可以设置为以下几种：

	//	D3D11_CULL_BACK：剔除背面。
	//	D3D11_CULL_FRONT：剔除正面。
	//	D3D11_CULL_NONE：不进行剔除（默认）。
	if (FAILED(pDevice->CreateRasterizerState(&rsDesc, &pRasterizerState))) {
		MessageBox(NULL, L"Failed to create rasterizer state.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	pContext->RSSetState(pRasterizerState.Get());

	rsDesc.FillMode = D3D11_FILL_WIREFRAME;
	if (FAILED(pDevice->CreateRasterizerState(&rsDesc, &pRasterizerState_uv))) {
		MessageBox(NULL, L"Failed to create rasterizer state.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	pContext->RSSetState(pRasterizerState_uv.Get());
}

// 使用可选视口
bool CreateViewport(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext,
	int width, int height)
{

	if (!viewport.has_value()) {
		viewport = D3D11_VIEWPORT();
	}

	viewport->Width = static_cast<float>(width);
	viewport->Height = static_cast<float>(height);
	viewport->MinDepth = 0.0f;
	viewport->MaxDepth = 1.0f;
	viewport->TopLeftX = 0.0f;
	viewport->TopLeftY = 0.0f;

	pContext->RSSetViewports(1u, &(*viewport));
	return true;
}

//顶点上传
bool CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice)
{
	std::vector<Vector> point;
	Vector v;
	v.color = { 1,1,1,1 };
	v.hasTexture = 1.0f;
	v.normal = { 0,0, -1 };

	v.texcoord = { 0,0 };
	v.position = { -0.5,-0.5,0,0 };
	point.push_back(v);

	v.texcoord = { 1,0 };
	v.position = { 0.5,-0.5,0,0 };
	point.push_back(v);

	v.texcoord = { 1,1 };
	v.position = { -0.5,0.5,0,0 };
	point.push_back(v);

	v.texcoord = { 0,1 };
	v.position = { 0.5,0.5,0,0 };
	point.push_back(v);


	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(4 * sizeof(Vector));
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
	vertexDataDesc.pSysMem = point.data();
	if (FAILED(pDevice->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, &g_pVertexBuffer))) {
		MessageBox(NULL, L"Failed to create vertex buffer.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}


	return true;
}

//索引上传
bool CreateIndexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice)
{
	D3D11_BUFFER_DESC indexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA indexData = {};

	std::vector<uint16_t> indices_16;
	indices_16.push_back(0);
	indices_16.push_back(1);
	indices_16.push_back(3);
	indices_16.push_back(0);
	indices_16.push_back(3);
	indices_16.push_back(2);

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0u;
	indexBufferDesc.ByteWidth = 6 * sizeof(uint16_t);
	indexBufferDesc.StructureByteStride = sizeof(uint16_t);

	indexData.pSysMem = indices_16.data();

	if (FAILED(pDevice->CreateBuffer(&indexBufferDesc, &indexData, &g_pIndexBuffer))) {
		MessageBox(NULL, L"Failed to create index buffer.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

// Shader 和 输入布局
bool CompileAndUploadShader(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice)
{
	// 添加着色器shader
	Microsoft::WRL::ComPtr<ID3DBlob> pPixelBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pVertexBlob;

	// 编译像素着色器
	std::wstring p = L"shader_pixel_2d.cso";
	if (FAILED(D3DReadFileToBlob(p.c_str(), &pPixelBlob))) {
		MessageBox(NULL, L"Failed to load pixel shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	if (FAILED(pDevice->CreatePixelShader(pPixelBlob->GetBufferPointer(), pPixelBlob->GetBufferSize(), nullptr, &g_pPixelShader))) {
		MessageBox(NULL, L"Failed to create pixel shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	p = L"shader_vertex_2d.cso";
	// 编译顶点着色器
	if (FAILED(D3DReadFileToBlob(p.c_str(), &pVertexBlob))) {
		MessageBox(NULL, L"Failed to load vertex shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	if (FAILED(pDevice->CreateVertexShader(pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize(), nullptr, &g_pVertexShader))) {
		MessageBox(NULL, L"Failed to create vertex shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// 定义输入布局
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vector, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vector, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vector, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vector, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,          0, offsetof(Vector, hasTexture), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 使用顶点着色器的字节码创建输入布局
	if (FAILED(pDevice->CreateInputLayout(
		layout,
		UINT(std::size(layout)),
		pVertexBlob->GetBufferPointer(),
		pVertexBlob->GetBufferSize(),
		&g_pInputLayout)))
	{
		MessageBox(NULL, L"Failed to create input layout.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}


	{
		// 添加着色器shader
		Microsoft::WRL::ComPtr<ID3DBlob> pPixelBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> pVertexBlob;

		// 编译像素着色器
		std::wstring p = L"shader_pixel_2d_uv.cso";
		if (FAILED(D3DReadFileToBlob(p.c_str(), &pPixelBlob))) {
			MessageBox(NULL, L"Failed to load pixel shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		if (FAILED(pDevice->CreatePixelShader(pPixelBlob->GetBufferPointer(), pPixelBlob->GetBufferSize(), nullptr, &g_pPixelShader_uv))) {
			MessageBox(NULL, L"Failed to create pixel shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		p = L"shader_vertex_2d_uv.cso";
		// 编译顶点着色器
		if (FAILED(D3DReadFileToBlob(p.c_str(), &pVertexBlob))) {
			MessageBox(NULL, L"Failed to load vertex shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		if (FAILED(pDevice->CreateVertexShader(pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize(), nullptr, &g_pVertexShader_uv))) {
			MessageBox(NULL, L"Failed to create vertex shader.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}

		// 定义输入布局
		//D3D11_INPUT_ELEMENT_DESC layout[] =
		//{
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//};


		// 定义输入布局
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vector, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vector, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, offsetof(Vector, texcoord), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vector, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT,          0, offsetof(Vector, hasTexture), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		// 使用顶点着色器的字节码创建输入布局
		if (FAILED(pDevice->CreateInputLayout(
			layout,
			UINT(std::size(layout)),
			pVertexBlob->GetBufferPointer(),
			pVertexBlob->GetBufferSize(),
			&g_pInputLayout_uv)))
		{
			MessageBox(NULL, L"Failed to create input layout.", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
	}
	//MessageBox(NULL, L"Shaders and Input Layout created successfully.", L"Success", MB_OK | MB_ICONINFORMATION);
	return true;
}

// ==== 追加：常量バッファ生成 ====
struct ConstantBuffer
{
	DirectX::XMMATRIX mtx;
	DirectX::XMFLOAT2 screenSize;
	DirectX::XMFLOAT2 padding;
	DirectX::XMFLOAT2 pos;
	DirectX::XMFLOAT2 Rotation_Scale;
};

//常量定义
bool CreateConstantBuffer(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext)
{


	D3D11_BUFFER_DESC cbd = {};
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cbInitData = {};
	ConstantBuffer cb = {};
	cb.mtx = DirectX::XMMatrixIdentity();
	cb.screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
	cb.pos = { WinCamera.pos.x,WinCamera.pos.y };
	cb.Rotation_Scale = { WinCamera.Rotation,WinCamera.Scale };

	cbInitData.pSysMem = &cb;

	pDevice->CreateBuffer(&cbd, &cbInitData, &g_pConstantBuffer);

	return true;
}

void ResetConstantBuffer(ID3D11DeviceContext* context)
{
	D3D11_MAPPED_SUBRESOURCE mapped;
	context->Map(g_pConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	ConstantBuffer* cb = reinterpret_cast<ConstantBuffer*>(mapped.pData);

	// ======== 复原所有参数 ========
	cb->mtx = DirectX::XMMatrixIdentity();
	cb->screenSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
	cb->padding = { 0,0 };

	// 相机参数归零/默认
	cb->pos = { WinCamera.pos.x, WinCamera.pos.y };
	cb->Rotation_Scale = { WinCamera.Rotation, WinCamera.Scale };    // rotation=0°, scale=1

	context->Unmap(g_pConstantBuffer.Get(), 0);
}


