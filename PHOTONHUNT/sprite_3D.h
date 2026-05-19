#pragma once

#include <vector>
#include <fbxsdk.h>

enum class RenderLayer
{
	Opaque,         // 不透明物体层
	Transparent,    // 半透明层
	TwoD,           // 2D层（Sprite/UI）
	Light,          // 光照层
	PostProcess,    // 后处理
	Debug,          // 调试层
	UI              // 编辑器UI层
};

struct VertexStatic3D {
	DirectX::XMFLOAT3 v = { 0.0f, 0.0f, 0.0f };        //坐标
	DirectX::XMFLOAT3 vn = { 0.0f, 0.0f, 0.0f };       //法线
	DirectX::XMFLOAT2 vt = { 0.0f, 0.0f };              //uv
	//DirectX::XMFLOAT4 color = { 0.0f, 0.0f ,0.0f,0.0f };//颜色
	// 下面是骨骼相关（跟顶点一起上传 GPU）
	uint32_t boneIndex[4] = { 0, 0, 0, 0 };      // 影响该顶点的骨骼索引
	float    boneWeight[4] = { 0, 0, 0, 0 };     // 对应权重（和为 1）
};

struct MatrixBuffer_new
{
	DirectX::XMMATRIX world; // 世界矩阵
};


class Object_3D
{
public:
	Object_3D();
	Object_3D(float x, float y, float z, std::string path);
	class Vec3
	{
	public:
		float x, y, z;
	};
	class Transform_3D
	{
	public:
		DirectX::XMFLOAT3 Position = { 0,0,0 };
		DirectX::XMFLOAT3 Rotation = { 0,0,0 };
		DirectX::XMFLOAT3 Scale = {1,1,1};

	};
	class ColorRGBA
	{
	public:
		float R = 1.0f;
		float G = 1.0f;
		float B = 1.0f;
		float A = 1.0f;
	};

	// 物体的包围盒，用于剔除/碰撞
	struct Bounds {
		DirectX::XMFLOAT3 center;   // 中心点（相对对象）
		DirectX::XMFLOAT3 size;     // 长宽高（Extent）
	};
	struct BoneInfo_3D
	{
		FbxNode* node;                      // 骨骼节点（FBX）
		DirectX::XMFLOAT4X4 invBindPose;    // 逆绑定姿态（初始化一次）
		DirectX::XMFLOAT4X4 skinMatrix;     // 当前帧的动画矩阵（每帧更新）
	};

	Transform_3D Transform;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer = nullptr; // 顶点缓冲区句柄/索引
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer = nullptr; // 索引缓冲区句柄/索引
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer = nullptr;        // 常量

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader = nullptr;//创建像素shader
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader = nullptr;// 创建顶点shader
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout = nullptr;       // 输入布局

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState = nullptr; // 光栅化状态


	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> pTextures;//材质

	Bounds bound;  // 每个 Mesh 自动自带一个包围盒
	ColorRGBA Color;
	RenderLayer type;
	int indicesCount;
	std::vector<BoneInfo_3D> BoneInfos; //骨骼
	Microsoft::WRL::ComPtr<ID3D11Buffer> skinCBuf = nullptr;
public:
	void UploadSkinningCB();

};

/// <summary>
/// //////////////////////////////
/// </summary>

extern Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_3D;//创建像素shader
extern Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_3D;// 创建顶点shader
extern Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader_3D_Light;//创建像素shader
extern Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader_3D_Light;// 创建顶点shader
extern Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout_3D;       // 输入布局


/// <summary>
/// /////////////////////////////////////////////
/// </summary>

void DrawObject_3D(Object_3D* obj_3D);
bool InitinputLayouts(void);

bool meshes_parser(std::string& path,
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pVertexBuffer,
	Microsoft::WRL::ComPtr<ID3D11Buffer>& pIndexBuffer,
	Object_3D::Bounds& bound,
	int& indicesCount);


