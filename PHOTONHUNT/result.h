#pragma once

#include "sprite.h"

#define WIDTH	(5)
#define HEIGHT	(3)
#define WORD_INTERVAL	(130.0f)
#define TIMER_X	(200.0f)
#define TIMER_Y	(150.0f)
#define DEAD_X	(400.0f)
#define DEAD_Y	(-250.0f)
#define WORD_ROT	(0.05f)


class Result {
public:
	Object_2D obj;
	int num;
};

void InitializeResult();
void FinalizeResult();
void DrawResult();
void UpdateResult();

void AddDead();
void ResetDead();