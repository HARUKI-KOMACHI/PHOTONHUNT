//  gridmap.cpp  グリッドマップ & A* 経路探索
#include "gridmap.h"
#include "wall_1.h"     // GetWalls()
#include "collision.h"  // ConvertUVtoModel()
#include "gamebg_1.h"   // GetGameBG()
#include <math.h>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <cassert>

// ── グローバルインスタンス ──────────────────────────────────
static GridMap g_GridMap;

//  内部ヘルパー
static int GridIndex(int x, int y)
{
    return y * g_GridMap.width + x;
}

// UV → ワールド座標変換（collision.cpp の UVtoWorld と同じロジック）
static Object_2D::Vec2 UVtoWorld(const Object_2D::Vec2& uv, const Object_2D& obj)
{
    // UV → ローカル
    float localX = (uv.x - 0.5f) * obj.Size.x;
    float localY = (uv.y - 0.5f) * obj.Size.y;

    // スケール適用
    localX *= obj.Transform.Scale.x;
    localY *= obj.Transform.Scale.y;

    // 回転適用
    float rad = obj.Transform.Rotation * 3.14159265f / 180.0f;
    float cosR = cosf(rad);
    float sinR = sinf(rad);
    float rx = localX * cosR - localY * sinR;
    float ry = localX * sinR + localY * cosR;

    // ワールド座標へ
    return Object_2D::Vec2(rx + obj.Transform.Position.x,
        ry + obj.Transform.Position.y);
}

// 指定ワールド矩形と重なるグリッドセルを通行不可にする
static void MarkRectImpassable(float worldMinX, float worldMinY,
    float worldMaxX, float worldMaxY)
{
    // ワールド矩形に対応するグリッド範囲を計算
    int gx0 = (int)floorf((worldMinX - g_GridMap.originX) / GRID_CELL_SIZE);
    int gy0 = (int)floorf((worldMinY - g_GridMap.originY) / GRID_CELL_SIZE);
    int gx1 = (int)floorf((worldMaxX - g_GridMap.originX) / GRID_CELL_SIZE);
    int gy1 = (int)floorf((worldMaxY - g_GridMap.originY) / GRID_CELL_SIZE);

    // 範囲クランプ
    gx0 = (std::max)(gx0, 0);
    gy0 = (std::max)(gy0, 0);
    gx1 = (std::min)(gx1, g_GridMap.width - 1);
    gy1 = (std::min)(gy1, g_GridMap.height - 1);

    for (int gy = gy0; gy <= gy1; ++gy)
        for (int gx = gx0; gx <= gx1; ++gx)
            g_GridMap.passable[GridIndex(gx, gy)] = false;
}

