// ===================================================
// collision.cpp 当たり判定
// 
// 制作者：				日付：2024
// ===================================================
#include "main.h"
#include "collision.h"

// ===================================================
// バウンディングボックスの当たり判定
// 
// 引数:
// 	矩形Ａの中心座標
// 	矩形Ｂの中心座標
// 	矩形Ａのサイズ
// 	矩形Ｂのサイズ
//
// 戻り値
//  true：当たっている
// 	false：当たっていない
// ===================================================
bool CheckBoxCollider(Float2 PosA, Float2 PosB, Float2 SizeA, Float2 SizeB)
{
	float ATop =	PosA.y - SizeA.y / 2;	// Aの上端
	float ABottom =	PosA.y + SizeA.y / 2;	// Aの下端
	float ARight =	PosA.x + SizeA.x / 2;	// Aの右端
	float ALeft =	PosA.x - SizeA.x / 2;	// Aの左端

	float BTop =	PosB.y - SizeB.y / 2;	// Bの上端
	float BBottom =	PosB.y + SizeB.y / 2;	// Bの下端
	float BRight =	PosB.x + SizeB.x / 2;	// Bの右端
	float BLeft =	PosB.x - SizeB.x / 2;	// Bの左端

	if ((ARight >= BLeft) &&		// Aの右端 > Bの左端
		(ALeft <= BRight) &&		// Aの左端 < Bの右端
		(ABottom >= BTop) &&		// Aの下端 > Bの上端
		(ATop <= BBottom))			// Aの上端 < Bの下端
	{
		// 当たっている
		return true;
	}

	// 当たっていない
	return false;
}

void CheckBoxColliderResolve(Float2* PosA, Float2 PosB, Float2 SizeA, Float2 SizeB)
{
	Float2 oldpos = *PosA;
	
}

// ===================================================
// バウンディングサークルの当たり判定
// 
// 引数:
// 	円１の中心座標
// 	円２の中心座標
// 	円１の半径
// 	円２の半径
//
// 戻り値
//  true：当たっている
// 	false：当たっていない
// ===================================================
bool CheckCircleCollider(Float2 PosA, Float2 PosB, float rA, float rB)
{
	// (円Aの中心座標X - 円Bの中心座標X)の2乗 + (円Aの中心座標Y - 円Bの中心座標Y)の2乗
	float distance = (PosA.x - PosB.x) * (PosA.x - PosB.x) + (PosA.y - PosB.y) * (PosA.y - PosB.y);
	
	// (円1の半径+円2の半径)の2乗
	float rSum = (rA + rB) * (rA + rB);

	// 2つの円の距離が半径の合計を下回った時
	if (distance <= rSum)
	{
		// 当たっている
		return true;
	}

	// 当たっていない
	return false;
}



Float2 GetNormal(Float2 a, Float2 b) {
    Float2 edge = { b.x - a.x, b.y - a.y };
    return { -edge.y, edge.x };
}

void Project(const Float2* points, int count, Float2 axis, float& min, float& max) {
    min = max = points[0].x * axis.x + points[0].y * axis.y;
    for (int i = 1; i < count; ++i) {
        float p = points[i].x * axis.x + points[i].y * axis.y;
        if (p < min) min = p;
        if (p > max) max = p;
    }
}

bool CheckCollision_Rect_Triangle(Float2 rectPos, Float2 rectSize, Float2 tri[3]) {
    Float2 half = { rectSize.x / 2.0f, rectSize.y / 2.0f };

    Float2 rect[4] = {
        { rectPos.x - half.x, rectPos.y - half.y },
        { rectPos.x + half.x, rectPos.y - half.y },
        { rectPos.x + half.x, rectPos.y + half.y },
        { rectPos.x - half.x, rectPos.y + half.y }
    };

    Float2 axes[5] = {
        GetNormal(rect[0], rect[1]),
        GetNormal(rect[1], rect[2]),
        GetNormal(tri[0], tri[1]),
        GetNormal(tri[1], tri[2]),
        GetNormal(tri[2], tri[0]),
    };

    for (int i = 0; i < 5; ++i) {
        float len = std::sqrt(axes[i].x * axes[i].x + axes[i].y * axes[i].y);
        if (len != 0.0f) {
            axes[i].x /= len;
            axes[i].y /= len;
        }

        float minA, maxA, minB, maxB;
        Project(rect, 4, axes[i], minA, maxA);
        Project(tri, 3, axes[i], minB, maxB);
        if (maxA < minB || maxB < minA)
            return false;
    }

    return true;
}

