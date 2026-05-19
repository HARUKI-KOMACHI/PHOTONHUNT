#include "tutorialUI.h"
#include "sprite.h"
#include "texture.h"
#include "player_1.h"
#include "main.h"
#include "goal.h"

Object_2D Text(0, 0, 300, 160);

unsigned int moveText;
unsigned int shadowText;
unsigned int attackText;
unsigned int endText;

float idleTimer;
float idleDuration;

void InitializetutorialUI() {
	moveText = LoadTexture("Assets/Textures/UI/moveText.png");
	shadowText = LoadTexture("Assets/Textures/UI/shadowText.png");
	attackText = LoadTexture("Assets/Textures/UI/attackText.png");
	endText = LoadTexture("Assets/Textures/UI/endText.png");
	Text.Z_Test = 10.0f;
	Text.Color.A = 0.0f;
	idleDuration = 2.0f;
}
void FinalizetutorialUI() {

}
void UpdatetutorialUI() {
	Player& player = GetPlayer();
	Text.Transform.Position.x = player.anim->obj.Transform.Position.x + OFFSET_X;
	Text.Transform.Position.y = player.anim->obj.Transform.Position.y + OFFSET_Y;

	if (!g_goal.isOpen) {
		if (player.anim->obj.Transform.Position.x < -100.0f) {
			if (player.anim->obj.Transform.Position.y > 100.0f) {
				Text.texNo = moveText;
			}
			else {
				Text.texNo = shadowText;
			}
		}
		else {
			Text.texNo = attackText;
		}
	}
	else {
		Text.texNo = endText;
	}

	if (!Win->kbd.AnyKeyIsPressed() && !manager.IsAnyInput()&&
		player.state==HUMAN) {
		idleTimer += 1.0f / 60.0f;
		if (idleTimer >= idleDuration) {
			Text.Color.A += 1.0f / 30.0f;
			if (Text.Color.A >= 1.0f)Text.Color.A = 1.0f;
		}
		else {
			Text.Color.A -= 1.0f / 30.0f;
			if (Text.Color.A <= 0.0f)Text.Color.A = 0.0f;
		}
	}
	else {
		Text.Color.A -= 1.0f / 30.0f;
		if (Text.Color.A <= 0.0f)Text.Color.A = 0.0f;
		idleTimer = 0.0f;
	}
}
void DrawtutorialUI() {
	BoxVertexsMake(&Text);
	DrawObject_2D(&Text);
}