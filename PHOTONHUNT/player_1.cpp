#include "player_1.h"
#include "sprite.h"
#include "shadow_1.h"
#include "collision.h"
#include "wall_1.h"
#include "main.h"
#include "enemy_1.h"
#include "graphics.h"
#include "gamebg_1.h"
#include  "hpbar.h"
#include "game_1.h"
#include "fade.h"
#include "Audio.h"
#include "goal.h"
#include "result.h"
#include "GamepadManager _1.0/GamepadManager.h"
#include <math.h>

static Player g_Player;

//  内部関数プロトタイプ
static void UpdateAttack(void);
static Enemy* FindNearestEnemy(float range);
static float  CalcDistance2D(float x1, float y1, float x2, float y2);
static bool IsPositionInsideWall(float cx,float cy);
static bool IsPositionInsideFloor(float cx, float cy);

void InitializePlayer01()
{
	g_Player.anim = FindObjectByName("player.an");
	g_Player.anim->obj.loadPolyCollision("Assets/Collision/player.obj");
	g_Player.anim->obj.Size = { 96.0f, 96.0f };
	g_Player.state = HUMAN;
	g_Player.idleState = IDLE_NONE;
	g_Player.hp = g_Player.maxHp;
	g_Player.movespeed = 4.0f;
	g_Player.inShadow = false;
	g_Player.shadowGaugeDepleting = false;
	g_Player.anim->obj.showPolyCollision = false;
	g_Player.anim->obj.Z_Test = 0.6f;
	g_Player.idleDuration = 30.0f; // 30秒でアイドル状態に移行

	for (auto& player : g_Player.anim->list) {
		player.size = g_Player.anim->obj.Size;
	}

	Win->Gfx().CreateDynamicCollisionBuffer(&g_Player.anim->obj);

	//攻撃関連の初期化
	g_Player.attackPhase = ATTACK_PHASE_NONE;
	g_Player.attackTimer = 0.0f;
	g_Player.attackTargetPos = { 0.0f, 0.0f };
	g_Player.attackBehindPos = { 0.0f, 0.0f };
	g_Player.attacking = false;
	g_Player.canAttack = false;
	g_Player.slashEffectActive = false;

	//影関連の初期化
	g_Player.shadowGauge = g_Player.shadowGaugeMax; // ゲージを満タンで開始

	// 斬撃エフェクトのアニメーションオブジェクトを取得
	// ※ エディタで登録したアニメーション名に合わせてください
	g_Player.slashEffect = FindObjectByName("slash_effect.an");
	g_Player.slashEffect->obj.Z_Test = 0.5f;

	SetAnimation(*g_Player.anim, "idle_human");

	// 音声関係初期化
	g_Player.runNow = false;
	g_Player.attackSe = LoadAudio(L"Assets/Sounds/player/player_attack.wav");
	g_Player.humanRunSe = LoadAudio(L"Assets/Sounds/player/player_run.wav");
	g_Player.shadowRunSe = LoadAudio(L"Assets/Sounds/player/shadow_run.wav");
	g_Player.transformSe = LoadAudio(L"Assets/Sounds/player/transform.wav");
}

void FinalizePlayer01(){
	// 音声関係解放
	UnloadAudio(g_Player.attackSe);
	UnloadAudio(g_Player.humanRunSe);
	UnloadAudio(g_Player.shadowRunSe);
	UnloadAudio(g_Player.transformSe);

	g_Player.anim->obj.vt.clear();
	g_Player.anim->obj.Indices_vt.clear();
}

