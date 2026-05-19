#pragma once
// =========================================================
// game.h ゲームシーン制御
// 
// 制作者:		日付：
// =========================================================
#ifndef _GAME1_H_
#define _GAME1_H_

// =========================================================
// マクロ宣言
// =========================================================

void InitializeGame_1(void);
void UpdateGame_1(void);
void DrawGame_1(void);
void FinalizeGame_1(void);
void Reset();

extern float clearTime;

//float* GetGamecloA(void);
//
//GAMESCENE GetGameScene(void);
//
//int GetFraction(void);
#endif