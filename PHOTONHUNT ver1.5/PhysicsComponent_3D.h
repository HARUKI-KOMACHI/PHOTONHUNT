#pragma once
#include "main.h"



enum class WorldUnit
{
	Millimeter,   // mm
	Centimeter,   // cm
	Decimeter,    // dm
	Meter,        // m
	Kilometer     // km
};


class PhysicsComponent_3D
{
public:
	PhysicsComponent_3D();
	~PhysicsComponent_3D();

public:
	// 绑定对象（用于访问位置）
	Object_3D* object = nullptr;

	// 质量（用于缩放力对速度的影响）
	float mass = 1.0f;

	// 密度（预留）
	float density = 1.0f;

	// 当前速度
	DirectX::XMFLOAT3 velocity = { 0, 0, 0 };

	// 当前加速度（中间计算用）
	DirectX::XMFLOAT3 acceleration = { 0, 0, 0 };

	// 当前帧累积的力（持续力）
	DirectX::XMFLOAT3 force = { 0, 0, 0 };

	// 重力方向与大小（直接使用向量，不做开关）
	DirectX::XMFLOAT3 gravity = { 0, -9.8f, 0 };

	// 阻力系数（简化）
	float drag = 0.5f;

	// 轴向锁定
	bool lockX = false;
	bool lockY = false;
	bool lockZ = false;

	// 旋转锁定（预留）
	bool lockRotation = true;
	//世界单位
	WorldUnit worldUnit = WorldUnit::Decimeter;


public:
	// 添加持续力（每帧可调用）
	void AddForce(const DirectX::XMFLOAT3& force);

	//// 添加瞬时力（直接改变速度）
	//void AddImpulse(const DirectX::XMFLOAT3& impulse);

	// 每帧更新
	void Update();

public:
	// 物理内部时间（只累计，不计算 dt）
	std::chrono::steady_clock::time_point  time;

	bool initialized = false;

};


inline float GetWorldUnitScale(WorldUnit unit);