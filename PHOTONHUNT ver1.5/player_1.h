#pragma once
#include "Editor.h"

enum PlayerState
{
    HUMAN,          // 人間状態
    SHADOW,         // 影状態
    ATTACK,         // 攻撃中
    DYING,          // 死亡モーション中
	IDLE,		   // アイドル状態（触られていないとき）
};

// 攻撃フェーズ
enum AttackPhase
{
    ATTACK_PHASE_NONE = 0,
    ATTACK_PHASE_WINDUP = 1,    // フレーム1〜5：その場で溜め
    ATTACK_PHASE_SLASH = 2,    // フレーム6〜8：敵の位置に斬撃エフェクト
    ATTACK_PHASE_LAND = 3,    // フレーム9〜11：敵の後ろにワープして着地
};

enum PlayerIdleState
{
    IDLE_NONE=0,        // 通常状態
    IDLE_SITTING,     // 座り中
    IDLE_GESTURE,     // しぐさループ中
    IDLE_STANDING,     // 立ち上がり中
};

//アニメーションスピード
#define ANIMATION_SPEED (0.3f)
// 攻撃範囲
#define ATTACK_RANGE (150.0f)

// 敵の後ろに出る距離
#define ATTACK_BEHIND_OFFSET (60.0f)

// 各フェーズの継続時間（秒）
// アニメーションのフレーム数 / FPS で計算
// フェーズ1: 5フレーム, フェーズ2: 3フレーム, フェーズ3: 3フレーム
#define ATTACK_PHASE1_DURATION (5.0f / 60.0f * 10.0f)
#define ATTACK_PHASE2_DURATION (3.0f / 60.0f * 10.0f) 
#define ATTACK_PHASE3_DURATION (3.0f / 60.0f * 10.0f) 

class Player {
public:
    animation_Object_2D* anim;
	PlayerState state;
	PlayerIdleState idleState;

    int hp;
	int maxHp = 100;

    Object_2D::Vec2 startPos;
    float movespeed;
    bool inShadow;
    float charge;

    // 攻撃関連
    AttackPhase attackPhase;            // 現在の攻撃フェーズ
    float attackTimer;            // フェーズ内の経過時間（秒）
    bool attacking;
    bool canAttack;

    // 攻撃対象の情報（フェーズをまたいで使う）
    Object_2D::Vec2 attackTargetPos;    // 攻撃対象の座標（フェーズ2でエフェクト表示に使う）
    Object_2D::Vec2 attackBehindPos;    // 敵の後ろの座標（フェーズ3でワープ先として使う）

    // 斬撃エフェクト用オブジェクト（フレーム6〜8をここで描画）
    animation_Object_2D* slashEffect;   // 斬撃エフェクトのアニメーション
    bool slashEffectActive;             // エフェクト表示中か

    //影関連
    float shadowGauge = 0.0f;              // 影ゲージ（0.0〜100.0）
	float shadowGaugeMax = 100.0f;          // 影ゲージの最大値
	float shadowGaugeRegenRate = 10.0f;     // 影ゲージの回復速度（秒あたりの増加量）
	float shadowGaugeDepleteRate = 20.0f;   // 影ゲージの消耗速度（秒あたりの減少量）
	bool shadowGaugeDepleting = false;     // 影ゲージが一度全て空になったか
	float shadowGaugeNeededForAttack = 30.0f; // 攻撃に必要な影ゲージの量

    //アイドル状態
	float idleTimer;                    // 触っていない時間
    float idleDuration;                //この時間触られなかったらアイドル状態になる

    //音関連
    bool runNow = false;    //歩行中かどうか
    int attackSe = 0;    //攻撃音
    int transformSe = 0;    //変身音
    int humanRunSe = 0;      //人間移動音
    int shadowRunSe = 0;        //影移動音
};

void InitializePlayer01();
void FinalizePlayer01();
void UpdatePlayer01();
void DrawPlayer01();

Player& GetPlayer();
void TakeDamagePlayer(int damage);
void SetPlayerPos(Object_2D::Vec2 pos);

void ResetPlayer();