void UpdatePlayer01()
{
	if (g_Player.anim == nullptr) return;

	//デバック用
	if (Win->kbd.KeyJustPressed('P'))
	{
		g_Player.anim->obj.showPolyCollision = !g_Player.anim->obj.showPolyCollision;
	}

	g_Player.anim->obj.Transform.Scale = { 1.0f,1.0f };
	g_Player.anim->obj.Transform.Offset = { 0.0f,0.0f };

	if (g_Player.hp <= 0)g_Player.state = PlayerState::DYING;

	// 移動前の位置を保存
	float oldX = g_Player.anim->obj.Transform.Position.x;
	float oldY = g_Player.anim->obj.Transform.Position.y;

	g_Player.inShadow = CheckPlayerInShadow();
	if (!g_Player.inShadow && g_Player.state == SHADOW)
	{
		g_Player.state = PlayerState::HUMAN;
		if (g_Player.state == PlayerState::SHADOW)
		{
			StopAudio(g_Player.shadowRunSe);
			g_Player.runNow = false;
		}
	}

	vector2 leftStick = manager.GetleftThumb();

	// スティックの優先方向を先に決める
	int absX = abs(leftStick.X);
	int absY = abs(leftStick.Y);

	bool stickUp = (absY > absX) && leftStick.Y > 0;
	bool stickDown = (absY > absX) && leftStick.Y < 0;
	bool stickLeft = (absX >= absY) && leftStick.X < 0;
	bool stickRight = (absX >= absY) && leftStick.X > 0;
	
	//アイドル状態判定
	if (!Win->kbd.AnyKeyIsPressed()&&!manager.IsAnyInput()) {
		g_Player.idleTimer += 1.0f / 60.0f;
		if (g_Player.idleTimer >= g_Player.idleDuration)
		{
			g_Player.state = IDLE;
		}
	}
	else {
		if (g_Player.state == IDLE)g_Player.state = HUMAN;
		g_Player.idleTimer = 0.0f;
	}

	switch (g_Player.state) {
		case PlayerState::HUMAN:
			g_Player.anim->speed = ANIMATION_SPEED;
			g_Player.anim->obj.flipX = false;
			manager.VibrateController(0, 0);
			g_Player.charge = 0.0f;
			g_Player.canAttack = false;
			StopAudio(g_Player.shadowRunSe);

			WinCamera.Scale -= 1.0f / 120.0f;
			if (WinCamera.Scale <= 2.5f) {
				WinCamera.Scale = 2.5f;
			}

			// 影ゲージの回復
			if (g_Player.shadowGauge < g_Player.shadowGaugeMax) {
				if (g_Player.shadowGaugeDepleting) {
					g_Player.shadowGauge += g_Player.shadowGaugeRegenRate * (1.0f / 60.0f) * 1.5f;
				}
				else {
					g_Player.shadowGauge += g_Player.shadowGaugeRegenRate * (1.0f / 60.0f);
				}
			}

			if (g_Player.shadowGauge >= g_Player.shadowGaugeMax) {
				g_Player.shadowGauge = g_Player.shadowGaugeMax;
				g_Player.shadowGaugeDepleting = false; // ゲージが満タンになったことを記録
			}

			if (Win->kbd.KeyJustPressed('Q')||manager.JustPressed("A"))
			{
				if (g_Player.state == PlayerState::HUMAN &&
					!g_Player.shadowGaugeDepleting &&
					g_Player.inShadow == true) {
					g_Player.state = PlayerState::SHADOW;
					StopAudio(g_Player.humanRunSe);
					g_Player.runNow = false;
					PlayAudio(g_Player.transformSe, false);
				}
			}

			if (Win->kbd.KeyIsPressed('W') || stickUp) {
				g_Player.anim->obj.Transform.Position.y += g_Player.movespeed;
				SetAnimation(*g_Player.anim, "front_run_human");
				if (!g_Player.runNow)
				{
					PlayAudio(g_Player.humanRunSe, true);
					g_Player.runNow = true;
				}
			}
			else if (Win->kbd.KeyIsPressed('S') || stickDown) {
				g_Player.anim->obj.Transform.Position.y -= g_Player.movespeed;
				SetAnimation(*g_Player.anim, "back_run_human");
				if (!g_Player.runNow)
				{
					PlayAudio(g_Player.humanRunSe, true);
					g_Player.runNow = true;
				}
			}
			else if (Win->kbd.KeyIsPressed('A') || stickLeft) {
				g_Player.anim->obj.Transform.Position.x -= g_Player.movespeed;
				SetAnimation(*g_Player.anim, "left_run_human");
				if (!g_Player.runNow)
				{
					PlayAudio(g_Player.humanRunSe, true);
					g_Player.runNow = true;
				}
			}
			else if (Win->kbd.KeyIsPressed('D') || stickRight) {
				g_Player.anim->obj.Transform.Position.x += g_Player.movespeed;
				SetAnimation(*g_Player.anim, "right_run_human");
				if (!g_Player.runNow)
				{
					PlayAudio(g_Player.humanRunSe, true);
					g_Player.runNow = true;
				}
			}
			else {
				g_Player.anim->speed = ANIMATION_SPEED * 0.5f;
				SetAnimation(*g_Player.anim, "idle_human");
				if (g_Player.runNow)
				{
					StopAudio(g_Player.humanRunSe);
					g_Player.runNow = false;
				}
			}
			break;

		case PlayerState::SHADOW:
			StopAudio(g_Player.humanRunSe);
			// 影ゲージの消耗
			g_Player.shadowGauge -= g_Player.shadowGaugeDepleteRate * (1.0f / 60.0f);
			if (g_Player.shadowGauge <= 0.0f) {
				g_Player.shadowGauge = 0.0f;
				g_Player.shadowGaugeDepleting = true; // ゲージが空になったことを記録
				g_Player.state = PlayerState::HUMAN; // 人間状態に戻る
			}

			if (Win->kbd.KeyJustPressed('Q') || manager.JustPressed("A"))
			{
				if (g_Player.state == PlayerState::SHADOW) {
					g_Player.state = PlayerState::HUMAN;
					StopAudio(g_Player.shadowRunSe);
					g_Player.runNow = false;
					PlayAudio(g_Player.transformSe, false);
				}
			}

			if (!g_Player.shadowGaugeDepleting) {
				if (Win->kbd.KeyIsPressed('W') || stickUp) {
					g_Player.anim->obj.Transform.Position.y += g_Player.movespeed;
					if (!g_Player.runNow)
					{
						PlayAudio(g_Player.shadowRunSe, true);
						g_Player.runNow = true;
					}
				}
				else if (Win->kbd.KeyIsPressed('S') || stickDown) {
					g_Player.anim->obj.Transform.Position.y -= g_Player.movespeed;
					if (!g_Player.runNow)
					{
						PlayAudio(g_Player.shadowRunSe, true);
						g_Player.runNow = true;
					}
				}
				else if (Win->kbd.KeyIsPressed('A') || stickLeft) {
					g_Player.anim->obj.Transform.Position.x -= g_Player.movespeed;
					if (!g_Player.runNow)
					{
						PlayAudio(g_Player.shadowRunSe, true);
						g_Player.runNow = true;
					}
				}
				else if (Win->kbd.KeyIsPressed('D') || stickRight) {
					g_Player.anim->obj.Transform.Position.x += g_Player.movespeed;
					if (!g_Player.runNow)
					{
						PlayAudio(g_Player.shadowRunSe, true);
						g_Player.runNow = true;
					}
				}
				else {
					if (g_Player.runNow)
					{
						StopAudio(g_Player.shadowRunSe);
						g_Player.runNow = false;
					}
				}
				g_Player.anim->obj.Transform.Scale = { 0.5f,0.5f };
				g_Player.anim->obj.Transform.Offset.y = -28.0f;
				SetAnimation(*g_Player.anim, "in_shadow_motion");
			}

			if (g_Player.attacking == false) {

				if (Win->kbd.KeyIsPressed('E') || manager.IsPressed("B")) {
					g_Player.charge += g_Player.shadowGaugeNeededForAttack / 42.0f;
					if (g_Player.charge >= g_Player.shadowGaugeNeededForAttack&&
						g_Player.shadowGauge >= g_Player.shadowGaugeNeededForAttack) {
						g_Player.canAttack = true;
						manager.VibrateController(150, 150);
					}
					else {
						g_Player.canAttack = false;
						manager.VibrateController(50, 50);
					}
				}
				
				if (Win->kbd.KeyJustReleased('E') || manager.JustReleased("B")) {
					
					if (g_Player.canAttack)
					{
						g_Player.shadowGauge -= g_Player.shadowGaugeNeededForAttack;
						Enemy* target = FindNearestEnemy(ATTACK_RANGE);
						if (target != nullptr)
						{
							// 攻撃対象の座標を保存
							g_Player.attackTargetPos.x = target->anim->obj.Transform.Position.x;
							g_Player.attackTargetPos.y = target->anim->obj.Transform.Position.y;

							// 「敵の後ろ」の座標を計算
							// プレイヤー → 敵 の方向ベクトルを求めて、敵の位置からさらに進んだ場所にする
							float px = g_Player.anim->obj.Transform.Position.x;
							float py = g_Player.anim->obj.Transform.Position.y;
							float ex = g_Player.attackTargetPos.x;
							float ey = g_Player.attackTargetPos.y;
							float dx = ex - px;
							float dy = ey - py;
							float len = sqrtf(dx * dx + dy * dy);
							if (len > 0.0f)
							{
								dx /= len;
								dy /= len;
							}
							g_Player.attackBehindPos.x = ex + dx * ATTACK_BEHIND_OFFSET;
							g_Player.attackBehindPos.y = ey + dy * ATTACK_BEHIND_OFFSET;

							// ── 壁めり込み解消 ──────────────────────────────────────────
							{
								const float step = 5.0f;
								const int maxSteps = 20;

								float bx = g_Player.attackBehindPos.x;
								float by = g_Player.attackBehindPos.y;

								bool resolved = false;

								// 真後ろへ後退
								for (int i = 0; i < maxSteps; i++)
								{
									if (!IsPositionInsideWall(bx, by)&& !IsPositionInsideFloor(bx, by))
									{
										resolved = true;
										break;
									}
									bx -= dx * step;
									by -= dy * step;
								}

								// 全方向ダメならプレイヤーの元の位置へフォールバック
								if (!resolved)
								{
									bx = px;
									by = py;
								}

								g_Player.attackBehindPos.x = bx;
								g_Player.attackBehindPos.y = by;
							}

							// ── 敵の方向に合わせて左右反転 ───────────────────
							// 敵がプレイヤーより左にいれば反転、右にいればそのまま
							g_Player.anim->obj.flipX = (g_Player.attackTargetPos.x < g_Player.anim->obj.Transform.Position.x);

							// 攻撃状態へ移行
							g_Player.state = PlayerState::ATTACK;
							g_Player.attackPhase = ATTACK_PHASE_WINDUP;
							g_Player.attackTimer = 0.0f;
							g_Player.slashEffectActive = false;

							g_Player.anim->speed = 0.1f;//10fで一枚更新

							// フェーズ1のアニメーションを再生
							SetAnimation(*g_Player.anim, "right_attack_windup");

							TakeDamageEnemy(*target, 100);
							g_Player.attacking = true;
							manager.VibrateController(250, 250);
							StopAudio(g_Player.shadowRunSe);
							PlayAudio(g_Player.attackSe, false);

							return;
						}
						else {
							g_Player.state = PlayerState::HUMAN;
						}
					}
					g_Player.charge = 0.0f;
					manager.VibrateController(0, 0);
				}
			}
			break;

		case PlayerState::ATTACK:
			UpdateAttack();
			return;

		case PlayerState::DYING:
			manager.VibrateController(0, 0);
			g_Player.anim->speed = 0.1f;
			SetAnimation(*g_Player.anim, "player_get_attack_left");
			if(IsAnimationFinished(*g_Player.anim))
			{
				g_Player.anim->speed = 0.0f;
				AddDead();
				ResetFade();
			}
			return;

		case PlayerState::IDLE:
			g_Player.anim->speed = 0.1f;
			manager.VibrateController(0, 0);
			WinCamera.Scale += 1.0f / 120.0f;
			if(WinCamera.Scale >= 3.5f)
			{
				WinCamera.Scale = 3.5f;
			}
			if (g_Player.idleTimer >= g_Player.idleDuration * 2) {//一分間隔
				g_Player.anim->speed = 0.05f;
				SetAnimation(*g_Player.anim, "player_stay");

				if (IsAnimationFinished(*g_Player.anim))
				{
					g_Player.idleTimer = g_Player.idleDuration;
					SetAnimation(*g_Player.anim, "idle_human");
				}
			}
			return;

		default:
			break;
	}

	//床との衝突チェック
	GameBG& gamebg = GetGameBG();
	bool bgCollision = false;
	bgCollision = CheckCollisionMeshVsMesh(g_Player.anim->obj, gamebg.obj);

	// 壁との衝突チェック
	bool wallCollision = false;
	std::vector<Wall>& walls = GetWalls();
	for (auto& wall : walls)
	{
		if (wall.usePolygonCollision && !wall.obj.Indices_vt.empty())
		{
			wallCollision = CheckCollisionMeshVsMesh(g_Player.anim->obj, wall.obj);
		}
		if (wallCollision) break; // 衝突した壁が見つかったら探索終了
	}

	// 床または壁に衝突したら元の位置に戻す
	if (wallCollision || bgCollision)
	{
		g_Player.anim->obj.Transform.Position.x = oldX;
		g_Player.anim->obj.Transform.Position.y = oldY;
	}

	SetFrame(g_Player.state);
}