// 三角形の各セルへの焼き付け（グリッドセルをなめて三角形と重なるか判定）
// シンプルに AABB → 各セル中心の重心座標判定で行う
static bool PointInTriangle2D(float px, float py,
    float ax, float ay,
    float bx, float by,
    float cx, float cy)
{
    // 符号付き面積（同符号なら内側）
    auto sign = [](float p1x, float p1y, float p2x, float p2y,
        float p3x, float p3y) -> float
        {
            return (p1x - p3x) * (p2y - p3y) - (p2x - p3x) * (p1y - p3y);
        };

    float d1 = sign(px, py, ax, ay, bx, by);
    float d2 = sign(px, py, bx, by, cx, cy);
    float d3 = sign(px, py, cx, cy, ax, ay);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

static void MarkTriangleImpassable(float ax, float ay,
    float bx, float by,
    float cx, float cy)
{
    // 三角形の AABB を計算
    float minX = (std::min)({ ax, bx, cx });
    float minY = (std::min)({ ay, by, cy });
    float maxX = (std::max)({ ax, bx, cx });
    float maxY = (std::max)({ ay, by, cy });

    // 対応グリッド範囲
    int gx0 = (int)floorf((minX - g_GridMap.originX) / GRID_CELL_SIZE);
    int gy0 = (int)floorf((minY - g_GridMap.originY) / GRID_CELL_SIZE);
    int gx1 = (int)floorf((maxX - g_GridMap.originX) / GRID_CELL_SIZE);
    int gy1 = (int)floorf((maxY - g_GridMap.originY) / GRID_CELL_SIZE);

    gx0 = (std::max)(gx0, 0);
    gy0 = (std::max)(gy0, 0);
    gx1 = (std::min)(gx1, g_GridMap.width - 1);
    gy1 = (std::min)(gy1, g_GridMap.height - 1);

    for (int gy = gy0; gy <= gy1; ++gy)
    {
        for (int gx = gx0; gx <= gx1; ++gx)
        {
            // セルの中心ワールド座標
            float cx = g_GridMap.originX + (gx + 0.5f) * GRID_CELL_SIZE;
            float cy = g_GridMap.originY + (gy + 0.5f) * GRID_CELL_SIZE;

            if (PointInTriangle2D(cx, cy, ax, ay, bx, by, cx, cy))
                g_GridMap.passable[GridIndex(gx, gy)] = false;
        }
    }
}


void InitializeGridMap(float originX, float originY, int width, int height)
{
    g_GridMap.originX = originX;
    g_GridMap.originY = originY;
    g_GridMap.width = width;
    g_GridMap.height = height;
    g_GridMap.passable.assign(width * height, true);  // 全マス通行可でリセット
}

void BuildGridMapFromWalls(void)
{
    // 全マス通行可にリセット（再構築のため）
    std::fill(g_GridMap.passable.begin(), g_GridMap.passable.end(), true);

    std::vector<Wall>& walls = GetWalls();

    for (auto& wall : walls)
    {
        if (wall.usePolygonCollision && !wall.obj.vt.empty() && !wall.obj.Indices_vt.empty())
        {
            // ── ポリゴンコリジョン壁 ──────────────────────────
            // UV頂点をワールド座標に変換し、三角形ごとにグリッドへ焼き付ける

            // UV → ワールド座標リストを作成
            std::vector<Object_2D::Vec2> worldVerts;
            worldVerts.reserve(wall.obj.vt.size());
            for (const auto& uv : wall.obj.vt)
            {
                worldVerts.push_back(UVtoWorld(uv, wall.obj));
            }

            // 三角形リストをなめる
            for (size_t i = 0; i + 2 < wall.obj.Indices_vt.size(); i += 3)
            {
                uint16_t i0 = wall.obj.Indices_vt[i];
                uint16_t i1 = wall.obj.Indices_vt[i + 1];
                uint16_t i2 = wall.obj.Indices_vt[i + 2];

                MarkTriangleImpassable(
                    worldVerts[i0].x, worldVerts[i0].y,
                    worldVerts[i1].x, worldVerts[i1].y,
                    worldVerts[i2].x, worldVerts[i2].y
                );
            }
        }
        else
        {
            // ── 通常の矩形壁 ──────────────────────────────────
            float halfW = wall.obj.Size.x * 0.5f;
            float halfH = wall.obj.Size.y * 0.5f;
            float px = wall.obj.Transform.Position.x;
            float py = wall.obj.Transform.Position.y;

            MarkRectImpassable(px - halfW, py - halfH,
                px + halfW, py + halfH);
        }
    }
}

// 床コリジョン（GameBG の OBJ コリジョン）をグリッドに反映する
// BuildGridMapFromWalls() の後に呼ぶこと
void BuildGridMapFromBG(void)
{
    GameBG& bg = GetGameBG();
    const Object_2D& obj = bg.obj;

    // OBJ コリジョンがなければ何もしない
    if (obj.vt.empty() || obj.Indices_vt.empty()) return;

    // UV → ワールド座標リストを作成
    std::vector<Object_2D::Vec2> worldVerts;
    worldVerts.reserve(obj.vt.size());
    for (const auto& uv : obj.vt)
    {
        worldVerts.push_back(UVtoWorld(uv, obj));
    }

    // 三角形ごとにグリッドへ焼き付ける（壁と同じ処理）
    for (size_t i = 0; i + 2 < obj.Indices_vt.size(); i += 3)
    {
        uint16_t i0 = obj.Indices_vt[i];
        uint16_t i1 = obj.Indices_vt[i + 1];
        uint16_t i2 = obj.Indices_vt[i + 2];

        MarkTriangleImpassable(
            worldVerts[i0].x, worldVerts[i0].y,
            worldVerts[i1].x, worldVerts[i1].y,
            worldVerts[i2].x, worldVerts[i2].y
        );
    }
}

void FinalizeGridMap(void)
{
    g_GridMap.passable.clear();
    g_GridMap.width = 0;
    g_GridMap.height = 0;
}

GridMap& GetGridMap(void)
{
    return g_GridMap;
}

GridPos WorldToGrid(float wx, float wy)
{
    int gx = (int)floorf((wx - g_GridMap.originX) / GRID_CELL_SIZE);
    int gy = (int)floorf((wy - g_GridMap.originY) / GRID_CELL_SIZE);
    return { gx, gy };
}

Object_2D::Vec2 GridToWorld(const GridPos& gp)
{
    // セル中心のワールド座標
    float wx = g_GridMap.originX + (gp.x + 0.5f) * GRID_CELL_SIZE;
    float wy = g_GridMap.originY + (gp.y + 0.5f) * GRID_CELL_SIZE;
    return { wx, wy };
}

bool InBounds(int gx, int gy)
{
    return gx >= 0 && gx < g_GridMap.width && gy >= 0 && gy < g_GridMap.height;
}

bool InBounds(const GridPos& gp)
{
    return InBounds(gp.x, gp.y);
}

bool IsPassable(int gx, int gy)
{
    if (!InBounds(gx, gy)) return false;
    return g_GridMap.passable[GridIndex(gx, gy)];
}

bool IsPassable(const GridPos& gp)
{
    return IsPassable(gp.x, gp.y);
}

// ── HasClearLine（Bresenham ライントレース）────────────────────
bool HasClearLine(float wx1, float wy1, float wx2, float wy2)
{
    GridPos from = WorldToGrid(wx1, wy1);
    GridPos to = WorldToGrid(wx2, wy2);

    int x0 = from.x, y0 = from.y;
    int x1 = to.x, y1 = to.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        if (!IsPassable(x0, y0)) return false;
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return true;
}

// ── A* 経路探索 ─────────────────────────────────────────────

// ハッシュ（unordered_map のキーに使う）
struct GridPosHash
{
    size_t operator()(const GridPos& p) const
    {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 16);
    }
};