bool CheckCollisionSAT(Float2 PosA, Float2 PosB, Float2 SizeA, Polygon_* SizeB)
{
    int num = 0;
    int num_max = SizeB->faces.size() / 3;

    for (int i = 0;i < num_max;i++)
    {
        Float2 tri[3];

        for (int l = 0; l < 3; l++)
        {
            tri[l].x = SizeB->vertices[SizeB->faces[i * 3 + l]].x + GetScreenOffsetX();
            tri[l].y = SizeB->vertices[SizeB->faces[i * 3 + l]].y + GetScreenOffsetY();
        }

        if(CheckCollision_Rect_Triangle(PosA, PosB, tri))
        {
            return true;
        }
    }

    return false;
}

bool CheckCollision_Object2D(const Object_2D& A, const Object_2D& B)
{
    // ================================
    // 1️⃣ 顶点模式（支持旋转或自定义顶点）
    // ================================
    auto CheckOverlapByVertex = [](const Object_2D& objA, const Object_2D& objB)
        {
            // 检查四个角是否有任意点在对方多边形内
            auto PointInRect = [](const Object_2D& obj, float x, float y)
                {
                    // 使用“向量叉积”判断点是否在矩形内
                    for (int i = 0; i < 4; i++)
                    {
                        int j = (i + 1) % 4;
                        float edgeX = obj.Vertexs[j].x - obj.Vertexs[i].x;
                        float edgeY = obj.Vertexs[j].y - obj.Vertexs[i].y;
                        float toPointX = x - obj.Vertexs[i].x;
                        float toPointY = y - obj.Vertexs[i].y;
                        float cross = edgeX * toPointY - edgeY * toPointX;
                        if (cross > 0) return false; // 点在边的外侧
                    }
                    return true;
                };

            for (int i = 0; i < 4; i++)
            {
                if (PointInRect(objB, objA.Vertexs[i].x, objA.Vertexs[i].y)) return true;
                if (PointInRect(objA, objB.Vertexs[i].x, objB.Vertexs[i].y)) return true;
            }
            return false;
        };

    // ================================
    // 2️⃣ 尺寸模式（退化为AABB检测）
    // ================================
    auto CheckOverlapBySize = [](const Object_2D& objA, const Object_2D& objB)
        {
            float ATop = objA.Transform.Position.y - objA.Size.y / 2;
            float ABottom = objA.Transform.Position.y + objA.Size.y / 2;
            float ARight = objA.Transform.Position.x + objA.Size.x / 2;
            float ALeft = objA.Transform.Position.x - objA.Size.x / 2;

            float BTop = objB.Transform.Position.y - objB.Size.y / 2;
            float BBottom = objB.Transform.Position.y + objB.Size.y / 2;
            float BRight = objB.Transform.Position.x + objB.Size.x / 2;
            float BLeft = objB.Transform.Position.x - objB.Size.x / 2;

            return (ARight >= BLeft &&
                ALeft <= BRight &&
                ABottom >= BTop &&
                ATop <= BBottom);
        };

    // ================================
    // 3️⃣ 判定逻辑：是否需要顶点检测
    // ================================
    bool useVertex = false;
    for (int i = 0; i < 4; i++)
    {
        if (A.Transform.Rotation != 0.0f || B.Transform.Rotation != 0.0f)
        {
            useVertex = true;
            break;
        }
    }

    if (useVertex)
        return CheckOverlapByVertex(A, B);
    else
        return CheckOverlapBySize(A, B);
}