//  攻撃更新（ATTACK state のときだけ呼ばれる）
static void UpdateAttack(void)
{
	g_Player.attackTimer += 1.0f / 60.0f;
	g_Player.state = PlayerState::ATTACK;

	switch (g_Player.attackPhase)
	{
		// ── フェーズ1：その場で溜め（アニメーション1〜5）──────────
	case ATTACK_PHASE_WINDUP:
		// アニメーションは SetAnimation で流れているのでそのまま待つ
		if (IsAnimationFinished(*g_Player.anim))
		{
			// フェーズ2へ
			g_Player.attackPhase = ATTACK_PHASE_SLASH;
			g_Player.attackTimer = 0.0f;

			// 斬撃エフェクトを敵の位置に移動して表示
			if (g_Player.slashEffect != nullptr)
			{
				g_Player.slashEffect->obj.Transform.Position.x = g_Player.attackTargetPos.x;
				g_Player.slashEffect->obj.Transform.Position.y = g_Player.attackTargetPos.y;
				g_Player.slashEffect->speed = g_Player.anim->speed;
				g_Player.slashEffect->obj.flipX = g_Player.anim->obj.flipX;
				SetAnimation(*g_Player.slashEffect, "slash");
				g_Player.slashEffectActive = true;
			}

			g_Player.anim->obj.Color.A = 0.0f; // プレイヤーを透明にしてエフェクトだけ見えるようにする
		}
		break;

		// ── フェーズ2：斬撃エフェクト（アニメーション6〜8）────────
	case ATTACK_PHASE_SLASH:
		if (IsAnimationFinished(*g_Player.slashEffect))
		{
			g_Player.attackPhase = ATTACK_PHASE_LAND;
			// エフェクトを非表示
			g_Player.slashEffectActive = false;

			// プレイヤーを敵の後ろにワープ
			g_Player.anim->obj.Transform.Position.x = g_Player.attackBehindPos.x;
			g_Player.anim->obj.Transform.Position.y = g_Player.attackBehindPos.y;

			// 着地アニメーション再生
			g_Player.anim->obj.Color.A = 1.0f;
			SetAnimation(*g_Player.anim, "right_attack_land");  // ※ アニメーション名を合わせてください
		}
		break;

		// ── フェーズ3：敵の後ろで着地（アニメーション9〜11）────────
	case ATTACK_PHASE_LAND:
		if (IsAnimationFinished(*g_Player.anim))
		{
			g_Player.attackPhase = ATTACK_PHASE_NONE;
			g_Player.attackTimer = 0.0f;
			g_Player.state = PlayerState::HUMAN;
			g_Player.attacking = false;

			SetAnimation(*g_Player.anim, "idle_human");
		}
		break;

	default:
		break;
	}
}

