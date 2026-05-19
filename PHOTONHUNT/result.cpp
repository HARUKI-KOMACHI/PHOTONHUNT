#include "result.h"
#include "player_1.h"
#include "main.h"
#include "texture.h"
#include "game_1.h"
#include "fade.h"
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

Object_2D result(0, 0, 1980, 1080);
std::vector<Result> timer;
std::vector<Result> dead;

int deadCount = 0;

unsigned int numberTexID = -1;

void InitializeResult() {
	WinCamera.Scale = 1.0f;
	WinCamera.pos = { 0,0 };

	numberTexID = LoadTexture("Assets/Textures/UI/number.png");

	result.texNo = LoadTexture("Assets/Textures/result.png");

	float s = sinf(WORD_ROT * M_PI);
	float c = cosf(WORD_ROT * M_PI);

	int minutes = (int)clearTime / 60;
	int seconds = (int)clearTime % 60;
	int ms = (int)(clearTime * 10) % 10;

	// ms（小数第一位）小さいサイズでpush
	Result Mpush;
	Mpush.obj.texNo = numberTexID;
	Mpush.num = ms;
	Mpush.obj.Size = { 75.0f, 75.0f }; // 小さいサイズ
	timer.push_back(Mpush);

	// seconds（2桁固定）
	int tempSeconds = seconds;
	for (int i = 0; i < 2; i++) {
		Result Spush;
		Spush.obj.texNo = numberTexID;
		Spush.num = tempSeconds % 10;
		Spush.obj.Size = { 150.0f, 150.0f };
		timer.push_back(Spush);
		tempSeconds /= 10;
	}

	// : セパレーター
	Result Cpush;
	Cpush.obj.texNo = numberTexID;
	Cpush.num = 11;
	Cpush.obj.Size = { 150.0f, 150.0f };
	timer.push_back(Cpush);

	// minutes（可変桁）
	int tempMinutes = minutes;
	 do{
		Result MPush;
		MPush.obj.texNo = numberTexID;
		MPush.num = tempMinutes % 10;
		MPush.obj.Size = { 150.0f, 150.0f };
		timer.push_back(MPush);
		tempMinutes /= 10;
	 } while (tempMinutes > 0);

	std::reverse(timer.begin(),timer.end());

	for (int i = 0; i < timer.size(); i++) {
		if (timer[i].num == 0)timer[i].num = 9;
		else timer[i].num--;

		Object_2D_UV(&timer[i].obj, timer[i].num, WIDTH, HEIGHT);

		if (i == timer.size() - 1) {
			timer[i].obj.Transform.Position.x = TIMER_X + (WORD_INTERVAL * i * c) * 0.9f;
			timer[i].obj.Transform.Position.y = TIMER_Y + WORD_INTERVAL * 0.4f * i * s;
		}
		else if (timer[i].num == 0) {
			timer[i].obj.Transform.Position.x = TIMER_X + (WORD_INTERVAL * i * c) * 0.8f;
			timer[i].obj.Transform.Position.y = TIMER_Y + WORD_INTERVAL * 0.5f * i * s;
		}
		else if (timer[i].num == 10) {
			timer[i].obj.Transform.Position.x = TIMER_X + (WORD_INTERVAL * i * c) * 0.6f;
			timer[i].obj.Transform.Position.y = TIMER_Y + WORD_INTERVAL * 0.5f * i * s;
		}
		else {
			timer[i].obj.Transform.Position.x = TIMER_X + WORD_INTERVAL * i * c;
			timer[i].obj.Transform.Position.y = TIMER_Y + WORD_INTERVAL * 0.75f * i * s;
		}
		timer[i].obj.Transform.Rotation = WORD_ROT * M_PI;
	}

	do {
		Result Dpush;
		Dpush.obj.texNo = numberTexID;
		Dpush.num = deadCount % 10;

		Dpush.obj.Size = { 150.0f,150.0f };
		Dpush.obj.Z_Test = 1.0f;
		dead.push_back(Dpush);
		deadCount /= 10;
	} while (deadCount > 0);

	std::reverse(dead.begin(), dead.end());

	for (int i = 0; i < dead.size(); i++) {
		if (dead[i].num == 0)dead[i].num = 9;
		else dead[i].num--;

		Object_2D_UV(&dead[i].obj, dead[i].num, WIDTH, HEIGHT);

		if (dead[i].num == 0) {
			dead[i].obj.Transform.Position.x = DEAD_X + (WORD_INTERVAL * i * c) * 0.8f;
			dead[i].obj.Transform.Position.y = DEAD_Y + WORD_INTERVAL * 0.5f * i * s;
			dead[i].obj.Transform.Rotation = WORD_ROT * M_PI;
		}
		else{
			dead[i].obj.Transform.Position.x = DEAD_X + WORD_INTERVAL * i * c;
			dead[i].obj.Transform.Position.y = DEAD_Y + WORD_INTERVAL * 0.75f * i * s;
			dead[i].obj.Transform.Rotation = WORD_ROT * M_PI;
		}
		
	}
}

void FinalizeResult() {
	timer.clear();
	dead.clear();
}

void UpdateResult() {
	if (Win->kbd.KeyIsPressed('E') || manager.JustPressed("B")) {
		StartFade();
	}
}

void DrawResult() {
	BoxVertexsMake(&result);
	DrawObject_2D(&result);

	for (auto& t : timer) {
		BoxVertexsMake(&t.obj);
		DrawObject_2D(&t.obj);
	}

	for (auto& d : dead) {
		BoxVertexsMake(&d.obj);
		DrawObject_2D(&d.obj);
	}
}

void AddDead() {
	deadCount++;
}

void ResetDead() {
	deadCount = 0;
}
