#include "PhysicsComponent_3D.h"
#include <chrono> // 引入 chrono 进行高精度计时

using namespace std::chrono;

PhysicsComponent_3D::PhysicsComponent_3D()
{
}

PhysicsComponent_3D::~PhysicsComponent_3D()
{
}


void PhysicsComponent_3D::Update()
{
	if (!object) return;

	const auto old = time; // 备份旧的时间点
	time = steady_clock::now(); // 记录新的时间点

	// 第一次调用：只初始化时间，不做物理
	if (!initialized)
	{
		initialized = true;
		return;
	}


	const duration<float> frameTime = time - old; // 计算时间差（单位：秒）

	float deltaTime = frameTime.count();
	deltaTime = (std::min)(deltaTime, 1.0f / 60.0f);

	// 1. 重力作为持续力加入
	float unitScale = GetWorldUnitScale(worldUnit);

	force.x += gravity.x * mass * unitScale;
	force.y += gravity.y * mass * unitScale;
	force.z += gravity.z * mass * unitScale;


	// 2. 由力计算加速度 a = F / m
	acceleration.x = force.x / mass;
	acceleration.y = force.y / mass;
	acceleration.z = force.z / mass;

	// 3. 速度变化 v += a * dt
	velocity.x += acceleration.x * deltaTime;
	velocity.y += acceleration.y * deltaTime;
	velocity.z += acceleration.z * deltaTime;

	// 4. 阻力（简化，直接反向削减速度）
	velocity.x -= velocity.x * drag * deltaTime;
	velocity.y -= velocity.y * drag * deltaTime;
	velocity.z -= velocity.z * drag * deltaTime;

	// 5. 轴向锁定
	if (lockX) velocity.x = 0.0f;
	if (lockY) velocity.y = 0.0f;
	if (lockZ) velocity.z = 0.0f;

	// 6. 更新位置 p += v * dt
	object->Transform.Position.x += velocity.x * deltaTime;
	object->Transform.Position.y += velocity.y * deltaTime;
	object->Transform.Position.z += velocity.z * deltaTime;

	// 7. 清空本帧持续力
	force = { 0, 0, 0 };
}



void PhysicsComponent_3D::AddForce(const DirectX::XMFLOAT3& worldForce)
{
	float unitScale = GetWorldUnitScale(worldUnit);
	// 累加持续力（每帧可多次调用）
	force.x += worldForce.x * unitScale;
	force.y += worldForce.y * unitScale;
	force.z += worldForce.z * unitScale;
}


inline float GetWorldUnitScale(WorldUnit unit)
{
	switch (unit)
	{
	case WorldUnit::Millimeter: return 1000.0f;   // 1 m = 1000 mm
	case WorldUnit::Centimeter: return 100.0f;    // 1 m = 100 cm
	case WorldUnit::Decimeter:  return 10.0f;     // 1 m = 10 dm
	case WorldUnit::Meter:      return 1.0f;      // 1 m = 1 m
	case WorldUnit::Kilometer:  return 0.001f;    // 1 m = 0.001 km
	default: return 1.0f;
	}
}

