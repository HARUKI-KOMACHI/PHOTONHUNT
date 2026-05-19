#include "enemy_1.h"
#include "sprite.h"
#include "player_1.h"
#include "shadow_1.h"
#include "wall_1.h"
#include "collision.h"
#include "gridmap.h"
#include "goal.h"
#include "main.h"
#include "Audio.h"
#include <math.h>

static void Patrol(Enemy& enemy);
static void Chase(Enemy& enemy);
static void Search(Enemy& enemy);
static void Attack(Enemy& enemy);
static float GetDistance(const Object_2D& obj1, const Object_2D& obj2);
static bool CanSeePlayer(const Enemy& enemy);

// ── ルート検索・移動────────────────────────────────
static void RequestPath(Enemy& enemy, float goalX, float goalY);
static void MoveAlongPath(Enemy& enemy, float speed);
static void UpdateAnimation(Enemy& enemy, float dx, float dy);
static bool IsBlockedByWall(Wall wall,
	float ex, float ey, float px, float py);

static std::vector<Enemy> g_Enemies;
extern std::vector<Segment> g_WallSegments;

static int attackSe = -1;
static int deathSe = -1;

void InitializeEnemy01(void) {
	attackSe = LoadAudio(L"Assets/Sounds/enemy/enemy_attack.wav");
	deathSe = LoadAudio(L"Assets/Sounds/enemy/enemy_death.wav");
}

void FinalizeEnemy01() {
	g_Enemies.clear();
	FinalizeGridMap();

	UnloadAudio(attackSe);
	UnloadAudio(deathSe);
}

void AddEnemy(const std::string& animName, const std::vector<MovePoint>& path)
{
	Enemy enemy;

	// アニメーション取得
	enemy.anim = FindObjectByName(animName);
	if (enemy.anim == nullptr)
	{
		// アニメーションが見つからない場合は追加しない
		return;
	}

	// 初期設定
	enemy.hp = enemy.maxHP;
	enemy.state = ENEMY_PATROL;
	enemy.viewAngle = 90.0f;				// 視野
	enemy.viewDistance = 150.0f;			// 視界範囲
	enemy.viewDirection = 0.0f;				// 右向き
	enemy.movePath = path;
	enemy.currentPointIndex = 0;
	enemy.patrolSpeed = 3.0f;				// 巡回速度
	enemy.waitTimer = 0.0f;
	enemy.waitDuration = 2.0f;				// 2秒待機
	enemy.chaseSpeed = 4.5f;				//追跡速度
	enemy.attackRange = 50.0f;				//攻撃範囲
	enemy.attackTimer = 0.0f;
	enemy.attackDuration = 5.0f;				//攻撃クールダウン
	enemy.canAttacking = true;				//攻撃可能フラグ
	enemy.searchTimer = 0.0f;
	enemy.searchDuration = 4.0f;
	enemy.searchDirection = 0;				// 最初は上向き
	enemy.searchDirectionTimer = 0.0f;
	enemy.searchDirectionDuration = 1.0f;	// 1方向あたり1秒
	enemy.active = true;
	enemy.pathPointIndex = 0;
	enemy.lastSeenPlayerPos = { 0.0f, 0.0f };
	enemy.pathUpdateTimer = PATH_UPDATE_INTERVAL;  // 最初から探索できるようにする
	enemy.searchArrived = false;

	enemy.anim->obj.Transform.Scale = { 0.5f,0.5f };
	enemy.anim->speed = ENEMY_ANIMATION_SPEED;

	// 最初のポイントに配置
	if (!path.empty())
	{
		enemy.anim->obj.Transform.Position.x = path[0].x;
		enemy.anim->obj.Transform.Position.y = path[0].y;
	}

	g_Enemies.push_back(enemy);
}

