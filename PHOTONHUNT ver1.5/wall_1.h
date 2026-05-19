#pragma once
// wall.h 壁・障害物システム
#ifndef _WALL_H_
#define _WALL_H_

#include "sprite.h"
#include <vector>
#include <string>

// 壁クラス
class Wall
{
public:
	Object_2D obj;				// 描画・当たり判定用オブジェクト
	bool selected;				// 選択されているか
	bool createsShadow;			// 影を生成するか
	bool usePolygonCollision;	// ポリゴン当たり判定を使用するか
	float shadowFadeX;
	float shadowFadeY;

	float shadowHeight;					// 壁の高さ（影の長さに影響）
};

// エディターモード
enum EditorMode
{
	MODE_PLAY,		// プレイモード（通常のゲーム）
	MODE_EDIT		// エディットモード（壁配置）
};

// 関数宣言
void InitializeWall01(void);
void UpdateWall01(void);
void DrawWall01(void);
void FinalizeWall01(void);

// エディター機能
void SetEditorMode(EditorMode mode);
EditorMode GetEditorMode(void);
void UpdateWallEditor(void);

// 壁の追加・削除
void AddWall(float x, float y, float w, float h
	, const std::string& textureFile = "", const std::string& objFile = ""
	, float wallHeight = 50.0f, float shadowX = -FLT_MAX, float shadowY = FLT_MAX);
void RemoveSelectedWall(void);

// 保存・読み込み
void SaveWalls(const std::string& filename);
void LoadWalls(const std::string& filename);

// 取得
std::vector<Wall>& GetWalls(void);

#endif