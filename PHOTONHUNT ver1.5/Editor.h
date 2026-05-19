// =========================================================
// editor.h プレイヤー制御
// 
// 制作者:		日付：
// =========================================================
#ifndef _EDITOR_H_
#define _EDITOR_H_
#include <string>
#include <vector>
#include "sprite.h"
// =========================================================
// 構造体宣言
// =========================================================
class animation
{
public:
	std::string name;
	std::vector<int> animation_id;
	Object_2D::Vec2 size;
	float IDnum = 0;
};

class animation_Object_2D
{
public:
	std::string name;
	Object_2D obj;
	std::vector<animation> list;
	int animationID = 0;
	bool animPlaying = true;
	float speed = 0.5f;
};

void SaveObjectParamBinary(const std::string& dir, const animation_Object_2D& data);
bool LoadObjectParamBinary(const std::string& dir, animation_Object_2D& data);
void LoadAllObjectParams();


void InitializeEditor(void);
void UpdateEditor(void);
void DrawEditor(void);
void FinalizeEditor(void);

std::vector<animation_Object_2D>& Getobj_an(void);
Object_2D& GetMouse(void);

bool IsAnimationFinished(animation_Object_2D& obj);


//ヘルパー関数
animation_Object_2D* FindObjectByName(const std::string& name);
bool SetAnimation(animation_Object_2D& obj, const std::string& animName);
#endif