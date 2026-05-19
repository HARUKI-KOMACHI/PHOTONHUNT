#pragma once
#include "sprite.h"
#include <vector>

// 光源
class LightSource
{
public:
	float x;				// 位置X
	float y;				// 位置Y
	float radius;			// 光の半径
	bool active;			// 有効フラグ
};

// 影エリア
class ShadowArea
{
public:
	Object_2D area;			// 影の範囲（矩形）
	bool active;			// 有効フラグ
};

// 動的影（壁から生成される影）
class DynamicShadow
{
public:
	Object_2D shadowObj;	// 影のポリゴン
	bool active;			// 有効フラグ
};

// 関数宣言
void InitializeShadow01(void);
void UpdateShadow01(void);
void DrawShadow01(void);
void FinalizeShadow01(void);

// 光源・影の追加
void AddLight(float x, float y, float radius);
void AddShadowArea(float x, float y, float w, float h);

// 動的影の生成（壁から）
void GenerateDynamicShadows(void);

// 判定
bool CheckPlayerInShadow(void);

// 取得
std::vector<LightSource>& GetLights(void);
std::vector<ShadowArea>& GetShadows(void);