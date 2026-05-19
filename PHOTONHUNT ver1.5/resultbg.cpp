// =========================================================
// resultbg.cpp リザルトシーン背景制御
// 
// 制作者：32 chen		日付：2025
// =========================================================
#include "main.h"
#include "texture.h"
#include "sprite.h"
#include "resultbg.h"
#include "result.h"
#include "fade.h"

// =========================================================
// グローバル変数
// =========================================================

Object_2D resultbg_2D(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);
unsigned int ResultbgTextureId;	// テクスチャID

Object_2D::Vec2 Fraction_ResultbgPos = { 0,-SCREEN_HEIGHT / 2 + 200 };
int FractiontexResultbgtID = -1;
// =========================================================
// リザルトシーン背景初期化
// =========================================================
void InitializeResultbg(void)
{

	// テクスチャ読み込み
	ResultbgTextureId = LoadTexture("rom/Result.png");
	resultbg_2D.texNo = ResultbgTextureId;

}

// =========================================================
// リザルトシーン背景更新
// =========================================================
void UpdateResultbg(void)
{
	if (Win->kbd.KeyJustPressed('V')|| manager.JustPressed("A"))
	{
		//StartFade(SCENE_GAME);
		SetScene(SCENE_TITLE);

	}
}

// =========================================================
// リザルトシーン背景描画
// =========================================================
void DrawResultbg(void)
{
	
	BoxVertexsMake(&resultbg_2D);
	DrawObject_2D(&resultbg_2D);
}

// =========================================================
// リザルトシーン背景終了処理
// =========================================================
void FinalizeResultbg(void)
{
	UnloadTexture(ResultbgTextureId);
    UnloadTexture(FractiontexResultbgtID);
}