//  範囲内で一番近い敵を探す
static Enemy* FindNearestEnemy(float range)
{
	std::vector<Enemy>& enemies = GetEnemies();

	float  px = g_Player.anim->obj.Transform.Position.x;
	float  py = g_Player.anim->obj.Transform.Position.y;
	float  nearestDist = range;  // range 以内でないと対象にしない
	Enemy* nearest = nullptr;

	for (auto& enemy : enemies)
	{
		if (!enemy.active)        continue;
		if (enemy.anim == nullptr) continue;

		float dist = CalcDistance2D(px, py,
			enemy.anim->obj.Transform.Position.x,
			enemy.anim->obj.Transform.Position.y);

		if (dist > nearestDist) continue;

		GameBG& gamebg = GetGameBG();
		if (CheckCollisionLineVsMesh(g_Player.anim->obj.Transform.Position, enemy.anim->obj.Transform.Position, gamebg.obj)) {
			continue;
		}

		// 壁との衝突チェック
		bool blocked = false;
		std::vector<Wall>& walls = GetWalls();
		for (auto& wall : walls)
		{
			if (CheckCollisionLineVsMesh(g_Player.anim->obj.Transform.Position, enemy.anim->obj.Transform.Position, wall.obj)) {
				blocked = true;
				break;
			}
		}
		if (blocked) continue;

		nearestDist = dist;
		nearest = &enemy;
	}

	return nearest;  // 範囲内に敵がいなければ nullptr
}

