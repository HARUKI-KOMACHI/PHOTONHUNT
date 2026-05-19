// sun.cpp 太陽システム（平行光）
#include "sun_1.h"
#define _USE_MATH_DEFINES
#include <math.h>

// グローバル変数
static SunSystem g_Sun;

// 太陽初期化
void InitializeSun01(void)
{
	g_Sun.currentDirection = SUN_NORTH;		// 開始時は北（上）
	g_Sun.rotationTimer = 0.0f;
	g_Sun.rotationInterval = 2.0f;			// 10秒で次の方向へ
	g_Sun.autoRotate = true;				// 自動回転ON
	g_Sun.directionChanged = true;			// 初回は影を生成
	g_Sun.sunVec = { 0.0f,0.0f };
}

// 太陽更新
void UpdateSun01(void)
{
	if (!g_Sun.autoRotate) return;

	// タイマーを進める
	g_Sun.rotationTimer += 1.0f / 60.0f;  // 60FPS想定
	if (g_Sun.rotationTimer >= 60.0f)g_Sun.rotationTimer = 0.0f;

	float theta = (2.0f * M_PI/60.0f) * g_Sun.rotationTimer;


	float x = sinf(theta);  // -1.0 ～ 1.0
	float y = cosf(theta);  // -1.0 ～ 1.0

	g_Sun.sunVec = { x,y };
}

// 太陽描画（デバッグ用）
void DrawSun01(void)
{
	// TODO: 太陽のアイコンやUIを表示する場合はここに実装
	// 現在は何も描画しない
}

// 太陽の方向ベクトルを取得
Object_2D::Vec2 GetSunVector(void)
{
	return g_Sun.sunVec;
}

// 太陽の方向を取得
SunDirection GetSunDirection(void)
{
	return g_Sun.currentDirection;
}

// 太陽の方向を設定
void SetSunDirection(SunDirection direction)
{
	if (g_Sun.currentDirection != direction)
	{
		g_Sun.currentDirection = direction;
		g_Sun.directionChanged = true;
		g_Sun.rotationTimer = 0.0f;
	}
}

// 影の更新が必要か判定
bool NeedShadowUpdate(void)
{
	return g_Sun.directionChanged;
}

// 影更新フラグをクリア
void ClearShadowUpdateFlag(void)
{
	g_Sun.directionChanged = false;
}

// 太陽システム取得
SunSystem& GetSunSystem(void)
{
	return g_Sun;
}