void ResolveCollision_AABB(
    Object_2D& A,
    const Object_2D& B,
    const Object_2D::Vec2& oldPosA
)
{
    if (CheckCollision_Object2D(A, B))
    {

        if (oldPosA.y + A.Size.y / 2 <= B.Transform.Position.y - B.Size.y / 2)
        {
            A.Transform.Position.y = B.Transform.Position.y - (A.Size.y / 2 + B.Size.y / 2) - 0.1f;

        }
        else if (oldPosA.y - A.Size.y / 2 >= B.Transform.Position.y + B.Size.y / 2)
        {
            A.Transform.Position.y = B.Transform.Position.y + (A.Size.y / 2 + B.Size.y / 2) + 0.1f;
        }

        //return;
    }

    if (CheckCollision_Object2D(A, B))
    {

        if (oldPosA.x + A.Size.x / 2 <= B.Transform.Position.x - B.Size.x / 2)
        {
            A.Transform.Position.x = B.Transform.Position.x - (A.Size.x / 2 + B.Size.x / 2) - 0.1f;
        }
        else if (oldPosA.x - A.Size.x / 2 >= B.Transform.Position.x + B.Size.x / 2)
        {
            A.Transform.Position.x = B.Transform.Position.x + (A.Size.x / 2 + B.Size.x / 2) + 0.1f;
        }

        //return;
    }
}


Object_2D::Vec2 Sub(const Object_2D::Vec2& a, const Object_2D::Vec2& b)
{
    return Object_2D::Vec2(a.x - b.x, a.y - b.y);
}

// 2D叉积（结果是标量）
float Cross(const Object_2D::Vec2& a, const Object_2D::Vec2& b)
{
    return a.x * b.y - a.y * b.x;
}


bool PointInTriangle(
    const Object_2D::Vec2& P,
    const Object_2D::Vec2& A,
    const Object_2D::Vec2& B,
    const Object_2D::Vec2& C)
{
    Object_2D::Vec2 AB = Sub(B, A);
    Object_2D::Vec2 BC = Sub(C, B);
    Object_2D::Vec2 CA = Sub(A, C);

    Object_2D::Vec2 AP = Sub(P, A);
    Object_2D::Vec2 BP = Sub(P, B);
    Object_2D::Vec2 CP = Sub(P, C);

    float c1 = Cross(AB, AP);
    float c2 = Cross(BC, BP);
    float c3 = Cross(CA, CP);

    // 若三个符号都 >= 0（或 <= 0），则点在三角形内
    bool hasNeg = (c1 < 0) || (c2 < 0) || (c3 < 0);
    bool hasPos = (c1 > 0) || (c2 > 0) || (c3 > 0);

    return !(hasNeg && hasPos);
}


bool SegmentsIntersect(
    const Object_2D::Vec2& A,
    const Object_2D::Vec2& B,
    const Object_2D::Vec2& C,
    const Object_2D::Vec2& D)
{
    Object_2D::Vec2 AB = B - A;
    Object_2D::Vec2 AC = C - A;
    Object_2D::Vec2 AD = D - A;

    Object_2D::Vec2 CD = D - C;
    Object_2D::Vec2 CA = A - C;
    Object_2D::Vec2 CB = B - C;

    float d1 = Cross(AB, AC);
    float d2 = Cross(AB, AD);
    float d3 = Cross(CD, CA);
    float d4 = Cross(CD, CB);

    // 严格相交（包含穿过）
    if ((d1 * d2 < 0.0f) && (d3 * d4 < 0.0f))
        return true;

    return false;
}


