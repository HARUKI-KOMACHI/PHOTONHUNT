#pragma once
#include "sprite.h"
class Goal {
public:
	Object_2D obj;
	bool isOpen;
};

extern Goal g_goal;

void InitializeGoal();
void FinalizeGoal();
void UpdateGoal();
void DrawGoal();

void AddGoal(float x, float y, float w, float h, const std::string& objFile);