void UpdateEnemy01(void)
{
	bool noActive = true;
	auto scene = CheckScene();

	for (auto& enemy : g_Enemies)
	{
		if (scene != SCENE_GAME_1) {
			
			static float reviveTimer;
			if (enemy.hp <= 0)
			{
				enemy.animTimer += 1.0f / 60.0f;
				enemy.anim->speed = 0.1f;
				SetAnimation(*enemy.anim, "slime_get_attack");
				

				if (IsAnimationFinished(*enemy.anim))
				{
					enemy.active = false;
					PlayAudio(deathSe, false);
				}

				if (!enemy.active) {
					reviveTimer += 1.0f / 60.0f;
					if (reviveTimer >= 2.0f) {
						ResetEnemy();
						reviveTimer = 0.0f;
					}
				}
				return;
			}

			Patrol(enemy);

			break;
		}

		if (enemy.active) {
			noActive = false;
			if (enemy.hp <= 0)
			{
				enemy.animTimer += 1.0f / 60.0f;
				enemy.anim->speed = 0.1f;
				SetAnimation(*enemy.anim, "slime_get_attack");
				

				if (IsAnimationFinished(*enemy.anim))
				{
					enemy.active = false;
					PlayAudio(deathSe, false);
				}
				return;
			}

			bool seePlayer = CanSeePlayer(enemy);

			switch (enemy.state)
			{
			case ENEMY_PATROL:
				Patrol(enemy);
				if (seePlayer)
				{
					enemy.state = ENEMY_CHASE;
					enemy.pathUpdateTimer = PATH_UPDATE_INTERVAL;  // 即座に経路計算させる
					enemy.pathPoints.clear();
				}
				break;

			case ENEMY_CHASE:
				Chase(enemy);
				break;

			case ENEMY_SEARCH:
				Search(enemy);
				// 捜索中にプレイヤーを再発見したら追跡へ
				if (seePlayer)
				{
					enemy.state = ENEMY_CHASE;
					enemy.pathUpdateTimer = PATH_UPDATE_INTERVAL;
					enemy.pathPoints.clear();
				}
				break;

			case ENEMY_ATTACK:
				Attack(enemy);
				break;
			}
		}
	}

	if (noActive&&scene==SCENE_GAME_1) {
		g_goal.isOpen = true;
	}
}

// 巡回更新
static void Patrol(Enemy& enemy)
{
	if (enemy.movePath.empty()) return;

	if (enemy.waitTimer > 0.0f)
	{
		enemy.waitTimer -= 1.0f / 60.0f;
		SetAnimation(*enemy.anim, "idle");
		return;
	}

	MovePoint& target = enemy.movePath[enemy.currentPointIndex];

	float dx = target.x - enemy.anim->obj.Transform.Position.x;
	float dy = target.y - enemy.anim->obj.Transform.Position.y;
	float dist = sqrtf(dx * dx + dy * dy);

	if (dist < 5.0f)
	{
		enemy.currentPointIndex = (enemy.currentPointIndex + 1) % (int)enemy.movePath.size();
		enemy.waitTimer = enemy.waitDuration;
		return;
	}

	dx /= dist;
	dy /= dist;

	enemy.anim->obj.Transform.Position.x += dx * enemy.patrolSpeed;
	enemy.anim->obj.Transform.Position.y += dy * enemy.patrolSpeed;

	enemy.viewDirection = atan2f(dy, dx) * 180.0f / 3.14159265f;
	UpdateAnimation(enemy, dx, dy);
}

