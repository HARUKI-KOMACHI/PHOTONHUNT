#pragma once
#include "player_1.h"

// 定数定義
#define HPGAUGE_POSITION_X  (-235.0f)
#define HPGAUGE_POSITION_Y  (155.0f)
#define HPGAUGE_WIDTH       (300.0f)

// 初期化・終了
void InitializeHPbar(void);
void FinalizeHPbar(void);

// 更新・描画
void UpdateHPbar(void);
void DrawHPbar(void);

void SetFrame(PlayerState);