bool TriangleVsTriangle(
    const  Object_2D::Vec2& A0, const  Object_2D::Vec2& A1, const  Object_2D::Vec2& A2,
    const  Object_2D::Vec2& B0, const  Object_2D::Vec2& B1, const  Object_2D::Vec2& B2)
{
    // 1. A 的点在 B 里
    if (PointInTriangle(A0, B0, B1, B2)) return true;
    if (PointInTriangle(A1, B0, B1, B2)) return true;
    if (PointInTriangle(A2, B0, B1, B2)) return true;

    // 2. B 的点在 A 里
    if (PointInTriangle(B0, A0, A1, A2)) return true;
    if (PointInTriangle(B1, A0, A1, A2)) return true;
    if (PointInTriangle(B2, A0, A1, A2)) return true;

    // 3. 边相交
    if (SegmentsIntersect(A0, A1, B0, B1)) return true;
    if (SegmentsIntersect(A0, A1, B1, B2)) return true;
    if (SegmentsIntersect(A0, A1, B2, B0)) return true;

    if (SegmentsIntersect(A1, A2, B0, B1)) return true;
    if (SegmentsIntersect(A1, A2, B1, B2)) return true;
    if (SegmentsIntersect(A1, A2, B2, B0)) return true;

    if (SegmentsIntersect(A2, A0, B0, B1)) return true;
    if (SegmentsIntersect(A2, A0, B1, B2)) return true;
    if (SegmentsIntersect(A2, A0, B2, B0)) return true;

    return false;
}

//SegmentsIntersect 没有
bool CheckCollisionQuadVsMesh(
    const Object_2D& A,
    const Object_2D& B)
{
    std::vector<Object_2D::Vec2> vtB;
    ConvertUVtoModel(B, &vtB);

    // Quad A 顶点（世界坐标）
    const Object_2D::Vec2& A0 = A.Vertexs[0];
    const Object_2D::Vec2& A1 = A.Vertexs[1];
    const Object_2D::Vec2& A2 = A.Vertexs[2];
    const Object_2D::Vec2& A3 = A.Vertexs[3];

    // A 拆成两个三角形
    const Object_2D::Vec2 A_tri[2][3] =
    {
        { A0, A1, A2 },
        { A0, A2, A3 }
    };

    // 遍历 B 的三角形
    for (size_t i = 0; i + 2 < B.Indices_vt.size(); i += 3)
    {
        if (i + 2 >= B.Indices_vt.size())continue;

        if (vtB.size() <= B.Indices_vt[i + 0])continue;
        if (vtB.size() <= B.Indices_vt[i + 1])continue;
        if (vtB.size() <= B.Indices_vt[i + 2])continue;
        Object_2D::Vec2 B0 = vtB[B.Indices_vt[i + 0]];
        Object_2D::Vec2 B1 = vtB[B.Indices_vt[i + 1]];
        Object_2D::Vec2 B2 = vtB[B.Indices_vt[i + 2]];

        // A 的两个三角形 vs 当前 B 三角形
        for (int t = 0; t < 2; t++)
        {
            if (TriangleVsTriangle(
                A_tri[t][0], A_tri[t][1], A_tri[t][2],
                B0, B1, B2))
            {
                return true;
            }
        }
    }
    return false;
}

