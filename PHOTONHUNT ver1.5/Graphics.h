#pragma once
#include "ChiliWin.h"
#include <d3d11.h>
#include <wrl.h>
#include "ChiliTimer/ChiliTimer.h"

#pragma comment(lib, "dxgi.lib")



#include <DirectXMath.h>
#include "shader.h"
#include "sprite_3D.h"

enum BLENDSTATE
{
	BLENDSTATE_NONE = 0,
	BLENDSTATE_ALFA,
	BLENDSTATE_ADD,
	BLENDSTATE_SUB,
	BLENDSTATE_PREMULTIPLIED,   // ← 新增：无黑边透明

	BLENDSTATE_MAX,
};

enum class LightType {
	Directional,  // 平行光（太阳）
	Point,        // 点光源（灯泡）
	Spot,         // 聚光灯（手电筒）
	Beam          // 线性光 / 激光型光
};

struct LightData {
	int type;             // LightType
	DirectX::XMFLOAT3 position;      // 光源位置
	DirectX::XMFLOAT3 direction;     // 光源方向
	float range;          // 最大照射距离
	DirectX::XMFLOAT3 color;         // 光颜色
	float intensity;      // 光强
	float angle;          // 聚光灯角度（弧度制）
	int enabled;          // 是否启用
	int castShadow;       // 是否投影
};

struct Light {
	LightType type;       // 光类型
	LightData data;       // 光参数数据
	Microsoft::WRL::ComPtr<ID3D11Buffer> pLightBuffer; // GPU常量缓冲
	int version = 0;      // 版本号
	bool active = true;   // 是否启用
};

struct LightBufferData
{
	DirectX::XMFLOAT3 lightDirection; // 12 bytes
	float intensity;                  // 16 ✅ 对齐

	DirectX::XMFLOAT3 lightColor;     // 12 bytes
	float ambientFactor;              // 16 ✅ 对齐

	DirectX::XMFLOAT3 lightPos;       // ✅新增：光源世界坐标
	float padding2;                   // ✅对齐到 16 字节
};

class Graphics
{
public:

	void depth();

	void SetBlendState(BLENDSTATE blend);
private:
	void InitBlendStates();

	
public:
	int width = 0;
	int height = 0;
public:
	Graphics() {};
	Graphics(HWND hWnd, int w, int h);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics() = default;


	void EndFrame();
	void ClearBuffer(float red, float green, float blue)noexcept;
	void CreateDynamicCollisionBuffer(Object_2D* obj);

	void Draw(ID3D11DepthStencilView* dsv, ID3D11RasterizerState* rs = nullptr);

	ChiliTimer timer;

public:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;               
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;               
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;       
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;     

	Microsoft::WRL::ComPtr<ID3D11BlendState> pNoColorWriteBlend; // ステンシル書き込み専用
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizerState_Scissor;

	//////////////////////////////////////////////////////////////////////////////
public:
	void EndFrame_3D();

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pMainDepthStencilState;
	//场景深度
	Microsoft::WRL::ComPtr<ID3D11Texture2D>        sceneDepth;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> sceneDSV;

	void EndSceneRender();

	// =============================================================
// 光照
// =============================================================
	HRESULT EnsureSceneLightTargets(UINT w, UINT h);
	void BeginSceneLightRender();
	bool RenderLightScene(void);
	Microsoft::WRL::ComPtr<ID3D11Texture2D>        sceneLightDepth;  // 灯光视角深度纹理
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneLightRTV;    // 灯光渲染深度写入
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneLightDepthSRV; // 阴影贴图采样使用

	// =============================================================
// 不透明场景颜色缓冲（主画面颜色输出）
// =============================================================
	HRESULT EnsureSceneOpaqueTargets(UINT w, UINT h);
	void BeginSceneOpaqueRender();
	bool RenderOpaqueScene(void);
	Microsoft::WRL::ComPtr<ID3D11Texture2D>        sceneOpaqueTex;   // 不透明场景颜色纹理
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneOpaqueRTV;   // 渲染目标视图（写入颜色）
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneOpaqueSRV; // 着色器资源视图（用于后期读取）

	// =============================================================
	// 透明场景颜色缓冲（透明对象绘制）
	// =============================================================
	HRESULT EnsureSceneTransparentTargets(UINT w, UINT h);
	void BeginSceneTransparentRender();
	bool RenderTransparentScene(void);
	Microsoft::WRL::ComPtr<ID3D11Texture2D>        sceneTransparentTex;   // 透明场景颜色纹理
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> sceneTransparentRTV;   // 渲染目标视图（用于绘制透明对象）
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sceneTransparentSRV; // 着色器资源视图（用于后期合成）

	void Init_3D(void);

	DirectX::XMMATRIX transform_proj;
	DirectX::XMMATRIX transform_Light_proj;
	DirectX::XMMATRIX view_imgui;
	
	// 更新相机常量缓冲（Unity 同款 OrbitCamera）
	void UpdateConstantBuffer(
		Camera_3D camera_3D,
		const DirectX::XMMATRIX& projMatrix);
	void UpdateConstantBuffer(
		const Light& light,
		const DirectX::XMMATRIX& projMatrix);
	// 透明对象排序（从远到近）
	void SortTransparentObjects(const DirectX::XMFLOAT3& cameraPos);

	// 更新物体常量缓冲
	void UpdateObjectConstantBuffer(
		Object_3D& object);
};

Object_3D::Bounds ComputeWorldBounds(const Object_3D* obj);

extern Light light;