// =========================================================
// editor.cpp プレイヤー制御
// 
// 制作者:		日付：
// =========================================================
#include "main.h"
#include "texture.h"
#include "sprite.h"
#include "editor.h"
#include "collision.h"
#include "game.h"
#include "fade.h"
#include <filesystem>


// =========================================================
// マクロ宣言
// =========================================================


#include <fstream>

// =========================================================
// グローバル変数
// =========================================================

void getanime(const std::string path , std::vector<int>& ID, Object_2D::Vec2& size)
{
	size = { 0, 0 }; // 初始化最大值

	for (auto& entry : std::filesystem::directory_iterator(path))
	{
		if (!entry.is_regular_file()) continue;

		std::string file = entry.path().string();

		// 判断是否是图片
		std::string ext = entry.path().extension().string();
		for (auto& c : ext) c = tolower(c);

		if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
			ext == ".bmp" || ext == ".tga" || ext == ".gif")
		{
			Object_2D::Vec2 curSize = { 0, 0 };

			// 加载并取到当前图片大小
			int texID = LoadTexture(file, curSize);
			ID.push_back(texID);

			// 仅在加载了图片时比较大小
			size.x = (std::max)(size.x, curSize.x);
			size.y = (std::max)(size.y, curSize.y);
		}
	}
}

// =========================================================
// プレイヤー初期化
// =========================================================
std::vector<animation_Object_2D> obj_an;

static bool g_Loading = false;
static float g_LoadProgress = 0.0f;
static float g_LoadMax = 1.0f;

Object_2D Mouse(0, 0, 10, 10);

static float prevMouseX = 0.0f;
static float prevMouseY = 0.0f;

void InitializeEditor(void)
{
	std::string path = "Assets/Animations";
	Mouse.texNo = LoadTexture("rom/color/white.png");
	// 遍历整个文件夹内容
	for (auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			// 获取文件夹名
			std::string folderName = entry.path().filename().string();
			std::string path_ = entry.path().string();
			// 全小写
			for (auto& c : folderName) c = tolower(c);

			// 判断是否以 .animation 结尾
			const std::string suffix = ".an";
			if (folderName.size() >= suffix.size() &&
				folderName.compare(folderName.size() - suffix.size(), suffix.size(), suffix) == 0)
			{
				animation_Object_2D obj_anmation_2D;

				for (auto& entry_child : std::filesystem::directory_iterator(path_))
				{

					if (entry_child.is_directory())
					{
						animation an;
						
						an.name = entry_child.path().filename().string();
						std::string path__ = entry_child.path().string();
						getanime(path__, an.animation_id,an.size);

						obj_anmation_2D.list.push_back(an);
					}
				}
				obj_anmation_2D.name = folderName;
				obj_an.push_back(obj_anmation_2D);
			}
		}
	}
	LoadAllObjectParams(); // ← 加载动画后再补充参数
}

// =========================================================
// プレイヤー更新
// =========================================================

void UpdateEditor(void)
{
	if (obj_an.empty()) return;
	for (auto& obj : obj_an)
	{
		if (!obj.animPlaying) continue;

		Object_2D::Vec2 obj_size;
		if (obj.list.empty()) continue;
		obj_size.x = obj.list[obj.animationID].size.x;
		obj_size.y = obj.list[obj.animationID].size.y;

		obj.obj.Size = obj_size;
		obj.obj.texNo = obj.list[obj.animationID].animation_id[obj.list[obj.animationID].IDnum];
		obj.list[obj.animationID].IDnum += obj.speed;
		if (obj.list[obj.animationID].IDnum > obj.list[obj.animationID].animation_id.size()-1)
		{
			obj.list[obj.animationID].IDnum = 0;
		}
	}

	if (Win->mouse.IsInWindow())
	{
		auto [mouseX, mouseY] = Win->mouse.GetCorrectedPos(SCREEN_WIDTH, SCREEN_HEIGHT);

		if (Win->mouse.MiddleIsPressed())
		{
			float deltaX = mouseX - prevMouseX;
			float deltaY = mouseY - prevMouseY;

			WinCamera.pos.x -= deltaX;
			WinCamera.pos.y += deltaY;
		}
		prevMouseX = mouseX;
		prevMouseY = mouseY;
		// 中键未按下，控制角色
		Mouse.Transform.Position.x = mouseX + WinCamera.pos.x;
		Mouse.Transform.Position.y = -mouseY + WinCamera.pos.y;
	}
}