// 追跡更新
static void Chase(Enemy& enemy)
{
	Player& player = GetPlayer();

	float dist = GetDistance(enemy.anim->obj, player.anim->obj);

	// ── 見失い判定 ───────────────────────────────────────────
	if (dist > enemy.viewDistance * 1.5f || player.state == PlayerState::SHADOW)
	{
		enemy.state = ENEMY_SEARCH;
		enemy.searchTimer = enemy.searchDuration;
		enemy.searchArrived = false;
		enemy.pathPoints.clear();
		// 捜索の目標地点へ向かう経路を計算
		RequestPath(enemy, enemy.lastSeenPlayerPos.x, enemy.lastSeenPlayerPos.y);
		return;
	}

	// ── 攻撃範囲内 ──────────────────────────────────────────
	if (dist < enemy.attackRange)
	{
		enemy.state = ENEMY_ATTACK;
		return;
	}

	// ── プレイヤーを視界に収めている間は最終座標を更新 ────────
	if (CanSeePlayer(enemy))
	{
		enemy.lastSeenPlayerPos.x = player.anim->obj.Transform.Position.x;
		enemy.lastSeenPlayerPos.y = player.anim->obj.Transform.Position.y;
	}

	// ── 経路更新（一定間隔で A* を再計算）───────────────────
	enemy.pathUpdateTimer += 1.0f / 60.0f;
	if (enemy.pathUpdateTimer >= PATH_UPDATE_INTERVAL)
	{
		enemy.pathUpdateTimer = 0.0f;
		RequestPath(enemy,
			player.anim->obj.Transform.Position.x,
			player.anim->obj.Transform.Position.y);
	}

	// ── 経路に沿って移動 ─────────────────────────────────────
	if (!enemy.pathPoints.empty() && enemy.pathPointIndex < (int)enemy.pathPoints.size())
	{
		MoveAlongPath(enemy, enemy.chaseSpeed);
	}
	else
	{
		// 経路なし（A* 失敗 or 初回）→ 既存の直接移動でフォールバック
		if (dist > 0.0f)
		{
			float dx = player.anim->obj.Transform.Position.x - enemy.anim->obj.Transform.Position.x;
			float dy = player.anim->obj.Transform.Position.y - enemy.anim->obj.Transform.Position.y;
			dx /= dist;
			dy /= dist;

			float oldX = enemy.anim->obj.Transform.Position.x;
			float oldY = enemy.anim->obj.Transform.Position.y;

			enemy.anim->obj.Transform.Position.x += dx * enemy.chaseSpeed;
			enemy.anim->obj.Transform.Position.y += dy * enemy.chaseSpeed;

			BoxVertexsMake(&enemy.anim->obj);

			std::vector<Wall>& walls = GetWalls();
			for (auto& wall : walls)
			{
				bool hit = (wall.usePolygonCollision && !wall.obj.Indices_vt.empty())
					? CheckCollisionQuadVsMesh(enemy.anim->obj, wall.obj)
					: CheckCollision_Object2D(enemy.anim->obj, wall.obj);

				if (hit)
				{
					enemy.anim->obj.Transform.Position.x = oldX;
					enemy.anim->obj.Transform.Position.y = oldY;
					break;
				}
			}

			enemy.viewDirection = atan2f(dy, dx) * 180.0f / 3.14159265f;
			UpdateAnimation(enemy, dx, dy);
		}
	}
}

static void Search(Enemy& enemy)
{
	enemy.searchTimer -= 1.0f / 60.0f;

	// 捜索時間終了 → 巡回に戻る
	if (enemy.searchTimer <= 0.0f)
	{
		enemy.state = ENEMY_PATROL;
		enemy.pathPoints.clear();
		return;
	}

	// ── フェーズ1：最後に見た場所へ移動 ──────────────────────
	if (!enemy.searchArrived)
	{
		if (!enemy.pathPoints.empty() && enemy.pathPointIndex < (int)enemy.pathPoints.size())
		{
			// 経路に沿って移動
			MoveAlongPath(enemy, enemy.patrolSpeed);

			// 経路を全部消費したら到着
			if (enemy.pathPointIndex >= (int)enemy.pathPoints.size())
			{
				enemy.searchArrived = true;
				enemy.searchDirectionTimer = enemy.searchDirectionDuration;
			}
		}
		else
		{
			// 経路が空 → 距離で到着判定
			float dx = enemy.lastSeenPlayerPos.x - enemy.anim->obj.Transform.Position.x;
			float dy = enemy.lastSeenPlayerPos.y - enemy.anim->obj.Transform.Position.y;
			float dist = sqrtf(dx * dx + dy * dy);

			if (dist < ARRIVE_THRESHOLD)
			{
				enemy.searchArrived = true;
				enemy.searchDirectionTimer = enemy.searchDirectionDuration;
			}
			else
			{
				// 経路を再要求
				RequestPath(enemy, enemy.lastSeenPlayerPos.x, enemy.lastSeenPlayerPos.y);
			}
		}
		return;
	}

	// ── フェーズ2：到着後に見回す ────────────────────────────
	enemy.searchDirectionTimer -= 1.0f / 60.0f;

	if (enemy.searchDirectionTimer <= 0.0f)
	{
		enemy.searchDirection = (enemy.searchDirection + 1) % 4;
		enemy.searchDirectionTimer = enemy.searchDirectionDuration;
	}

	switch (enemy.searchDirection)
	{
	case 0: SetAnimation(*enemy.anim, "front_walk_suraimu"); enemy.viewDirection = 90.0f; break;
	case 1: SetAnimation(*enemy.anim, "right_walk_suraimu"); enemy.viewDirection = 0.0f; break;
	case 2: SetAnimation(*enemy.anim, "back_walk_suraimu");  enemy.viewDirection = -90.0f; break;
	case 3: SetAnimation(*enemy.anim, "left_walk_suraimu");  enemy.viewDirection = 180.0f; break;
	}
}

