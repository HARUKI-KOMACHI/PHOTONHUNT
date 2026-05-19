
#include "main.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include "texture.h"

#include "shader.h"
#include "debug_ostream.h"

#include <filesystem>
#include <sstream>
#include <fstream>
#include <unordered_map>

#include "sprite_3D.h"

Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_3D;//创建像素shader
Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_3D;// 创建顶点shader
Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_3D_Light;//创建像素shader
Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_3D_Light;// 创建顶点shader
Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout_3D;       // 输入布局



void DrawObject_3D(Object_3D* obj_3D)
{

	if (obj_3D->type == RenderLayer::Opaque)
	{
		object_3D_list_Darw_Opaque.push_back(obj_3D);
	}
	if (obj_3D->type == RenderLayer::Transparent)
	{
		object_3D_list_Darw_Transparent.push_back(obj_3D);
	}


}

Object_3D::Object_3D()
{
	Transform = {
		{0,0,0},
		{0,0,0},
		{1,1,1}
	};

	Color = { 1,1,1,1 };
	type = RenderLayer::Opaque;
	// ✅ 创建物体用的常量缓冲 (即使是空的，也先占一个位置)
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(MatrixBuffer_new);   // 这里 MatrixBuffer = 你的物体常量结构

	HRESULT hr = Win->Gfx().pDevice->CreateBuffer(&cbd, nullptr, pConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create constant buffer.", L"Error", MB_OK | MB_ICONERROR);
	}
}
Object_3D::Object_3D(float x, float y, float z, std::string path = "")
{
	if (path != "")
	{
		meshes_parser(path,pVertexBuffer, pIndexBuffer, bound, indicesCount);
	}
	
	Transform= {
		{x,y,z},
		{0,0,0},
		{1,1,1}
	};

	Color = { 1,1,1,1 };
	type = RenderLayer::Opaque;
	// ✅ 创建物体用的常量缓冲 (即使是空的，也先占一个位置)
	D3D11_BUFFER_DESC cbd = {};
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(MatrixBuffer_new);   // 这里 MatrixBuffer = 你的物体常量结构

	HRESULT hr = Win->Gfx().pDevice->CreateBuffer(&cbd, nullptr, pConstantBuffer.GetAddressOf());
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create constant buffer.", L"Error", MB_OK | MB_ICONERROR);
	}

}

void Object_3D::UploadSkinningCB()
{
	SkinningCB skinCB{};
	skinCB.HasSkin = BoneInfos.empty() ? 0 : 1;

	const size_t MAX_BONES = 540;
	size_t count = BoneInfos.size() < MAX_BONES ? BoneInfos.size() : MAX_BONES;

	for (size_t i = 0; i < count; ++i)
		skinCB.Bones[i] = BoneInfos[i].skinMatrix;

	Win->Gfx().pContext->UpdateSubresource(
		skinCBuf.Get(), 0, nullptr, &skinCB, 0, 0
	);

	Win->Gfx().pContext->VSSetConstantBuffers(4, 1, skinCBuf.GetAddressOf());
}




