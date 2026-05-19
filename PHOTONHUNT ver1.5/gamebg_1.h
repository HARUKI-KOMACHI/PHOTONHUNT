#pragma once

#include "sprite.h"
#include "string.h"

class GameBG {
	public:
	Object_2D obj; // 背景オブジェクト
};

void InitializeGamebg01();
void UpdateGamebg01();
void DrawGamebg01();
void FinalizeGamebg01();

void AddBG(const std::string& textureFile, const std::string& objFile);
GameBG& GetGameBG();