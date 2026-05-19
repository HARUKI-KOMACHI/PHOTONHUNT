#include "Utility.h"

static Microsoft::WRL::ComPtr<ID3DBlob> Blob;

void LoadVS(Microsoft::WRL::ComPtr<ID3D11VertexShader>& pVertexShader, const std::wstring& name)
{
	// 1. 读取 cso
	if (FAILED(D3DReadFileToBlob(name.c_str(), &Blob)))
	{
		std::wstring msg = L"Failed to load vertex shader file:\n" + name;
		MessageBox(NULL, msg.c_str(), L"Shader Error", MB_OK | MB_ICONERROR);
		return;
	}

	// 2. 创建 VS
	if (FAILED(
		Win->Gfx().pDevice->CreateVertexShader(
			Blob->GetBufferPointer(),
			Blob->GetBufferSize(),
			nullptr,
			&pVertexShader)
	))
	{
		std::wstring msg = L"Failed to create vertex shader from file:\n" + name;
		MessageBox(NULL, msg.c_str(), L"Shader Error", MB_OK | MB_ICONERROR);
		return;
	}
}


void LoadPS(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pPixelShader, const std::wstring& name)
{
	// 1. 读取 cso 文件
	if (FAILED(D3DReadFileToBlob(name.c_str(), &Blob)))
	{
		std::wstring msg = L"Failed to load pixel shader file:\n" + name;
		MessageBox(NULL, msg.c_str(), L"Shader Error", MB_OK | MB_ICONERROR);
		return;
	}

	// 2. 创建 Pixel Shader
	if (FAILED(
		Win->Gfx().pDevice->CreatePixelShader(
			Blob->GetBufferPointer(),
			Blob->GetBufferSize(),
			nullptr,
			&pPixelShader)
	))
	{
		std::wstring msg = L"Failed to create pixel shader from file:\n" + name;
		MessageBox(NULL, msg.c_str(), L"Shader Error", MB_OK | MB_ICONERROR);
		return;
	}
}


// 光栅化状态
Microsoft::WRL::ComPtr<ID3D11RasterizerState> CreateRasterizerState_3D(const RenderStateDesc& desc)
{
	// ✅ 根据自定义的 RenderStateDesc 构造 D3D11_RASTERIZER_DESC
	D3D11_RASTERIZER_DESC rsDesc = {};

	rsDesc.FillMode = desc.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rsDesc.CullMode = desc.cullBack ? D3D11_CULL_BACK :
		(desc.flipCull ? D3D11_CULL_FRONT : D3D11_CULL_NONE);
	rsDesc.FrontCounterClockwise = desc.FrontCounterClockwise;
	rsDesc.DepthClipEnable = desc.depthClipEnable;
	rsDesc.ScissorEnable = desc.scissorEnable;
	rsDesc.MultisampleEnable = desc.multisample;
	rsDesc.AntialiasedLineEnable = desc.wireframe;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	//rsDesc.CullMode = D3D11_CULL_FRONT;

	// ✅ 创建渲染状态对象
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer;
	if (FAILED(Win->Gfx().pDevice->CreateRasterizerState(&rsDesc, &rasterizer)))
	{
		MessageBox(NULL, L"Failed to create rasterizer state.", L"Error", MB_OK | MB_ICONERROR);
	}

	return rasterizer;
}

std::string ChangeExtension(const std::string& path, const std::string& newExt)
{
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return path + newExt; // 没有后缀就直接加

	return path.substr(0, pos) + newExt;
}



AABB MakeWorldAABB(const Object_3D* obj)
{
	const auto& local = obj->bound;
	const auto& pos = obj->Transform.Position;
	const auto& scale = obj->Transform.Scale;

	// 1. 世界中心点
	DirectX::XMFLOAT3 worldCenter;
	worldCenter.x = pos.x + local.center.x * scale.x;
	worldCenter.y = pos.y + local.center.y * scale.y;
	worldCenter.z = pos.z + local.center.z * scale.z;

	// 2. 世界半尺寸（不考虑旋转）
	DirectX::XMFLOAT3 worldExtent;
	worldExtent.x = local.size.x * scale.x;
	worldExtent.y = local.size.y * scale.y;
	worldExtent.z = local.size.z * scale.z;

	// 3. 计算 min / max
	AABB aabb;
	aabb.min.x = worldCenter.x - worldExtent.x;
	aabb.min.y = worldCenter.y - worldExtent.y;
	aabb.min.z = worldCenter.z - worldExtent.z;

	aabb.max.x = worldCenter.x + worldExtent.x;
	aabb.max.y = worldCenter.y + worldExtent.y;
	aabb.max.z = worldCenter.z + worldExtent.z;

	return aabb;
}

bool CheckAABBCollision(const Object_3D* a, const Object_3D* b)
{
	if (a == nullptr || b == nullptr) return false;

	AABB boxA = MakeWorldAABB(a);
	AABB boxB = MakeWorldAABB(b);

	if (boxA.max.x < boxB.min.x || boxA.min.x > boxB.max.x) return false;
	if (boxA.max.y < boxB.min.y || boxA.min.y > boxB.max.y) return false;
	if (boxA.max.z < boxB.min.z || boxA.min.z > boxB.max.z) return false;

	return true;
}

int Sign(float v)
{
	if (v > 0.001f)  return  1;
	if (v < -0.001f) return -1;
	return 0;
}
