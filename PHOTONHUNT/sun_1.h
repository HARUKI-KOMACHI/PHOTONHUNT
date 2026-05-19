#pragma once
// sun.h 太陽システム（平行光）
#ifndef _SUN_H_
#define _SUN_H_

#include "sprite.h"

// 太陽の方向（8方向）
enum SunDirection
{
	SUN_NORTH,		// 北（上）
	SUN_NORTHEAST,	// 北東（右上）
	SUN_EAST,		// 東（右）
	SUN_SOUTHEAST,	// 南東（右下）
	SUN_SOUTH,		// 南（下）
	SUN_SOUTHWEST,	// 南西（左下）
	SUN_WEST,		// 西（左）
	SUN_NORTHWEST	// 北西（左上）
};
// 太陽システムクラス
class SunSystem
{
public:
	SunDirection currentDirection;	// 現在の方向
	float rotationTimer;			// 回転タイマー
	float rotationInterval;			// 方向切り替え間隔（秒）
	bool autoRotate;				// 自動回転するか
	bool directionChanged;			// 方向が変わったか（影更新フラグ）
	Object_2D::Vec2 sunVec;
};

// 関数宣言
void InitializeSun01(void);
void UpdateSun01(void);
void DrawSun01(void);

// 太陽の方向ベクトルを取得（正規化済み）
Object_2D::Vec2 GetSunVector(void);

// 太陽の方向を取得
SunDirection GetSunDirection(void);

// 太陽の方向を設定
void SetSunDirection(SunDirection direction);

// 影の更新が必要か判定
bool NeedShadowUpdate(void);

// 影更新フラグをクリア
void ClearShadowUpdateFlag(void);

// 太陽システム取得
SunSystem& GetSunSystem(void);

#endif