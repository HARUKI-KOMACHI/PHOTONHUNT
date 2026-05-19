#include "shadowgauge.h"
#include "sprite.h"
#include "texture.h"
#include "player_1.h"

Object_2D shadowGauge(0, 0, 40, 40);
Object_2D shadowFrame(0, 0, 40, 40);

int playerState = 0;
bool isShadowGaugeActive = false;

unsigned int shadowGaugeTexNo = -1;
unsigned int shadowFrameTexNo = -1;

void InitializeShadowGauge(void){
	shadowGaugeTexNo = LoadTexture("Assets/Textures/UI/gauge_in.png");
	shadowFrameTexNo = LoadTexture("Assets/Textures/UI/gauge_out.png");

	//ゲージの初期化
	shadowGauge.texNo = shadowGaugeTexNo;
	shadowGauge.Z_Test = 10.1f;

	//フレームの初期化
	shadowFrame.texNo = shadowFrameTexNo;
	shadowFrame.Z_Test = 10.0f;
}

void FinalizeShadowGauge() {
	UnloadTexture(shadowGaugeTexNo);
	UnloadTexture(shadowFrameTexNo);
}

void UpdateShadowGauge() {
	Player& player = GetPlayer();
	float percent = player.shadowGauge / player.shadowGaugeMax;
	if (percent < 0.0f) percent = 0.0f;
	if (percent > 1.0f) percent = 1.0f;

	shadowGauge.Transform.Position.x = player.anim->obj.Transform.Position.x + SHADOWGAUGE_POSITION_X;
	shadowGauge.Transform.Position.y = player.anim->obj.Transform.Position.y + SHADOWGAUGE_POSITION_Y;
	shadowFrame.Transform.Position.x = player.anim->obj.Transform.Position.x + SHADOWGAUGE_POSITION_X;
	shadowFrame.Transform.Position.y = player.anim->obj.Transform.Position.y + SHADOWGAUGE_POSITION_Y;

	BoxVertexsMake(&shadowGauge);

	// UV座標を調整
	shadowGauge.UV[0].y = percent;      // 左上 - percentで調整
	shadowGauge.UV[1].y = percent;   // 右上 - percentで調整
	shadowGauge.UV[2].y = 0.0f;   // 右下 - 固定
	shadowGauge.UV[3].y = 0.0f;   // 左下 - 固定

	float currentHeight = SHADOWGAUGE_HEIGHT * percent;

	shadowGauge.Vertexs[0].y = shadowGauge.Vertexs[2].y + currentHeight; // 左上
	shadowGauge.Vertexs[1].y = shadowGauge.Vertexs[3].y + currentHeight; // 右上

	if (player.shadowGauge != player.shadowGaugeMax) {
		isShadowGaugeActive = true;
	}
	else {
		isShadowGaugeActive = false;
	}

	playerState = player.state;
}

void DrawShadowGauge() {
	if (!isShadowGaugeActive) return;

	if (playerState == IDLE) return;

	DrawObject_2D(&shadowGauge);

	BoxVertexsMake(&shadowFrame);
	DrawObject_2D(&shadowFrame);
}