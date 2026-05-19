#pragma once
#include "sprite.h"
#include <vector>

// ============================================================
//  gridmap.h  グリッドマップ & A* 経路探索
//
//  使用の流れ:
//   1. InitGridMap()      でグリッドを初期化
//   2. BuildGridMapFromWalls() で壁からグリッドに通行不可を焼き付ける
//   3. FindPath() / HasClearLine() を敵AIから呼ぶ
// ============================================================

// ── グリッド1マスのワールド単位サイズ ────────────────────────
// 敵のサイズや壁の細かさに合わせて調整してください
// 小さいほど精度が上がるが、メモリ・探索コストも増える
static constexpr float GRID_CELL_SIZE = 20.0f;

// ── グリッド上の整数座標 ────────────────────────────────────
struct GridPos
{
    int x, y;
    bool operator==(const GridPos& o) const { return x == o.x && y == o.y; }
    bool operator!=(const GridPos& o) const { return !(*this == o); }
    bool operator< (const GridPos& o) const { return (y != o.y) ? (y < o.y) : (x < o.x); }
};

// ── グリッドマップ本体 ──────────────────────────────────────
struct GridMap
{
    int   width;    // 横セル数
    int   height;   // 縦セル数
    float originX;  // グリッド左上のワールドX
    float originY;  // グリッド左上のワールドY

    // true = 通行可, false = 壁(通行不可)
    // アクセス: passable[y * width + x]
    std::vector<bool> passable;
};

// ── 初期化・構築 ────────────────────────────────────────────
// originX,Y : グリッド左上のワールド座標
// width,height : セル数（ワールド幅 / GRID_CELL_SIZE を目安に）
void InitializeGridMap(float originX, float originY, int width, int height);

// wall_1.h の GetWalls() を参照して通行不可セルを設定する
// ※ 壁の初期化が完了した後に呼ぶこと
void BuildGridMapFromWalls(void);

// gamebg_1.h の GetGameBG() を参照して通行不可セルを設定する
// ※ BuildGridMapFromWalls() の後に呼ぶこと
void BuildGridMapFromBG(void);

void FinalizeGridMap(void);

// グローバルなグリッドマップを取得
GridMap& GetGridMap(void);

// ── 座標変換 ────────────────────────────────────────────────
GridPos         WorldToGrid(float wx, float wy);
Object_2D::Vec2 GridToWorld(const GridPos& gp);   // セル中心のワールド座標を返す

// ── 通行・範囲判定 ──────────────────────────────────────────
bool InBounds(int gx, int gy);
bool InBounds(const GridPos& gp);
bool IsPassable(int gx, int gy);
bool IsPassable(const GridPos& gp);

// ── 視線チェック（Bresenham ライントレース）─────────────────
// 2点間のグリッドを線でなぞり、途中に通行不可があればfalse
bool HasClearLine(float wx1, float wy1, float wx2, float wy2);

// ── A* 経路探索 ─────────────────────────────────────────────
// result : 見つかった経路（startを除くgoalまでの GridPos リスト、順番通り）
// 戻り値 : 経路が見つかれば true
bool FindPath(const GridPos& start, const GridPos& goal,
    std::vector<GridPos>& result);
