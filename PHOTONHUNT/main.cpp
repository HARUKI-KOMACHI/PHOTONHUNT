

#include "main.h"
#include "texture.h"
#include "sprite.h"

#include <iomanip>
#include "ChiliTimer/ChiliTimer.h"
#include "fade.h"
#include "result.h"
#include "StartMain.h"
#include "Audio.h"
#include "Window.h"
#include "Editor.h"
#include "game_1.h"
#include "tutorial.h"
#include "goal.h"

// =========================================================
// プロトタイプ宣言
// =========================================================
void Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
void Update(void);
void Draw(void);
void Finalize(void);

SCENE scene;

AppMode APPmode;

std::unique_ptr<Window> Win;

#ifdef _DEBUG
int g_CountFPS;
char g_DebugStr[2048];

#endif // _DEBUG

#pragma comment(lib,"winmm.lib")

int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 初期化
	Initialize(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	float targetFps = 60.0f;
	float minFrameTime = (targetFps > 0.0f) ? 1.0f / targetFps : 0.0f;

	float accumulatedTime = 0.0f;

	/*while (GetMessage(&msg, NULL, 0, 0))*/
	do
	{
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Msg);
			DispatchMessage(&Msg);
		}
		else
		{
			float dt = Win->Gfx().timer.Mark(); // 現在のフレームの所要時間（秒）
			accumulatedTime += dt;
			// ✅ フレーム間隔が十分であれば、描画を1回実行する
			if (targetFps == 0.0f || accumulatedTime >= minFrameTime)
			{
				float fps = 1.0f / accumulatedTime;

				accumulatedTime = 0.0f;

				static float smoothedFps = 60.0f;
				smoothedFps = smoothedFps * 0.9f + fps * 0.1f; // 徐々に新しい値に近づき、急激な変動を避ける
				//smoothedFps = fps * 0.1f; // 徐々に新しい値に近づき、急激な変動を避ける
				std::wostringstream oss;

				oss << L"FPS: " << std::fixed << std::setprecision(1) << smoothedFps;

				Win->SetTitle(oss.str());
				// ✔ 入力処理、更新ロジック、描画
				
				// 描画
					// 更新
				Update();
				Draw();
			}
		}
	} while (Msg.message != WM_QUIT);

	// 終了処理
	Finalize();

	return (int)Msg.wParam;
}


int bgm = -1;
// =========================================================
// 初期化
// =========================================================
void Initialize(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// システム系初期化
	//InitSystem(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Win = std::make_unique<Window>(SCREEN_WIDTH, SCREEN_HEIGHT, L"The Donkey Fart Box");

	InitializeFade();
	InitAudio();
	InitinputLayouts();
	inti_spine();
	Win->Gfx().Init_3D();
	manager.run();

	APPmode = AppMode::Game;
	scene = SCENE_TUTORIAL;
	//scene = SCENE_GAME_1;
	//scene = SCENE_RESULT;

	// オブジェクト初期化
	InitializeEditor();

	// 音声関係初期化
	bgm = LoadAudio(L"Assets/Sounds/bgm/game_01.wav");
	PlayAudio(bgm, true);

	switch (scene)
	{
	case SCENE_GAME_1:
		InitializeGame_1();
		break;
	case SCENE_TUTORIAL:
		InitializeTutorial();
		break;
	case SCENE_RESULT:
		InitializeResult();
		break;
	case SCENE_START:
		InitializeStartMain();
		break;
	default:break;
	}
}

