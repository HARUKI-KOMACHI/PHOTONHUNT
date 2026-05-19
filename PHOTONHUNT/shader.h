/*==============================================================================

   シェーダー [shader.h]
														 Author : Youhei Sato
														 Date   : 2025/05/15
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef SHADER_H
#define	SHADER_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <optional>
#include "sprite.h"

#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }


bool Shader_Initialize(Microsoft::WRL::ComPtr<ID3D11Device>&pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>&pContext);
void Shader_Finalize();

bool CreateRasterizerState(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext);
bool CreateViewport(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext,
	int width, int height);

struct Camera
{
	Object_2D::Vec2 pos = { 0,0 };
	float Rotation = 0.0f;
	float Scale = 1.0f;
};


struct Camera_3D
{
	struct CameraBuffer
	{
		DirectX::XMMATRIX view;       // 观察矩阵
		DirectX::XMMATRIX proj;       // 投影矩阵
		DirectX::XMFLOAT3 cameraPos;  // 相机位置
		float padding;                // 填充（16字节对齐）
	};

	float yaw = 0.0f;  // 左右角度
	float pitch = 0.0f;  // 上下角度
	float distance = 5.0f; // 距离
	DirectX::XMFLOAT3 target = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f }; // 由 yaw/pitch/distance 算出来

	Microsoft::WRL::ComPtr<ID3D11Buffer> g_cameraCB;   // 相机常量

};
extern Camera_3D WinCamera_3D;
extern Camera_3D WinLight_3D;
extern Camera WinCamera;
extern Microsoft::WRL::ComPtr<ID3D11Buffer> g_pVertexBuffer;

extern Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader;
extern Microsoft::WRL::ComPtr<ID3D11InputLayout> g_pInputLayout;
extern Microsoft::WRL::ComPtr<ID3D11Buffer> g_pVSConstantBuffer;
extern Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader;
extern Microsoft::WRL::ComPtr<ID3D11Buffer> g_pIndexBuffer;

/////////////////////////////////////////
extern Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader_uv;
extern Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader_uv;
extern Microsoft::WRL::ComPtr<ID3D11InputLayout> g_pInputLayout_uv;//输入布局
extern Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState_uv; // 光栅化状态
////////////////////////////////

extern Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState;
extern std::optional<D3D11_VIEWPORT> viewport;

extern Microsoft::WRL::ComPtr<ID3D11Buffer> g_pConstantBuffer;



//顶点上传
bool CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>&pDevice);

//索引上传
bool CreateIndexBuffer(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice);

// Shader 和 输入布局
bool CompileAndUploadShader(Microsoft::WRL::ComPtr<ID3D11Device>& pDevice);


//常量定义
bool CreateConstantBuffer(
	Microsoft::WRL::ComPtr<ID3D11Device>& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>& pContext);

void ResetConstantBuffer(ID3D11DeviceContext* context);
#endif // SHADER_H
