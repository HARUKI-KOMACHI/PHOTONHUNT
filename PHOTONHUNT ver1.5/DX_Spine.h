#pragma once
#include <string>
#include "main.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Vector_Spine
{
	DirectX::XMFLOAT4 position;   // 位置
	DirectX::XMFLOAT2 texcoord;   // 纹理坐标

	uint32_t boneIndex[4] = {0,0,0,0};          // 影响该顶点的骨骼
	float    boneWeight[4] = {0,0,0,0};         // 权重
};

enum class AtlasState
{
	None,        // 还没读到任何有效内容
	Page,        // 主图片（page / atlas 图片）
	Region       // 子图片（region）
};

struct AtlasPage {
	std::string imageFile; // xxx.png
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTexture;//材质
	int width = 0;
	int height = 0;
};

struct AtlasRegion {
	std::string name;
	std::string pageImage; // 来自哪张图片页
	int x = 0, y = 0;
	int width = 0, height = 0;
	int origWidth = 0, origHeight = 0;
	int offsetX = 0, offsetY = 0;
	bool rotated = false;

	// UV (0~1) 范围，可计算得到（非 atlas 原始字段）
	float u = 0, v = 0, u2 = 0, v2 = 0;
};

struct Atlas {
	std::unordered_map<std::string, AtlasPage> pages;
	std::unordered_map<std::string, AtlasRegion> regions;
	std::string name;
};


void LoadSpineAtlas(Atlas& atlas, std::string path);

class Spine
{
public:
	struct Skeleton
	{
		std::string hash;
		std::string spine;
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 size;
		std::string images;
		std::string audio;
	};

	struct Bone
	{
		//std::string name;
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 scale;

		float rotation;             // 4
		DirectX::XMFLOAT3 _pad;     // 12 → 后 16 字节
	};

	struct Slot
	{
		std::string name = "";
		std::string bone = "";
		std::string attachment = "";


	};

	struct Attachment
	{
		std::string slotName;
		std::string attachmentName;
		/// <summary>
		/// //////////////////////
		/// </summary>
		std::vector<Vector_Spine> points;
		std::vector<uint32_t> indices_32;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer = nullptr; // 顶点缓冲区句柄/索引
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer = nullptr; // 索引缓冲区句柄/索引
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer = nullptr;        // 常量

		DirectX::XMFLOAT2 pos;
		float rotation;
		DirectX::XMFLOAT2 scale;

		int hull;
		int indicesCount;
		std::vector<DirectX::XMFLOAT2> edges;
		DirectX::XMFLOAT2 size;
	};

	struct Animation
	{
		struct Translate
		{
			float time = 0.0f; // 时间点（秒）
			DirectX::XMFLOAT2 pos = {0.0f,0.0f};

		};

		struct Rotate
		{
			float time = 0.0f; // 时间点（秒）
			float rotation = 0.0f;
		};

		struct Scale
		{
			float time = 0.0f; // 时间点（秒）
			DirectX::XMFLOAT2 scale = { 0.0f,0.0f };

		};

		struct animation_bone
		{
			std::vector<Translate> translate;
			std::vector<Rotate> rotate;
			std::vector<Scale> scale;
		};
		std::string name;
		std::unordered_map<std::string, Animation::animation_bone> bones;
	};

public:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTexture;//材质
	Skeleton skeleton;
	std::unordered_map<std::string, Bone> bones;
	std::vector<Bone> bones_vector;
	std::unordered_map<std::string, Animation> animations;
	std::unordered_map<std::string, Attachment> attachments;
	std::vector<Slot> slots;
	Atlas atlas;
	Object_2D::Transform_2D Transform;
	unsigned int texNo = -1;
public:
	Spine(std::string path);
	~Spine(void);
private:
	json j;
};

void DrawObject_Spine(Spine* spine);

void  DrawSpine(int n);

void inti_Object_2D_spine();

void inti_spine();
void ResetspineCB(DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 scale, float rotation);

extern std::vector<Spine*> object_spine_list_Darw;