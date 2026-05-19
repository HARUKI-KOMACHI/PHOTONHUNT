#include "hpbar.h"
#include "texture.h"
#include "sprite.h"
#include "main.h"

// グローバル変数
Object_2D HPframe(0, 0, 300, 300);
Object_2D HPgauge(0, 0, 300, 300);

// テクスチャ番号
static int FrameTexNo1 = -1;
static int FrameTexNo2 = -1;
static int GaugeTexNo = -1;
PlayerState frameState = PlayerState::HUMAN;

// 初期化
void InitializeHPbar(void)
{
	FrameTexNo1 = LoadTexture("Assets/Textures/UI/HP_gauge_frame_01.png");
	FrameTexNo2 = LoadTexture("Assets/Textures/UI/HP_gauge_frame_02.png");
	GaugeTexNo = LoadTexture("Assets/Textures/UI/HP_gauge.png");

	// フレームの初期化
	HPframe.texNo = FrameTexNo1;
	HPframe.Z_Test = 10.0f;

	// ゲージの初期化
	HPgauge.texNo = GaugeTexNo;
	HPgauge.Z_Test = 10.1f;
}

// 終了処理
void FinalizeHPbar(void)
{
	UnloadTexture(FrameTexNo1);
	UnloadTexture(FrameTexNo2);
	UnloadTexture(GaugeTexNo);
}

// 更新
void UpdateHPbar(void)
{
	// HP割合をクランプ
	Player& player = GetPlayer();
	float percent = (float)player.hp/100.0f;
	if (percent < 0.0f) percent = 0.0f;
	if (percent > 1.0f) percent = 1.0f;

		

	HPgauge.Transform.Position.x = player.anim->obj.Transform.Position.x + HPGAUGE_POSITION_X;
	HPgauge.Transform.Position.y = player.anim->obj.Transform.Position.y + HPGAUGE_POSITION_Y;

	HPframe.Transform.Position.x = player.anim->obj.Transform.Position.x + HPGAUGE_POSITION_X;
	HPframe.Transform.Position.y = player.anim->obj.Transform.Position.y + HPGAUGE_POSITION_Y;

	BoxVertexsMake(&HPgauge);

	// UV座標を調整
	HPgauge.UV[0].x = 0.0f;      // 左上 - 固定
	HPgauge.UV[1].x = percent;   // 右上 - percentで調整
	HPgauge.UV[2].x = 0.0f;      // 左下 - 固定
	HPgauge.UV[3].x = percent;   // 右下 - percentで調整

	// ゲージのサイズを変更
	float currentWidth = HPGAUGE_WIDTH * percent;

	// 右側の頂点を左に移動
	HPgauge.Vertexs[1].x = HPgauge.Vertexs[0].x + currentWidth;  // 右上
	HPgauge.Vertexs[3].x = HPgauge.Vertexs[2].x + currentWidth;  // 右下

}

// 描画
void DrawHPbar(void)
{
	Player& player = GetPlayer();
	if (player.state == PlayerState::IDLE)return;

	DrawObject_2D(&HPgauge);

	BoxVertexsMake(&HPframe);
	DrawObject_2D(&HPframe);
}

void SetFrame(PlayerState state)
{
	if (state != PlayerState::HUMAN && state != PlayerState::SHADOW)return;
	if (frameState == state)return;

	if (state == PlayerState::HUMAN) {
		HPframe.texNo = FrameTexNo1;
		frameState = PlayerState::HUMAN;
	}
	
	if (state == PlayerState::SHADOW) {
		HPframe.texNo = FrameTexNo2;
		frameState = PlayerState::SHADOW;
	}
}