#include "main.h"
#include "startmain.h"
#include "sprite.h"
#include "texture.h"
#include "fade.h"

// =========================================================
// ƒQ[ƒ€ƒV[ƒ“‰Šú‰»
// =========================================================

Object_2D objdg(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT);

int text[2] = {};
Object_2D* textobj[3] = {};
int num = 0;
bool key = true;
static float colA = 1.0f;
void InitializeStartMain(void)
{
	colA = 1.0f;
	text[0] = LoadTexture("rom:/voiren.png");
	text[1] = LoadTexture("rom:/hal.png");

	textobj[0] = new Object_2D(0.0f, 0.0f, 500, 140);
	textobj[0]->texNo = text[0];
	textobj[0]->Color.A = 0;

	textobj[1] = new Object_2D(0.0f, 0.0f, 320, 140);
	textobj[1]->texNo = text[1];
	textobj[1]->Color.A = 0;
}

void UpdateStartMain(void)
{

	
	if (key)
	{
		if (textobj[num]->Color.A <= 1)
		{
			textobj[num]->Color.A += 0.01f;
		}
		else
		{
			key = false;
		}
		
	}
	else
	{

		if (textobj[num]->Color.A >= 0)
		{
			textobj[num]->Color.A -= 0.01f;
		}
		else
		{
			num++;
			key = true;
		}
	}

	if (Win->kbd.KeyJustPressed('V')|| manager.JustPressed("A"))
	{
		num++;
	}

	if (num >= 2)
	{
		SetScene(SCENE_TITLE);
	}
}

void DrawStartMain(void)
{
	BoxVertexsMake(textobj[num]);
	DrawObject_2D(textobj[num]);
}

void FinalizeStartMain(void)
{
	for (int i = 0;i < 2;i++)
	{
		delete textobj[i];

		if (text[i] != -1)
		{
			UnloadTexture(text[i]);
		}
	}
}

float* GetStartMaincloA(void)
{
	return &colA;
}