// =========================================================
// プレイヤー描画
// =========================================================
void DrawEditor(void)
{
	/*for (auto& object_2d : obj_an)
	{
		BoxVertexsMake(&object_2d.obj);
		DrawObject_2D(&object_2d.obj);
	}*/
}

// =========================================================
// プレイヤー終了処理
// =========================================================
void FinalizeEditor(void)
{
	
}

void SaveObjectParamBinary(const std::string& dir, const animation_Object_2D& data)
{
	// 保存路径 = 目录 + / + 名称 + 后缀
	std::string filePath = dir + "/" + data.name + ".oparam";

	std::ofstream ofs(filePath, std::ios::binary);
	if (!ofs.is_open()) return;

	const Object_2D& obj = data.obj;

	ofs.write((char*)&obj.Transform.Position.x, sizeof(float));
	ofs.write((char*)&obj.Transform.Position.y, sizeof(float));

	ofs.write((char*)&obj.Transform.Rotation, sizeof(float));

	ofs.write((char*)&obj.Transform.Scale.x, sizeof(float));
	ofs.write((char*)&obj.Transform.Scale.y, sizeof(float));

	ofs.write((char*)&obj.flipX, sizeof(bool));
	ofs.write((char*)&obj.flipY, sizeof(bool));

	ofs.write((char*)&obj.Z_Test, sizeof(float));

	ofs.close();
}


bool LoadObjectParamBinary(const std::string& dir, animation_Object_2D& data)
{
	std::string filePath = dir + "/" + data.name + ".oparam";

	if (!std::filesystem::exists(filePath))
		return false;   // 没有就跳过

	std::ifstream ifs(filePath, std::ios::binary);
	if (!ifs.is_open()) return false;

	Object_2D& obj = data.obj;

	ifs.read((char*)&obj.Transform.Position.x, sizeof(float));
	ifs.read((char*)&obj.Transform.Position.y, sizeof(float));

	ifs.read((char*)&obj.Transform.Rotation, sizeof(float));

	ifs.read((char*)&obj.Transform.Scale.x, sizeof(float));
	ifs.read((char*)&obj.Transform.Scale.y, sizeof(float));

	ifs.read((char*)&obj.flipX, sizeof(bool));
	ifs.read((char*)&obj.flipY, sizeof(bool));

	ifs.read((char*)&obj.Z_Test, sizeof(float));

	ifs.close();
	return true;
}


void LoadAllObjectParams()
{
	for (auto& obj : obj_an)
	{
		std::string dir = "Assets/Animations/" + obj.name;
		LoadObjectParamBinary(dir, obj);
	}
}


std::vector<animation_Object_2D>& Getobj_an(void)
{
	return obj_an;
}

Object_2D& GetMouse(void)
{
	return Mouse;
}

bool IsAnimationFinished(animation_Object_2D& obj)
{
	if (obj.animationID < 0 || obj.animationID >= obj.list.size())
		return false;

	auto& anim = obj.list[obj.animationID];
	// IDnumが最大フレーム数-1に達したら終了
	return anim.IDnum >= (anim.animation_id.size() - 2);  // または anim.maxFrames - 1
}

// =========================================================
// ヘルパー関数
// =========================================================

// 名前でオブジェクトを検索
animation_Object_2D* FindObjectByName(const std::string& name)
{
	for (auto& obj : obj_an)
	{
		if (obj.name == name)
			return &obj;
	}
	return nullptr;
}

// アニメーションを名前で設定
bool SetAnimation(animation_Object_2D& obj, const std::string& animName)
{
	for (int i = 0; i < obj.list.size(); i++)
	{
		if (obj.list[i].name == animName)
		{
			if (obj.animationID != i)  // 変更があった場合のみ
			{
				obj.animationID = i;
				obj.list[i].IDnum = 0;  // アニメーションをリセット
			}
			return true;
		}
	}
	return false;
}