bool CheckCollisionMeshVsMesh(
    const Object_2D& A,
    const Object_2D& B)
{
    // A, B 両方をUV座標からモデル座標へ変換
    std::vector<Object_2D::Vec2> vtA;
    std::vector<Object_2D::Vec2> vtB;
    ConvertUVtoModel(A, &vtA);
    ConvertUVtoModel(B, &vtB);

    // A の三角形リストを構築
    // Indices_vt が空の場合は従来通り Vertexs の Quad にフォールバック
    std::vector<std::array<Object_2D::Vec2, 3>> trisA;

    if (!A.Indices_vt.empty())
    {
        // メッシュとして三角形を列挙
        for (size_t i = 0; i + 2 < A.Indices_vt.size(); i += 3)
        {
            if (vtA.size() <= A.Indices_vt[i + 0]) continue;
            if (vtA.size() <= A.Indices_vt[i + 1]) continue;
            if (vtA.size() <= A.Indices_vt[i + 2]) continue;

            trisA.push_back(
                {
                    vtA[A.Indices_vt[i + 0]],
                    vtA[A.Indices_vt[i + 1]],
                    vtA[A.Indices_vt[i + 2]]
                });
        }
    }
    else
    {
        // フォールバック : Vertexs の Quad を2三角形に分割
        const Object_2D::Vec2& A0 = A.Vertexs[0];
        const Object_2D::Vec2& A1 = A.Vertexs[1];
        const Object_2D::Vec2& A2 = A.Vertexs[2];
        const Object_2D::Vec2& A3 = A.Vertexs[3];
        trisA.push_back({ A0, A1, A2 });
        trisA.push_back({ A0, A2, A3 });
    }

    // B の三角形と総当たり判定
    for (size_t i = 0; i + 2 < B.Indices_vt.size(); i += 3)
    {
        if (vtB.size() <= B.Indices_vt[i + 0]) continue;
        if (vtB.size() <= B.Indices_vt[i + 1]) continue;
        if (vtB.size() <= B.Indices_vt[i + 2]) continue;

        const Object_2D::Vec2 B0 = vtB[B.Indices_vt[i + 0]];
        const Object_2D::Vec2 B1 = vtB[B.Indices_vt[i + 1]];
        const Object_2D::Vec2 B2 = vtB[B.Indices_vt[i + 2]];

        for (const auto& triA : trisA)
        {
            if (TriangleVsTriangle(
                triA[0], triA[1], triA[2],
                B0, B1, B2))
            {
                return true;
            }
        }
    }
    return false;
}

// 直線(線分) vs メッシュ のコリジョン判定
bool CheckCollisionLineVsMesh(
    Object_2D::Vec2 L0,   // 線分の始点
    Object_2D::Vec2 L1,   // 線分の終点
    const Object_2D& B)
{
    std::vector<Object_2D::Vec2> vtB;
    ConvertUVtoModel(B, &vtB);

    // B のトライアングルをループ
    for (size_t i = 0; i + 2 < B.Indices_vt.size(); i += 3)
    {
        if (vtB.size() <= B.Indices_vt[i + 0]) continue;
        if (vtB.size() <= B.Indices_vt[i + 1]) continue;
        if (vtB.size() <= B.Indices_vt[i + 2]) continue;

        Object_2D::Vec2 B0 = vtB[B.Indices_vt[i + 0]];
        Object_2D::Vec2 B1 = vtB[B.Indices_vt[i + 1]];
        Object_2D::Vec2 B2 = vtB[B.Indices_vt[i + 2]];

        // ★ 線分 vs トライアングルの3辺 それぞれ交差判定
        if (SegmentsIntersect(L0, L1, B0, B1)) return true;
        if (SegmentsIntersect(L0, L1, B1, B2)) return true;
        if (SegmentsIntersect(L0, L1, B2, B0)) return true;
    }
    return false;
}


//void ConvertUVtoModel(
//    const Object_2D& obj,
//    std::vector<Object_2D::Vec2>* outVerts)
//{
//    if (outVerts)
//    {
//        outVerts->clear();
//        outVerts->reserve(obj.vt.size());
//
//        for (const auto& uv : obj.vt)
//        {
//            Object_2D::Vec2 out;
//            out.x = (uv.x - 0.5f) * obj.Size.x;
//            out.y = (uv.y - 0.5f) * obj.Size.y;
//            outVerts->push_back(out);
//        }
//    }
//   
//}

void ConvertUVtoModel(
    const Object_2D& obj,
    std::vector<Object_2D::Vec2>* outVerts)
{
    if (outVerts)
    {
        outVerts->clear();
        outVerts->reserve(obj.vt.size());
        for (const auto& uv : obj.vt)
        {
            Object_2D::Vec2 out;
            out.x = (uv.x - 0.5f) * obj.Size.x + obj.Transform.Position.x; // ★追加
            out.y = (uv.y - 0.5f) * obj.Size.y + obj.Transform.Position.y; // ★追加
            outVerts->push_back(out);
        }
    }
}