// 攻撃更新
static void Attack(Enemy& enemy)
{
	Player& player = GetPlayer();

	if (IsAnimationFinished(*enemy.anim)) {
		enemy.anim->obj.Transform.Scale = { 0.5f,0.5f };
		SetAnimation(*enemy.anim, "back_walk_suraimu");
	}

	if (!enemy.canAttacking) {
		enemy.attackTimer += 1.0f / 60.0f;
		if (enemy.attackTimer >= enemy.attackDuration)
		{
			enemy.attackTimer = 0.0f;
			enemy.canAttacking = true;
			enemy.state = EnemyState::ENEMY_SEARCH;
			enemy.searchTimer = enemy.searchDuration;
		}
		return;
	}

	// プレイヤーとの距離を計算
	float dist = GetDistance(enemy.anim->obj, player.anim->obj);

	// プレイヤーが遠すぎる場合、見失った判定
	if (dist > enemy.viewDistance * 1.5f)
	{
		// 捜索モードに移行
		enemy.state = ENEMY_SEARCH;
		enemy.searchTimer = enemy.searchDuration;
		return;
	}

	// 攻撃範囲外に出たら追跡に戻る
	if (dist > enemy.attackRange)
	{
		enemy.state = ENEMY_CHASE;
		return;
	}

	// 攻撃処理（ダメージ判定など）
	TakeDamagePlayer(100);
	enemy.anim->speed = 0.1f;
	PlayAudio(attackSe, false);

	// 攻撃アニメーション
	float dx = player.anim->obj.Transform.Position.x - enemy.anim->obj.Transform.Position.x;
	float absDx = fabsf(dx);

	if (absDx >= 0) {
		SetAnimation(*enemy.anim, "slime_attack_motion");
	}
	else {
		SetAnimation(*enemy.anim, "slime_attack_left_motion");
	}
	
	enemy.canAttacking = false;

	enemy.anim->obj.Transform.Scale = { 1.0f,1.0f };
}

// 視界判定
static bool CanSeePlayer(const Enemy& enemy)
{
	Player& player = GetPlayer();
	if (player.anim == nullptr) return false;
	if (player.state == PlayerState::SHADOW) return false;

	float ex = enemy.anim->obj.Transform.Position.x;
	float ey = enemy.anim->obj.Transform.Position.y;
	float px = player.anim->obj.Transform.Position.x;
	float py = player.anim->obj.Transform.Position.y;

	float dx = px - ex;
	float dy = py - ey;
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist > enemy.viewDistance) return false;

	float angleToPlayer = atan2f(dy, dx) * 180.0f / 3.14159265f;
	float angleDiff = angleToPlayer - enemy.viewDirection;
	while (angleDiff > 180.0f) angleDiff -= 360.0f;
	while (angleDiff < -180.0f) angleDiff += 360.0f;
	if (fabsf(angleDiff) >= enemy.viewAngle / 2.0f) return false;

	// ★ 壁による遮蔽チェック
	auto WallCol = GetWalls();
	for (auto& wall : WallCol) {
		if (IsBlockedByWall(wall, ex, ey, px, py))return false;
	}

	return true;
}

