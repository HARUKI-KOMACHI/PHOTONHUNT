// =========================================================
// game.h ゲームシーン制御
// 
// 制作者:		日付：
// =========================================================
#ifndef _GAME_H_
#define _GAME_H_

// =========================================================
// マクロ宣言
// =========================================================

enum GAMESCENE
{
	START,
	PLAY
};


enum class GameState
{
    Playing,        // 正常游戏中
    Inventory,      // 打开背包
    MapView,        // 查看地图 / 切换地图
    GameOver,       // 食物耗尽 / 失败惩罚
};


enum class TurnPhase
{
    PlayerSelect,      // 玩家选择阶段（移动 / 攻击 / 潜入 / 调查）
    PlayerExecute,     // 玩家执行阶段（动画 / 实际移动 / 攻击）
    EnemyAndLogic,     // 敌人 + 环境逻辑一起处理
    ResultCheck,       // 结果判定（战斗 / 探索 / 撤离）
    TurnEnd,           // 回合结束（食物消耗、状态更新）
};

enum class PlayerActionType
{
    None,
    Move,
    Attack,
    Stealth,      // 潜入影子
    Investigate,  // 调查
    Evacuate,     // 撤离（如果你是主动选择）
};

enum class PlayerMode
{
    Normal,   // 正常状态
    Shadow,   // 影子潜行状态
};

void InitializeGame_0(void);
void UpdateGame_0(void);
void DrawGame_0(void);
void FinalizeGame_0(void);

float* GetGamecloA(void);

GAMESCENE GetGameScene(void);

int GetFraction(void);
#endif