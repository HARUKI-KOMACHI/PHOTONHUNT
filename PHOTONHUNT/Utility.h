#pragma once

#include "main.h"



struct RenderStateDesc
{
    bool wireframe = false;         // 是否线框模式
    bool showPoint = false;         // 是否显示顶点
    bool cullBack = true;           // 是否剔除背面
    bool flipCull = false;          // 是否翻转剔除方向（描边）
    bool depthEnable = true;        // 是否启用深度测试
    bool depthWrite = true;         // 是否写入深度缓冲
    bool scissorEnable = false;     // 是否启用裁剪
    bool multisample = false;       // 是否启用MSAA
    bool depthClipEnable = true;    // 是否启用深度裁切
    int version = 1;                // 版本号，用于追踪修改
    BOOL FrontCounterClockwise = true;
};

void LoadVS(Microsoft::WRL::ComPtr<ID3D11VertexShader>& pVertexShader, const std::wstring& name);
void LoadPS(Microsoft::WRL::ComPtr<ID3D11PixelShader>& pPixelShader, const std::wstring& name);

// 光栅化状态
Microsoft::WRL::ComPtr<ID3D11RasterizerState> CreateRasterizerState_3D(const RenderStateDesc& desc);

std::string ChangeExtension(const std::string& path, const std::string& newExt);



struct AABB
{
	DirectX::XMFLOAT3 min;
	DirectX::XMFLOAT3 max;
};

AABB MakeWorldAABB(const Object_3D* obj);

bool CheckAABBCollision(const Object_3D* a, const Object_3D* b);

int Sign(float v);
