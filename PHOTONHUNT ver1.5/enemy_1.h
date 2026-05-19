#pragma once
#include "Editor.h"
#include "gridmap.h"
#include <vector>

#define DIR_UP (0)
#define DIR_RIGHT (1)
#define DIR_DOWN (2)
#define DIR_LEFT (3)

#define ENEMY_ANIMATION_SPEED 0.3f

// ── 経路更新間隔（秒）─────────────────────────────────────────
// 0.2f〜0.5f が目安（小さいほど追跡精度↑、処理負荷↑）
static constexpr float PATH_UPDATE_INTERVAL = 0.3f;

// ── ウェイポイント到着判定の閾値（ワールド単位）──────────────
static constexpr float ARRIVE_THRESHOLD = 8.0f;

enum EnemyState
{
	ENEMY_PATROL,		// 巡回
	ENEMY_CHASE,		// 追跡
	ENEMY_SEARCH,		// 捜索（見失った後）
	ENEMY_ATTACK		// 攻撃
};

struct Segment {
	float x1, y1, x2, y2;
};


class MovePoint {
public:
	float x;
	float y;
};

class Enemy {
public:
	animation_Object_2D* anim;
	EnemyState state;

	int hp;						// 体力
	int maxHP = 100;

	// 視界
	float viewAngle;					// 視野角（度）例: 60度
	float viewDistance;					// 視認距離
	float viewDirection;				// 向いてる方向（度）

	// 巡回
	std::vector<MovePoint> movePath;	// 巡回ルート
	int currentPointIndex;				// 現在の目標ポイント
	float patrolSpeed;					// 巡回速度
	float waitTimer;					// ポイント到着後の待機時間
	float waitDuration;					// 待機する秒数

	// 追跡・攻撃
	float chaseSpeed;					// 追跡速度
	float attackRange;					// 攻撃範囲
	float attackTimer;					// 攻撃クールダウンタイマー
	float attackDuration;				// 攻撃クールダウン時間（秒）
	bool canAttacking;				// 攻撃可能フラグ（攻撃アニメーションのタイミングで true になる）

	// 捜索
	float searchTimer;					// 捜索タイマー
	float searchDuration;				// 捜索時間（秒）
	int searchDirection;				// 現在見ている方向（0=上, 1=右, 2=下, 3=左）
	float searchDirectionTimer;			// 各方向を見る時間
	float searchDirectionDuration;		// 1方向あたりの時間（秒）

	// A* で計算した経路をワールド座標のウェイポイントとして保持する
	std::vector<Object_2D::Vec2> pathPoints;	// 経路ウェイポイント（ワールド座標）
	int   pathPointIndex;						// 現在向かっているウェイポイントのインデックス

	// 最後にプレイヤーを視界に収めた座標（Search 状態の目標地点として使う）
	Object_2D::Vec2 lastSeenPlayerPos;

	// 経路更新タイマー（重い A* を毎フレーム走らせないため）
	// PATH_UPDATE_INTERVAL 秒ごとに経路を再計算する
	float pathUpdateTimer;

	// 捜索フェーズの管理（false=目標地点に移動中、true=到着して見回し中）
	bool searchArrived;

	int direction;					// 向き（0=上, 1=右, 2=下, 3=左）
	bool active;						// 生存フラグ
	float animTimer = 0;				// アニメーションタイマー
};



void InitializeEnemy01();
void FinalizeEnemy01();
void UpdateEnemy01();
void DrawEnemy01();

void TakeDamageEnemy(Enemy& enemy, int damage);

std::vector<Enemy>& GetEnemies();

void AddEnemy(const std::string& animName, const std::vector<MovePoint>& path);

void ResetEnemy();