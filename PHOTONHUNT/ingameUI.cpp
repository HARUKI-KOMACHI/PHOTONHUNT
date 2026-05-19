
#include "ingameUI.h"
#include "hpbar.h"
#include "shadowgauge.h"	
#include "logo.h"
#include "attackUI.h"
#include "tutorialUI.h"
#include "main.h"

void InitializeIngameUI(void) {
	InitializeHPbar();
	InitializeShadowGauge();
	InitializeLogo();
	InitializeAttackUI();
	InitializetutorialUI();
}
void UpdateIngameUI(void) {
	if (CheckScene() == SCENE_TUTORIAL) {
		UpdatetutorialUI();
	}
	UpdateHPbar();
	UpdateShadowGauge();
	UpdateLogo();
	UpdateAttackUI();
	
}
void DrawIngameUI(void) {
	if (CheckScene() == SCENE_TUTORIAL) {
		DrawtutorialUI();
	}
	DrawHPbar();
	DrawShadowGauge();
	DrawLogo();
	DrawAttackUI();
}
void FinalizeIngameUI(void) {
    FinalizeHPbar();    
	FinalizeShadowGauge();
	FinalizeLogo();
	FinalizeAttackUI();
	FinalizetutorialUI();
}