bool CheckCollision_Objects(
    const Object_2D& A,
    const std::vector<Object_2D::Vec2>& vtA,
    const std::vector<uint16_t>& idxA,

    const Object_2D& B,
    const std::vector<Object_2D::Vec2>& vtB,
    const std::vector<uint16_t>& idxB)
{
    auto UVtoWorld = [&](const Object_2D::Vec2& uv, const Object_2D& obj)
        {
            float localX = (uv.x - 0.5f) * obj.Size.x;
            float localY = (uv.y - 0.5f) * obj.Size.y;

            localX *= obj.Transform.Scale.x;
            localY *= obj.Transform.Scale.y;

            float rad = obj.Transform.Rotation * 3.14159265f / 180.0f;
            float cosR = cosf(rad);
            float sinR = sinf(rad);

            float rx = localX * cosR - localY * sinR;
            float ry = localX * sinR + localY * cosR;

            return Object_2D::Vec2(rx + obj.Transform.Position.x,
                ry + obj.Transform.Position.y);
        };

    auto TrianglesIntersect = [&](const Object_2D::Vec2& A0,
        const Object_2D::Vec2& A1,
        const Object_2D::Vec2& A2,
        const Object_2D::Vec2& B0,
        const Object_2D::Vec2& B1,
        const Object_2D::Vec2& B2)
        {
            auto project = [&](const Object_2D::Vec2& ax, const Object_2D::Vec2& p)
                {
                    return ax.x * p.x + ax.y * p.y;
                };

            auto genAxes = [&](const Object_2D::Vec2& p0,
                const Object_2D::Vec2& p1,
                const Object_2D::Vec2& p2)
                {
                    std::vector<Object_2D::Vec2> out;
                    out.push_back({ p1.y - p0.y, -(p1.x - p0.x) });
                    out.push_back({ p2.y - p1.y, -(p2.x - p1.x) });
                    out.push_back({ p0.y - p2.y, -(p0.x - p2.x) });
                    return out;
                };

            auto axes = genAxes(A0, A1, A2);
            auto baxes = genAxes(B0, B1, B2);
            axes.insert(axes.end(), baxes.begin(), baxes.end());

            for (auto& axis : axes)
            {
                float aMin = project(axis, A0), aMax = aMin;
                aMin = (std::min)(aMin, project(axis, A1)); aMax = (std::max)(aMax, project(axis, A1));
                aMin = (std::min)(aMin, project(axis, A2)); aMax = (std::max)(aMax, project(axis, A2));

                float bMin = project(axis, B0), bMax = bMin;
                bMin = (std::min)(bMin, project(axis, B1)); bMax = (std::max)(bMax, project(axis, B1));
                bMin = (std::min)(bMin, project(axis, B2)); bMax = (std::max)(bMax, project(axis, B2));

                if (aMax < bMin || bMax < aMin)
                    return false;
            }
            return true;
        };

    // -------- 遍历 A/B 所有三角形，检查是否有任意一对碰撞 ----------
    for (size_t i = 0; i < idxA.size(); i += 3)
    {
        Object_2D::Vec2 A0 = UVtoWorld(vtA[idxA[i]], A);
        Object_2D::Vec2 A1 = UVtoWorld(vtA[idxA[i + 1]], A);
        Object_2D::Vec2 A2 = UVtoWorld(vtA[idxA[i + 2]], A);

        for (size_t j = 0; j < idxB.size(); j += 3)
        {
            Object_2D::Vec2 B0 = UVtoWorld(vtB[idxB[j]], B);
            Object_2D::Vec2 B1 = UVtoWorld(vtB[idxB[j + 1]], B);
            Object_2D::Vec2 B2 = UVtoWorld(vtB[idxB[j + 2]], B);

            if (TrianglesIntersect(A0, A1, A2, B0, B1, B2))
                return true;
        }
    }

    return false;
}