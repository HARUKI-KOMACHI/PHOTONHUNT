#include "tutorial.h"
#include "main.h"
#include "game_1.h"
#include "player_1.h"
#include"enemy_1.h"
#include "wall_1.h"
#include "sun_1.h"
#include "shadow_1.h"
#include "gamebg_1.h"
#include "ingameUI.h"
#include "Audio.h"
#include "goal.h"
#include "collision.h"
#include "fade.h"

GameBG tutorialFloor;

bool goalCollision = false;

void InitializeTutorial() {
	InitializePlayer01();
	InitializeEnemy01();
	InitializeWall01();
	InitializeSun01();
	InitializeShadow01();
	InitializeIngameUI();
	InitializeGamebg01();
	InitializeGoal();

	SetPlayerPos({ -550.0f, 500.0f });
	std::vector<MovePoint> path1 = { {150, 0}, {450,0},{450,-250}, {150,-250} };


	AddBG("Assets/Textures/objects/tutorial_stage.png", "Assets/Collision/tutorial_stage.obj");
	AddGoal(-350.0f, 500.0f, 128.0f, 128.0f, "Assets/Collision/goal.obj");

	AddWall(-420.0f, -100.0f, 200.0f, 200.0f, "Assets/Textures/objects/1-1_object_06.png", "Assets/Collision/1-1_object_06.obj", 60.0f);
	AddWall(300.0f, -100.0f, 200.0f, 200.0f, "Assets/Textures/objects/1-1_object_05.png", "Assets/Collision/1-1_object_05.obj", 80.0f);
	AddEnemy("suraimu1.an", path1);

	GenerateDynamicShadows();
	WinCamera.Scale = 2.5f;
}
void FinalizeTutorial() {
	FinalizePlayer01();
	FinalizeEnemy01();
	FinalizeShadow01();
	FinalizeIngameUI();
	FinalizeGamebg01();
	FinalizeGoal();
}
void UpdateTutorial() {
	UpdateSun01();
	UpdatePlayer01();
	UpdateEnemy01();
	UpdateGamebg01();
	UpdateWall01();
	UpdateGoal();
	UpdateIngameUI();

	GenerateDynamicShadows();

	if (Win->kbd.KeyJustPressed('^')) {
		StartFade();
	}

	Player& player = GetPlayer();
	WinCamera.pos.x = player.anim->obj.Transform.Position.x * WinCamera.Scale;
	WinCamera.pos.y = player.anim->obj.Transform.Position.y * WinCamera.Scale;

	if (g_goal.isOpen){
		goalCollision = CheckCollisionMeshVsMesh(player.anim->obj, g_goal.obj);
	}
	else {
		goalCollision = false;
	}

	if (player.state == ATTACK) g_goal.isOpen = true;
	if (goalCollision)
	{
		StartFade();
	}

	Win->kbd.UpdateState();
}

void DrawTutorial() {
	DrawShadow01();
	DrawPlayer01();
	DrawEnemy01();
	DrawGamebg01();
	DrawWall01();
	DrawIngameUI();
	DrawGoal();
}