//  2点間の距離
static float CalcDistance2D(float x1, float y1, float x2, float y2)
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	return sqrtf(dx * dx + dy * dy);
}

void DrawPlayer01()
{
	if (g_Player.anim == nullptr) return;

	BoxVertexsMake(&g_Player.anim->obj);
	DrawObject_2D(&g_Player.anim->obj);

	// 斬撃エフェクトの描画
	if (g_Player.slashEffectActive && g_Player.slashEffect != nullptr)
	{
		BoxVertexsMake(&g_Player.slashEffect->obj);
		DrawObject_2D(&g_Player.slashEffect->obj);
	}
}

Player& GetPlayer()
{
	return g_Player;
}

void TakeDamagePlayer(int damage) {
	g_Player.hp -= damage;
}

void SetPlayerPos(Object_2D::Vec2 pos) {
	g_Player.anim->obj.Transform.Position = pos;
	g_Player.startPos = pos;
}

static bool IsPositionInsideWall(float cx, float cy) {
	// プレイヤーのObject_2Dを候補座標へ一時コピー
	Object_2D tempObj = g_Player.anim->obj;
	tempObj.Transform.Position.x = cx;
	tempObj.Transform.Position.y = cy;

	auto& g_Walls = GetWalls();
	for (auto& wall : g_Walls)
	{
		if (CheckCollisionMeshVsMesh(tempObj, wall.obj))return true;
	}
	return false;
}

static bool IsPositionInsideFloor(float cx, float cy) {
	Object_2D tempObj = g_Player.anim->obj;
	tempObj.Transform.Position.x = cx;
	tempObj.Transform.Position.y = cy;

	auto& g_Floor = GetGameBG();
	if (CheckCollisionMeshVsMesh(tempObj, g_Floor.obj))return true;

	return false;
}

void ResetPlayer() {
	g_Player.state = HUMAN;
	g_Player.idleState = IDLE_NONE;
	g_Player.hp = g_Player.maxHp;
	g_Player.anim->obj.Transform.Position = g_Player.startPos;
	g_Player.inShadow = false;
	g_Player.shadowGaugeDepleting = false;
	g_Player.attackPhase = ATTACK_PHASE_NONE;
}