// 距離計算
static float GetDistance(const Object_2D& obj1, const Object_2D& obj2)
{
	float dx = obj2.Transform.Position.x - obj1.Transform.Position.x;
	float dy = obj2.Transform.Position.y - obj1.Transform.Position.y;
	return sqrt(dx * dx + dy * dy);
}

//  RequestPath  経路要求（直線 or A*）
static void RequestPath(Enemy& enemy, float goalX, float goalY)
{
	enemy.pathPoints.clear();
	enemy.pathPointIndex = 0;

	float myX = enemy.anim->obj.Transform.Position.x;
	float myY = enemy.anim->obj.Transform.Position.y;

	// ── 直線で行けるか確認（Bresenham） ─────────────────────
	if (HasClearLine(myX, myY, goalX, goalY))
	{
		// 直線で到達可能 → ウェイポイントはゴール1点だけ
		enemy.pathPoints.push_back({ goalX, goalY });
		return;
	}

	// ── A* で経路探索 ────────────────────────────────────────
	GridPos startGrid = WorldToGrid(myX, myY);
	GridPos goalGrid = WorldToGrid(goalX, goalY);

	// 目標グリッドが通行不可（壁の中など）なら近傍セルを探す
	if (!IsPassable(goalGrid))
	{
		bool found = false;
		for (int r = 1; r <= 3 && !found; ++r)
		{
			for (int dy2 = -r; dy2 <= r && !found; ++dy2)
			{
				for (int dx2 = -r; dx2 <= r && !found; ++dx2)
				{
					GridPos candidate{ goalGrid.x + dx2, goalGrid.y + dy2 };
					if (IsPassable(candidate))
					{
						goalGrid = candidate;
						found = true;
					}
				}
			}
		}
		if (!found) return;  // 周辺にも通行可がない → あきらめる
	}

	std::vector<GridPos> gridPath;
	if (FindPath(startGrid, goalGrid, gridPath))
	{
		enemy.pathPoints.reserve(gridPath.size());
		for (const GridPos& gp : gridPath)
		{
			enemy.pathPoints.push_back(GridToWorld(gp));
		}
	}
	// FindPath が false（経路なし）の場合は pathPoints が空のまま
	// Chase() でフォールバックの直接移動に切り替わる
}

//  MoveAlongPath  経路に沿った移動
// speed : 移動速度（ワールド単位/フレーム）
static void MoveAlongPath(Enemy& enemy, float speed)
{
	if (enemy.pathPoints.empty()) return;
	if (enemy.pathPointIndex >= (int)enemy.pathPoints.size()) return;

	// remaining : このフレームで動ける残り距離
	float remaining = speed;

	while (remaining > 0.0f && enemy.pathPointIndex < (int)enemy.pathPoints.size())
	{
		Object_2D::Vec2& wp = enemy.pathPoints[enemy.pathPointIndex];

		float dx = wp.x - enemy.anim->obj.Transform.Position.x;
		float dy = wp.y - enemy.anim->obj.Transform.Position.y;
		float dist = sqrtf(dx * dx + dy * dy);

		if (dist <= ARRIVE_THRESHOLD)
		{
			// ウェイポイントに到達 → 座標をスナップして次へ
			enemy.anim->obj.Transform.Position.x = wp.x;
			enemy.anim->obj.Transform.Position.y = wp.y;
			remaining -= dist;
			enemy.pathPointIndex++;
			continue;
		}

		if (dist <= remaining)
		{
			// 残り移動距離でこのウェイポイントを超えられる
			enemy.anim->obj.Transform.Position.x = wp.x;
			enemy.anim->obj.Transform.Position.y = wp.y;
			remaining -= dist;
			enemy.pathPointIndex++;
		}
		else
		{
			// ウェイポイント手前まで進んで終了
			float ndx = dx / dist;
			float ndy = dy / dist;

			enemy.anim->obj.Transform.Position.x += ndx * remaining;
			enemy.anim->obj.Transform.Position.y += ndy * remaining;

			enemy.viewDirection = atan2f(ndy, ndx) * 180.0f / 3.14159265f;
			UpdateAnimation(enemy, ndx, ndy);

			remaining = 0.0f;
		}
	}
}

