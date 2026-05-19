// =========================================================
// fade.cpp タイトルシーン背景制御
// 
// 制作者：32 chen		日付：2025
// =========================================================
#include "main.h"
#include "texture.h"
#include "sprite.h"
#include "fade.h"
#include "player_1.h"
#include "game_1.h"
#include "result.h"

// =========================================================
// グローバル変数
// =========================================================
//FADE fade;	// タイトルシーン背景の実体
unsigned int FadeTextureId;	// テクスチャID

Object_2D fade_2D(0.0f, 0.0f, SCREEN_WIDTH * 2.0f, SCREEN_HEIGHT*2.0f);
Object_2D::Vec2 playerPos = { 0.0f,0.0f };

STATE_FADE state;
bool use;
bool reset;

// =========================================================
// タイトルシーン背景初期化
// =========================================================
void InitializeFade(void)
{

	state = STATE_FADE_NONE;
	use = true;		// 使用フラグ
	reset = false;

	FadeTextureId=LoadTexture("rom/color/black.png");
	fade_2D.texNo = FadeTextureId;
	fade_2D.Z_Test = 100.0f;
	fade_2D.Color.A = 0.0f;
}

// =========================================================
// タイトルシーン背景更新
// =========================================================
void UpdateFade(void)
{
	if (use)
	{
		switch (state)
		{
		case STATE_FADE_NONE:

			break;
		case STATE_FADE_OUT:
			playerPos = GetPlayer().anim->obj.Transform.Position;
			fade_2D.Transform.Position = playerPos;
			fade_2D.Color.A+= 0.01f;
			if (fade_2D.Color.A > 1.0f )
			{
				state = STATE_FADE_IN;
				fade_2D.Color.A = 1.0f;

				if (reset) {
					Reset();
					reset = false;
					break;
				}

				switch (CheckScene())
				{

				case SCENE_TUTORIAL:
					ResetDead();
					SetScene(SCENE_GAME_1);
					break;
				case SCENE_GAME_1:
					SetScene(SCENE_RESULT);
					break;
				case SCENE_RESULT:
					SetScene(SCENE_TUTORIAL);
					break;
				default:break;
				}
			}
			break;
		case STATE_FADE_IN:
			fade_2D.Color.A -= 0.01f;
			if (fade_2D.Color.A < 0.0f)
			{
				state = STATE_FADE_NONE;
				fade_2D.Color.A = 0.0f;
			}
			break;
		default:break;
		}
	}
}

// =========================================================
// タイトルシーン背景描画
// =========================================================
void DrawFade(void)
{
	if (use)
	{
		BoxVertexsMake(&fade_2D);
		DrawObject_2D(&fade_2D);
	}
}

// =========================================================
// タイトルシーン背景終了処理
// =========================================================
void FinalizeFade(void)
{
	UnloadTexture(FadeTextureId);
}

// =========================================================
// フェードのスタート
// =========================================================
void StartFade()
{
	state = STATE_FADE_OUT;
	fade_2D.Color.A = 0.0f;
}

void ResetFade(void)
{
	state = STATE_FADE_OUT;
	if(!reset)fade_2D.Color.A = 0.0f;
	reset = true;
}