#pragma once

#ifndef MAIN_H
#define MAIN_H

#include "system.h"


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

//#define SCREEN_WIDTH 1280
//#define SCREEN_HEIGHT 720

//#define		SCREEN_WIDTH	(1280)
//#define		SCREEN_HEIGHT	(720)

#define NUM_VERTEX_QUADS		(4)	// 頂点数(四角形)

struct Float2
{
	float x, y, w;
};

struct Float3
{
	float x, y, z, w;
};

struct Float4
{
	float x, y, z, w;
};

struct Vec2 {
	float x, y;
};

struct Polygon_ {
	std::vector<Vec2> vertices;
	std::vector<int> faces;
};

struct Vector
{
	DirectX::XMFLOAT4 position;   // 位置
	DirectX::XMFLOAT4 color;      // 颜色
	DirectX::XMFLOAT2 texcoord;   // 纹理坐标
	DirectX::XMFLOAT3 normal;     // 法线
	float hasTexture;             // 是否使用贴图 (1.0f = 有, 0.0f = 无)
};


struct Vertex_GL {
	float position[4];   // location 0
	float normal[3];     // location 3
	float texcoord[2];   // location 2
	float color[4];      // location 1
	float hasTexture;    // location 4
};

// 頂点情報
struct VERTEX_3D
{
	Float3 Position;	// 座標
	Float4 Color;		// 色
	Float2 TexCoord;	// テクスチャ座標
};

Float2 MakeFloat2(float x, float y);

Float3 MakeFloat3(float x, float y, float z);

Float4 MakeFloat4(float x, float y, float z, float w);


// シーン遷移管理用
enum SCENE {
	SCENE_TITLE = 0,
	SCENE_GAME_0,
	SCENE_GAME_1,
	SCENE_TUTORIAL,
	SCENE_RESULT = 7,
	SCENE_MAX,
	SCENE_START
};


enum class AppMode
{
	Game,       // 游戏运行中（和发布版一样）
	Editor,     // 场景编辑器模式
};

extern int bgm;

// =========================================================
// シーン切り替え
// =========================================================
void SetScene(SCENE set);

SCENE CheckScene(void);

void ResetScene(void);


#endif // !MAIN_H