bool InitinputLayouts(void)
{
	// 添加着色器shader
	Microsoft::WRL::ComPtr<ID3DBlob> pPixelBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pVertexBlob;

	// 编译像素着色器
	if (FAILED(D3DReadFileToBlob(L"PS_StaticObject.cso", &pPixelBlob))) {
		MessageBox(NULL, L"Failed to load pixel shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (FAILED(Win->Gfx().pDevice->CreatePixelShader(pPixelBlob->GetBufferPointer(), pPixelBlob->GetBufferSize(), nullptr, &pPixelShader_3D))) {
		MessageBox(NULL, L"Failed to create pixel shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// 编译顶点着色器
	if (FAILED(D3DReadFileToBlob(L"VS_StaticObject.cso", &pVertexBlob))) {
		MessageBox(NULL, L"Failed to load vertex shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (FAILED(Win->Gfx().pDevice->CreateVertexShader(pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize(), nullptr, &pVertexShader_3D))) {
		MessageBox(NULL, L"Failed to create vertex shader.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	// 定义输入布局
	//D3D11_INPUT_ELEMENT_DESC layout[] = {
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexStatic3D, v), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexStatic3D, vn), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(VertexStatic3D, vt), D3D11_INPUT_PER_VERTEX_DATA, 0 }
	//};
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexStatic3D, v), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexStatic3D, vn), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VertexStatic3D, vt), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, offsetof(VertexStatic3D, boneIndex), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VertexStatic3D, boneWeight), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	// 使用顶点着色器的字节码创建输入布局
	if (FAILED(Win->Gfx().pDevice->CreateInputLayout(
		layout,
		UINT(std::size(layout)),
		pVertexBlob->GetBufferPointer(),
		pVertexBlob->GetBufferSize(),
		&pInputLayout_3D)))
	{
		MessageBox(NULL, L"Failed to create input layout.", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
}

// 自定义哈希函数，用于生成顶点的唯一键值
struct VertexHash {
	std::size_t operator()(const VertexStatic3D& v) const {
		return std::hash<float>()(v.v.x) ^ std::hash<float>()(v.v.y) ^ std::hash<float>()(v.v.z) ^
			std::hash<float>()(v.vn.x) ^ std::hash<float>()(v.vn.y) ^ std::hash<float>()(v.vn.z) ^
			std::hash<float>()(v.vt.x) ^ std::hash<float>()(v.vt.y);
	}
};

// 自定义相等判断，用于比较顶点是否相同
struct VertexEqual {
	bool operator()(const VertexStatic3D& v1, const VertexStatic3D& v2) const {
		return (v1.v.x == v2.v.x && v1.v.y == v2.v.y && v1.v.z == v2.v.z &&
			v1.vn.x == v2.vn.x && v1.vn.y == v2.vn.y && v1.vn.z == v2.vn.z &&
			v1.vt.x == v2.vt.x && v1.vt.y == v2.vt.y);
	}
};


bool meshes_parser(std::string& path, 
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pVertexBuffer,
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pIndexBuffer,
	Object_3D::Bounds& bound,
	int& indicesCount)
{

	std::filesystem::path filePath(path);

	//下面开始处理顶点和索引的读取
	std::vector<VertexStatic3D> point;      // 每个点包含：位置+法线+UV，打包后的结构（DX上传用）
	std::vector<uint32_t> indices_32;    // 索引列表



	std::vector<DirectX::XMFLOAT3> v;
	DirectX::XMFLOAT3 verts;
	std::vector<DirectX::XMFLOAT2> vt;
	DirectX::XMFLOAT2 vertext;
	std::vector<DirectX::XMFLOAT3> vn;
	DirectX::XMFLOAT3 vernormal;

	std::vector<uint32_t> Indices_32;
	std::vector<uint32_t> Indices_vt_32;
	std::vector<uint32_t> Indices_vn_32;

	std::string line;
	std::ifstream file(path);
	if (!file.is_open())
	{
		return -1;
	}
	while (getline(file, line))
	{
		std::stringstream s;
		s.clear();   // 清理流
		s.str("");   // 清空流
		s << line;

		char junk;
		int ty = 0;

		if (line[0] == 'v')
		{
			if (line[1] == ' ')
			{
				s >> junk >> verts.x >> verts.y >> verts.z;
				verts.z *= -1;
				v.push_back(verts);
			}
			if (line[1] == 't') // 处理纹理坐标
			{
				s >> junk >> junk >> vertext.x >> vertext.y;
				vt.push_back(vertext);
			}
			else if (line[1] == 'n') // 处理法线
			{
				s >> junk >> junk >> vernormal.x >> vernormal.y >> vernormal.z;
				vernormal.z *= -1;
				vn.push_back(vernormal);
			}

		}

		if (line[0] == 'f')
		{
			for (auto a : line)
			{
				if (a == ' ')
				{
					ty++;
				}
			}

			if (ty == 3)
			{
				int f[3][3];
				s >> junk >>
					f[0][0] >> junk >> f[0][1] >> junk >> f[0][2] >>
					f[1][0] >> junk >> f[1][1] >> junk >> f[1][2] >>
					f[2][0] >> junk >> f[2][1] >> junk >> f[2][2];
				Indices_32.push_back(f[0][0] - 1);
				Indices_32.push_back(f[1][0] - 1);
				Indices_32.push_back(f[2][0] - 1);
				Indices_vt_32.push_back(f[0][1] - 1);
				Indices_vt_32.push_back(f[1][1] - 1);
				Indices_vt_32.push_back(f[2][1] - 1);
				Indices_vn_32.push_back(f[0][2] - 1);
				Indices_vn_32.push_back(f[1][2] - 1);
				Indices_vn_32.push_back(f[2][2] - 1);
			}
			else if (ty == 4)
			{
				int f[4][3];
				s >> junk >>
					f[0][0] >> junk >> f[0][1] >> junk >> f[0][2] >>
					f[1][0] >> junk >> f[1][1] >> junk >> f[1][2] >>
					f[2][0] >> junk >> f[2][1] >> junk >> f[2][2] >>
					f[3][0] >> junk >> f[3][1] >> junk >> f[3][2];
				Indices_32.push_back(f[0][0] - 1);
				Indices_32.push_back(f[1][0] - 1);
				Indices_32.push_back(f[2][0] - 1);
				Indices_32.push_back(f[0][0] - 1);
				Indices_32.push_back(f[2][0] - 1);
				Indices_32.push_back(f[3][0] - 1);
				Indices_vt_32.push_back(f[0][1] - 1);
				Indices_vt_32.push_back(f[1][1] - 1);
				Indices_vt_32.push_back(f[2][1] - 1);
				Indices_vt_32.push_back(f[0][1] - 1);
				Indices_vt_32.push_back(f[2][1] - 1);
				Indices_vt_32.push_back(f[3][1] - 1);
				Indices_vn_32.push_back(f[0][2] - 1);
				Indices_vn_32.push_back(f[1][2] - 1);
				Indices_vn_32.push_back(f[2][2] - 1);
				Indices_vn_32.push_back(f[0][2] - 1);
				Indices_vn_32.push_back(f[2][2] - 1);
				Indices_vn_32.push_back(f[3][2] - 1);
			}

		}

	}


	file.close();


	//// 使用哈希表进行顶点查找
	std::unordered_map<VertexStatic3D, int, VertexHash, VertexEqual> vertexMap;
	for (int i = 0; i < Indices_32.size(); i++) {
		VertexStatic3D List;

		List.v = v[Indices_32[i]];
		List.vn = vn[Indices_vn_32[i]];
		List.vt = vt[Indices_vt_32[i]];

		auto it = vertexMap.find(List);
		if (it != vertexMap.end()) {
			indices_32.push_back(it->second);  // 更新索引
		}
		else {
			point.push_back(List);
			vertexMap[List] = point.size() - 1;
			indices_32.push_back(point.size() - 1);  // 更新索引
		}
	}

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(point.size() * sizeof(VertexStatic3D));
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexDataDesc = {};
	vertexDataDesc.pSysMem = point.data();

	if (!Win) {
		MessageBox(nullptr, L"Win is null", L"Error", MB_OK);
		return -1;
	}

	if (!Win->Gfx().pDevice) {
		MessageBox(nullptr, L"pDevice is null", L"Error", MB_OK);
		return -1;
	}

	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&vertexBufferDesc, &vertexDataDesc, pVertexBuffer.GetAddressOf())))
	{
		MessageBox(NULL, L"Failed to create vertex buffer.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}


	D3D11_BUFFER_DESC indexBufferDesc = {};
	D3D11_SUBRESOURCE_DATA indexData = {};

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.CPUAccessFlags = 0u;
	indexBufferDesc.ByteWidth = indices_32.size() * sizeof(uint32_t);
	indexBufferDesc.StructureByteStride = sizeof(uint32_t);

	indexData.pSysMem = indices_32.data();

	indicesCount = indices_32.size();

	if (FAILED(Win->Gfx().pDevice->CreateBuffer(&indexBufferDesc, &indexData, pIndexBuffer.GetAddressOf()))) {
		MessageBox(NULL, L"Failed to create index buffer.", L"Error", MB_OK | MB_ICONERROR);
		return -1;
	}

	// 假设 point 已经填充好所有顶点数据
	if (!point.empty())
	{
		DirectX::XMFLOAT3 minPos = point[0].v;
		DirectX::XMFLOAT3 maxPos = point[0].v;

		for (size_t i = 1; i < point.size(); ++i)
		{
			const auto& p = point[i].v;
			if (p.x < minPos.x) minPos.x = p.x;
			if (p.y < minPos.y) minPos.y = p.y;
			if (p.z < minPos.z) minPos.z = p.z;

			if (p.x > maxPos.x) maxPos.x = p.x;
			if (p.y > maxPos.y) maxPos.y = p.y;
			if (p.z > maxPos.z) maxPos.z = p.z;
		}

		// 计算中心点 = (max + min) / 2
		bound.center = {
			(minPos.x + maxPos.x) * 0.5f,
			(minPos.y + maxPos.y) * 0.5f,
			(minPos.z + maxPos.z) * 0.5f
		};

		// 计算 size = max - min
		bound.size = {
			maxPos.x - minPos.x,
			maxPos.y - minPos.y,
			maxPos.z - minPos.z
		};
	}

	

	return 1;
}




