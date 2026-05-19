// ===================================================
// collision.h 当たり判定
// 
// 制作者：				日付：2024
// ===================================================

#ifndef _COLLISION_H_
#define _COLLISION_H_
#include "main.h"
// ===================================================
// プロトタイプ宣言
// ===================================================
bool CheckBoxCollider(Float2 PosA, Float2 PosB, Float2 SizeA, Float2 SizeB);	// バウンディングボックスの当たり判定
bool CheckCircleCollider(Float2 PosA, Float2 PosB, float rA, float rB);		// バウンディングサークルの当たり判定

void CheckBoxColliderResolve(Float2* PosA, Float2 PosB, Float2 SizeA, Float2 SizeB);

bool CheckCollision_Object2D(const Object_2D& A, const Object_2D& B);

void ResolveCollision_AABB(
    Object_2D& A,
    const Object_2D& B,
    const Object_2D::Vec2& oldPosA
);

bool CheckCollisionSAT(Float2 PosA, Float2 PosB, Float2 SizeA, Polygon_* SizeB);
bool CheckCollision_Rect_Triangle(Float2 rectPos, Float2 rectSize, Float2 tri[3]);

bool CheckCollision_Objects(
	const Object_2D& A,
	const std::vector<Object_2D::Vec2>& vtA,
	const std::vector<uint16_t>& idxA,

	const Object_2D& B,
	const std::vector<Object_2D::Vec2>& vtB,
	const std::vector<uint16_t>& idxB);


Object_2D::Vec2 Sub(const Object_2D::Vec2& a, const Object_2D::Vec2& b);

// 2D叉积（结果是标量）
float Cross(const Object_2D::Vec2& a, const Object_2D::Vec2& b);

bool PointInTriangle(
	const Object_2D::Vec2& P,
	const Object_2D::Vec2& A,
	const Object_2D::Vec2& B,
	const Object_2D::Vec2& C);

bool CheckCollisionQuadVsMesh(
	const Object_2D& A,
	const Object_2D& B);

bool CheckCollisionMeshVsMesh(
	const Object_2D& A,
	const Object_2D& B);

bool CheckCollisionLineVsMesh(
	Object_2D::Vec2 L0,   // 線分の始点
	Object_2D::Vec2 L1,   // 線分の終点
	const Object_2D& B);

void ConvertUVtoModel(
	const Object_2D& obj,
	std::vector<Object_2D::Vec2>* outVerts = nullptr);


#endif