// ヒューリスティック：オクタイル距離（斜め移動コスト≒√2 の最良推定）
static float Heuristic(const GridPos& a, const GridPos& b)
{
    // マンハッタン距離（4方向移動に最適なヒューリスティック）
    return (float)(abs(a.x - b.x) + abs(a.y - b.y));
}

bool FindPath(const GridPos& start, const GridPos& goal,
    std::vector<GridPos>& result)
{
    result.clear();

    if (!IsPassable(start) || !IsPassable(goal)) return false;
    if (start == goal) return true;

    // オープンリスト：{f値, GridPos}、f値が小さい順に取り出す
    using Element = std::pair<float, GridPos>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> openList;

    std::unordered_map<GridPos, float, GridPosHash> gCost;   // 実コスト
    std::unordered_map<GridPos, GridPos, GridPosHash> cameFrom; // 経路復元用

    gCost[start] = 0.0f;
    openList.push({ Heuristic(start, goal), start });

    // 隣接方向：4方向のみ（上下左右）
    const int DX[] = { 1, -1,  0,  0 };
    const int DY[] = { 0,  0,  1, -1 };

    while (!openList.empty())
    {
        auto [fCur, current] = openList.top();
        openList.pop();

        // ゴール到達
        if (current == goal)
        {
            // cameFrom を逆にたどって経路を復元
            GridPos node = goal;
            while (node != start)
            {
                result.push_back(node);
                node = cameFrom[node];
            }
            std::reverse(result.begin(), result.end());
            return true;
        }

        for (int i = 0; i < 4; ++i)
        {
            GridPos next{ current.x + DX[i], current.y + DY[i] };

            if (!IsPassable(next)) continue;

            float newG = gCost[current] + 1.0f;  // 4方向は全て移動コスト1

            auto it = gCost.find(next);
            if (it == gCost.end() || newG < it->second)
            {
                gCost[next] = newG;
                cameFrom[next] = current;
                openList.push({ newG + Heuristic(next, goal), next });
            }
        }
    }

    return false;  // 経路なし
}