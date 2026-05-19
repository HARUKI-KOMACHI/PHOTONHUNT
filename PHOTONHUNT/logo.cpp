#include "sprite.h"
#include "logo.h"
#include "main.h"
#include "texture.h"
#include "player_1.h"

Object_2D logo(0, 0, 420, 48);
unsigned int logoTexNo = -1;
int state;
int idlestate;

void InitializeLogo() {
	logoTexNo = LoadTexture("assets/Textures/UI/logo.png");
	logo.texNo = logoTexNo;
	logo.Z_Test = 10.0f;
}
void FinalizeLogo() {
	UnloadTexture(logoTexNo);
}
void UpdateLogo() {
	Player& player = GetPlayer();
	state = player.state;
	idlestate = player.idleState;
	logo.Transform.Position.x = player.anim->obj.Transform.Position.x;
	logo.Transform.Position.y = player.anim->obj.Transform.Position.y + LOGO_OFFSET_Y;

	if (state == PlayerState::IDLE) {
		logo.Color.A += 0.01f;
		if (logo.Color.A >= 1.0f) {
			logo.Color.A = 1.0f;
		}
	}
	else {
		logo.Color.A -= 0.01f;
		if(logo.Color.A <= 0.0f)
		{
			logo.Color.A = 0.0f;
		}
	}
}
void DrawLogo() {
	if (state == PlayerState::IDLE) {
		BoxVertexsMake(&logo);
		DrawObject_2D(&logo);
	}
}