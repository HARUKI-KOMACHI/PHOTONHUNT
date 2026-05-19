#pragma once
// =========================================================
// fade.h タイトルシーン背景制御
// 
// 制作者：32 chen		日付：2025
// =========================================================
#ifndef _Fade_H_
#define _Fade_H_
void InitializeFade(void);
void UpdateFade(void);
void DrawFade(void);
void FinalizeFade(void);
void StartFade();
void ResetFade(void);

// =========================================================
// 構造体宣言
// =========================================================
enum STATE_FADE
{
	STATE_FADE_NONE = 0,
	STATE_FADE_OUT,
	STATE_FADE_IN,
	STATE_FADE_MAX
};

// =========================================================
// 構造体宣言
// =========================================================
struct FADE {
	Float2 pos;		// 座標
	Float2 size;	// サイズ
	Float4 color;
	STATE_FADE state;
	bool use;		// 使用フラグ
};

extern STATE_FADE state;

#endif