// =========================================================
// 更新
// =========================================================
void Update(void)
{
	// システム系更新
	UpdateFade();
	if (state == STATE_FADE_OUT)return;
	
	switch (scene)
	{
	case SCENE_GAME_1:
		UpdateGame_1();
		break;
	case SCENE_TUTORIAL:
		UpdateTutorial();
		break;
	case SCENE_RESULT:
		UpdateResult();
		break;
	case SCENE_START:
		UpdateStartMain();
		break;
	default:break;
	}

	UpdateEditor();

	{
		ImGuiIO& io = ImGui::GetIO();

		bool tabPressed = Win->kbd.KeyJustPressed(VK_TAB)
			|| ImGui::IsKeyPressed(ImGuiKey_Tab, false);

		static bool prevDown = false;
		static bool down = false;

		if (Win->kbd.KeyJustPressed(VK_TAB)
			|| ImGui::IsKeyPressed(ImGuiKey_Tab, false))
		{
			if (down)
			{
				prevDown = true;
			}

		}
		else if (!Win->kbd.KeyJustPressed(VK_TAB)
			&& !ImGui::IsKeyPressed(ImGuiKey_Tab, false))
		{
			down = true;
		}
		bool justPressed = (down && !prevDown);      // 当前按下 & 上一帧没按下

		if (prevDown)
		{
			APPmode = (APPmode == AppMode::Game)
				? AppMode::Editor
				: AppMode::Game;
			prevDown = false;
			down = false;
		}
	}

	
}

// =========================================================
// 描画
// =========================================================
void Draw(void)
{
	Win->Gfx().ClearBuffer(0.5f, 0.5f, 1.0f);
	//Win->pGfx_GL->ClearBuffer(0.5f, 0.5f, 1.0f);
	//Win->Gfx().depth();

	ImGui_ImplDX11_NewFrame();
	//ImGui_ImplOpenGL3_NewFrame();  // ✔
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
	//ImGuizmo::BeginFrame();


	// オブジェクト描画
	switch (scene)
	{
	case SCENE_GAME_1:
		DrawGame_1();
		break;
	case SCENE_TUTORIAL:
		DrawTutorial();
		break;
	case SCENE_RESULT:
		DrawResult();
		break;
	case SCENE_START:
		DrawStartMain();
		break;
	default:break;
	}

	if (APPmode == AppMode::Editor)
	{
		//DrawEditor();
		//DrawUi();
	}

	DrawFade();
	//Win->pGfx_GL->EndFrame();
	//Win->Gfx().EndFrame_3D();
	Win->Gfx().EndFrame();
}

// =========================================================
// 終了処理
// =========================================================
void Finalize(void)
{
	// オブジェクト終了処理
	switch (scene)
	{
	case SCENE_GAME_1:
		FinalizeGame_1();
		break;
	case SCENE_TUTORIAL:
		FinalizeTutorial();
		break;
	case SCENE_RESULT:
		FinalizeResult();
		break;
	case SCENE_START:
		FinalizeStartMain();
		break;
	default:break;
	}

	if (APPmode == AppMode::Editor)
	{
		FinalizeEditor();
	}
	StopAudio(bgm);
	UnloadAudio(bgm);

	FinalizeFade();
	UninitAudio();
	// システム系終了処理
	UninitSprite();
	UninitSystem();
}


// =========================================================
// シーン切り替え
// =========================================================
void SetScene(SCENE next)
{

	switch (scene)
	{
	case SCENE_GAME_1:
		FinalizeGame_1();
		break;
	case SCENE_TUTORIAL:
		FinalizeTutorial();
		break;
	case SCENE_RESULT:
		FinalizeResult();
		break;
	case SCENE_START:
		FinalizeStartMain();
		break;
	default:break;
	}

	scene = next;

	switch (scene)
	{
	case SCENE_GAME_1:
		InitializeGame_1();
		break;
	case SCENE_TUTORIAL:
		InitializeTutorial();
		break;
	case SCENE_RESULT:
		InitializeResult();
		break;
	default:break;
	}
}

SCENE CheckScene(void)
{
	return scene;
}

void ResetScene(void)
{
	switch (scene)
	{
	case SCENE_GAME_1:
		FinalizeGame_1();
		InitializeGame_1();
		break;
	case SCENE_TUTORIAL :
		FinalizeTutorial();
		InitializeTutorial();
		break;
	case SCENE_RESULT:
		FinalizeResult();
		InitializeResult();
		break;
	default:break;
	}
}