//  UpdateAnimation  4方向アニメーション切り替え（共通化）
static void UpdateAnimation(Enemy& enemy, float dx, float dy)
{
	float absDx = fabsf(dx);
	float absDy = fabsf(dy);

	if (absDx > absDy)
	{
		SetAnimation(*enemy.anim, (dx > 0) ? "right_walk_suraimu" : "left_walk_suraimu");
	}
	else
	{
		SetAnimation(*enemy.anim, (dy > 0) ? "front_walk_suraimu" : "back_walk_suraimu");
	}
}

// 敵描画
void DrawEnemy01(void)
{
	for (auto& enemy : g_Enemies)
	{
		if (!enemy.active) continue;
		if (enemy.anim == nullptr) continue;

		BoxVertexsMake(&enemy.anim->obj);
		DrawObject_2D(&enemy.anim->obj);
	}
}

void TakeDamageEnemy(Enemy& enemy, int damage)
{
	enemy.hp -= damage;
}

// 敵リスト取得
std::vector<Enemy>& GetEnemies(void)
{
	return g_Enemies;
}

// 線分ABと線分CDが交差するか
static bool SegmentsIntersect(
	float ax, float ay, float bx, float by,
	float cx, float cy, float dx, float dy)
{
	float d1x = bx - ax, d1y = by - ay;
	float d2x = dx - cx, d2y = dy - cy;
	float cross = d1x * d2y - d1y * d2x;
	if (fabsf(cross) < 1e-8f) return false;

	float tx = cx - ax, ty = cy - ay;
	float t = (tx * d2y - ty * d2x) / cross;
	float u = (tx * d1y - ty * d1x) / cross;

	return (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f);
}

static bool IsBlockedByWall(Wall wall,
	float ex,float ey,float px,float py)
{
	// 3インデックスごとに1三角形
	for (size_t i = 0; i + 2 < wall.obj.Indices_vt.size(); i += 3)
	{
		const auto& vt0 = wall.obj.vt[wall.obj.Indices_vt[i]];
		const auto& vt1 = wall.obj.vt[wall.obj.Indices_vt[i + 1]];
		const auto& vt2 = wall.obj.vt[wall.obj.Indices_vt[i + 2]];

		float u0 = (vt0.x - 0.5f) * wall.obj.Size.x, v0 = (vt0.y - 0.5f) * wall.obj.Size.y;
		float u1 = (vt1.x - 0.5f) * wall.obj.Size.x, v1 = (vt1.y - 0.5f) * wall.obj.Size.y;
		float u2 = (vt2.x - 0.5f) * wall.obj.Size.x, v2 = (vt2.y - 0.5f) * wall.obj.Size.y;

		// ローカル座標 → ワールド座標に変換
		float w0x = u0 + wall.obj.Transform.Position.x, w0y = v0 + wall.obj.Transform.Position.y;
		float w1x = u1 + wall.obj.Transform.Position.x, w1y = v1 + wall.obj.Transform.Position.y;
		float w2x = u2 + wall.obj.Transform.Position.x, w2y = v2 + wall.obj.Transform.Position.y;

		// 三角形の3辺それぞれと視線が交差するか
		if (SegmentsIntersect(ex, ey, px, py, w0x, w0y, w1x, w1y)) return true;
		if (SegmentsIntersect(ex, ey, px, py, w1x, w1y, w2x, w2y)) return true;
		if (SegmentsIntersect(ex, ey, px, py, w2x, w2y, w0x, w0y)) return true;
	}
	return false;
}

void ResetEnemy() {
	for (auto& enemy : g_Enemies) {
		enemy.hp = enemy.maxHP;
		enemy.state = ENEMY_PATROL;
		enemy.waitTimer = 0.0f;
		enemy.attackTimer = 0.0f;
		enemy.canAttacking = true;
		enemy.searchTimer = 0.0f;
		enemy.searchDirection = 0;
		enemy.searchDirectionTimer = 0.0f;
		enemy.active = true;
		enemy.pathPointIndex = 0;
		enemy.pathUpdateTimer = PATH_UPDATE_INTERVAL;
		enemy.searchArrived = false;
		enemy.anim->speed = ENEMY_ANIMATION_SPEED;
	}
}