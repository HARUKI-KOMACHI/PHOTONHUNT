#include "attackUI.h"
#include "sprite.h"
#include "texture.h"
#include "player_1.h"

Object_2D attackUI(0, 0, 75, 75);

void InitializeAttackUI() {
	attackUI.texNo = LoadTexture("Assets/Textures/UI/Attack.png");
	attackUI.Z_Test = 10.0f;
}
void FinalizeAttackUI() {

}
void UpdateAttackUI() {
	Player& player = GetPlayer();
	attackUI.Transform.Position.x = player.anim->obj.Transform.Position.x + OFFSET_X;
	attackUI.Transform.Position.y = player.anim->obj.Transform.Position.y + OFFSET_Y;
}
void DrawAttackUI() {
	Player& player = GetPlayer();
	if (player.canAttack == true) {
		BoxVertexsMake(&attackUI);
		DrawObject_2D(&attackUI);
	}
}