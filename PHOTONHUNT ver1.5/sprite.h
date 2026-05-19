#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <string>

enum class StencilRole { Normal, FloorWrite, ShadowMask };

struct mat4x4
{
	float m[4][4] = { 0 };
};

class Object_2D
{
public:
	Object_2D();
	Object_2D(float x, float y, float w, float h);
	class Vec2
	{
	public:
		float x, y;
		Vec2() : x(0), y(0) {}
		Vec2(float _x, float _y) : x(_x), y(_y) {}

		Vec2 operator-(const Vec2& rhs) const
		{
			return Vec2(x - rhs.x, y - rhs.y);
		}
	};
	class Transform_2D
	{
	public:
		Vec2 Position;
		float Rotation = 0.0f;
		Vec2 Scale = { 1.0f,1.0f };
		Vec2 Offset = { 0.0f,0.0f };
	};
	class ColorRGBA
	{
	public:
		float R = 1.0f;
		float G = 1.0f;
		float B = 1.0f;
		float A = 1.0f;
	};

	Vec2 Size;
	Transform_2D Transform;
	Vec2 UV[4];
	Vec2 Vertexs[4];
	ColorRGBA Color;
	unsigned int texNo = -1;
	int spine = -1;

	bool flipX = false;
	bool flipY = false;
	float Z_Test = 0.0f;
	Object_2D& operator=(const Object_2D& obj) {
		if (this != &obj) {
			Size = obj.Size;
			Transform = obj.Transform;
			for (int i = 0; i < 4; i++) {
				UV[i] = obj.UV[i];
				Vertexs[i] = obj.Vertexs[i];
			}
			Color = obj.Color;
			texNo = obj.texNo;
			flipX = obj.flipX;
			flipY = obj.flipY;

		}
		return *this;
	}

	Microsoft::WRL::ComPtr<ID3D11VertexShader> g_pVertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> g_pPixelShader = nullptr;
	std::vector< Microsoft::WRL::ComPtr<ID3D11Buffer>> g_pPSConstantBuffers;

	bool bShow = true;//表示する
	bool usePolyCollision = false;//ポリゴン判定を使用する
	bool showPolyCollision = false;//ポリゴン判定体を表示する

	std::vector<Object_2D::Vec2> vt;//顶点
	std::vector<uint16_t> Indices_vt;//索引

	void loadPolyCollision(std::string Path);//ポリゴン判定体を読み込む

	Microsoft::WRL::ComPtr<ID3D11Buffer> cachedVertexBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> cachedIndexBuffer = nullptr;
	bool bufferReady = false;
	int cachedIndexCount = 0;

	StencilRole stencilRole = StencilRole::Normal;
};
void Object_2D_UV(Object_2D* obj_2D, int bno, int wc, int hc);
void DrawObject_2D(Object_2D* obj_2D);
void BoxVertexsMake(Object_2D* obj_2D);

// プロトタイプ宣言
void UninitSprite();

void InitializeScreenOffset(void);

void SetScreenOffset(float x, float y);

float GetScreenOffsetX(void);

float GetScreenOffsetY(void);
