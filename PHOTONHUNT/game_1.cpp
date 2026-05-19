#include "main.h"
#include "game_1.h"

#include "player_1.h"
#include"enemy_1.h"
#include "wall_1.h"
#include "sun_1.h"
#include "shadow_1.h"
#include "gamebg_1.h"
#include "gridmap.h"
#include "ingameUI.h"
#include "Audio.h"
#include "goal.h"
#include "collision.h"
#include "fade.h"


static float colA = 1.0f;
//GAMESCENE gamescene = START;
// 全局或静态对象
static ChiliTimer g_timer;
float clearTime;

void InitializeGame_1(void) {
	g_timer = ChiliTimer();
	clearTime = 0.0f;
	colA = 1.0f;
	InitializePlayer01();
	InitializeEnemy01();
	InitializeWall01();
	InitializeSun01();
	InitializeShadow01();
	InitializeGamebg01();
	InitializeIngameUI();
	InitializeGoal();

	// ── グリッドマップの初期化 ─────────────────────────────
	// ※ ここの値はステージの大きさに合わせて変更してください
	//
	//   originX, originY : ステージ左上のワールド座標
	//   width            : 横セル数 = ステージ横幅 / GRID_CELL_SIZE
	//   height           : 縦セル数 = ステージ縦幅 / GRID_CELL_SIZE
	//
	// 例：ステージが (-800, -600) から (800, 600) の場合
	//   originX = -800, originY = -600
	//   width   = 1600 / 20 = 80
	//   height  = 1200 / 20 = 60
	InitializeGridMap(-SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, SCREEN_WIDTH / GRID_CELL_SIZE, SCREEN_HEIGHT / GRID_CELL_SIZE);

	SetPlayerPos({ -600.0f,-400.0f });

	std::vector<MovePoint> path1 = { {0,-400}, {500,-400} };
	std::vector<MovePoint> path2 = { {150,50}, {500,50},{500,-200},{500,50} };
	std::vector<MovePoint> path3 = { {-450,400}, {-450,0} };
	std::vector<MovePoint> path4 = { {-100.0f,370.0f},{300.0f,370.0f} };
	std::vector<MovePoint> path5 = { {-150.0f,150.0f},{-150.0f,-100.0f} };
	AddEnemy("suraimu1.an", path1);
	AddEnemy("suraimu2.an", path2);
	AddEnemy("suraimu3.an", path3);
	AddEnemy("suraimu4.an", path4);
	AddEnemy("suraimu5.an", path5);

	AddWall(-500, 290, 400, 400, "Assets/Textures/objects/1-1_object_01.png", "Assets/Collision/1-1_object_01.obj", 60.0f);
	AddWall(-210, 350, 450, 450, "Assets/Textures/objects/1-1_object_02.png", "Assets/Collision/1-1_object_02.obj", 60.0f);
	AddWall(-70, 340, 100, 100, "Assets/Textures/objects/1-1_object_03.png","Assets/Collision/1-1_object_03.obj",40.0f);
	AddWall(300, 450, 150, 150, "Assets/Textures/objects/1-1_object_04.png", "Assets/Collision/1-1_object_04.obj");
	AddWall(-240, 120, 100, 100, "Assets/Textures/objects/1-1_object_05.png", "Assets/Collision/1-1_object_05.obj",40.0f,-0.1f);
	AddWall(0, 50, 200, 200, "Assets/Textures/objects/1-1_object_06.png", "Assets/Collision/1-1_object_06.obj",60.0f);
	AddWall(-240, -110, 300, 300, "Assets/Textures/objects/1-1_object_07.png", "Assets/Collision/1-1_object_07.obj",50.0f,-0.3f);
	AddWall(-550, -80, 150, 150, "Assets/Textures/objects/1-1_object_08.png", "Assets/Collision/1-1_object_08.obj",60.0f);
	AddWall(-530, -280, 120, 120, "Assets/Textures/objects/1-1_object_09.png", "Assets/Collision/1-1_object_09.obj");
	AddWall(-420, -420, 350, 350, "Assets/Textures/objects/1-1_object_10.png", "Assets/Collision/1-1_object_10.obj",70.0f);
	AddWall(-100, -350, 70, 70, "Assets/Textures/objects/1-1_object_11.png", "Assets/Collision/1-1_object_11.obj",30.0f);
	AddWall(200, -150, 300, 300, "Assets/Textures/objects/1-1_object_12.png", "Assets/Collision/1-1_object_12.obj",90.0f);
	AddWall(600, -250, 350, 350, "Assets/Textures/objects/1-1_object_13.png", "Assets/Collision/1-1_object_13.obj",20.0f);

	AddBG("Assets/Textures/objects/1stage.png", "Assets/Collision/1stage.obj");

	AddGoal(-450.0f, 450.0f, 92.0f, 92.0f, "Assets/Collision/goal.obj");

	BuildGridMapFromWalls();
	BuildGridMapFromBG();

	GenerateDynamicShadows();

	WinCamera.Scale = 2.5f;
}
void UpdateGame_1(void) {
	UpdateSun01();
	UpdatePlayer01();
	UpdateEnemy01();
	UpdateGamebg01();
	UpdateWall01();
	UpdateGoal();
	UpdateIngameUI();

	Player& player = GetPlayer();
	WinCamera.pos.x = player.anim->obj.Transform.Position.x * WinCamera.Scale;
	WinCamera.pos.y = player.anim->obj.Transform.Position.y * WinCamera.Scale;

	if (g_goal.isOpen) {
		bool goalCollision = CheckCollisionMeshVsMesh(player.anim->obj, g_goal.obj);
		if (goalCollision) {
			clearTime = g_timer.Peek();
			StartFade();
		}
	}

	GenerateDynamicShadows();

	Win->kbd.UpdateState();
}
void DrawGame_1(void) {
	DrawShadow01();
	DrawPlayer01();
	DrawEnemy01();	
	DrawGamebg01();
	DrawWall01();
	DrawIngameUI();
	DrawGoal();
}
void FinalizeGame_1(void) {
	FinalizePlayer01();
	FinalizeEnemy01();
	FinalizeShadow01();
	FinalizeIngameUI();
	FinalizeGamebg01();
	FinalizeGoal();
}

void Reset() {
	ResetPlayer();
	ResetEnemy();
}