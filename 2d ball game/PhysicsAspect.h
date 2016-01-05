#pragma once

struct Manifold;
struct Circle128C;
struct GravityWell;
class LevelFile;

namespace Physics
{
	struct DynamicCircle;
}

#include <gamelib/PhysicsAspectBase.h>
#include <vxLib/Container/array.h>
#include <vxLib/Allocator/LinearAllocator.h>
#include <gamelib/DebugAllocator.h>
#include <gamelib/AABB.h>

struct OnCollisionCallbackData
{
	__m128 dir;
	PhysicsComponent* c0;
	PhysicsComponent* c1;
};

class PhysicsAspect : public PhysicsAspectBase
{
	OnCollisionCallbackSignature m_onCollisionCallback;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_dynamicCircleComponents;
	vx::array<GravityArea, DebugAllocatorPhysics<Allocator::MainAllocator>> m_gravityAreas;
	vx::array<GravityWell, DebugAllocatorEntity<Allocator::MainAllocator>> m_gravityWells;
	vx::array<Circle128C, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticCircles;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticCircleComponents;
	vx::array<AABB128, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticRects;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticRectComponents;
	vx::LinearAllocator m_frameAllocator;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_tmpDynamicCircleComponents;

	void accumulateForces(f32 dt);
	void verlet(f32 dt);
	void generateCollisions(const vx::array<Physics::DynamicCircle, vx::DelegateAllocator<vx::LinearAllocator>> &dynamicCircles, vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>>* manifolds);
	void resolveCollisions(const __m128 &dtSquared, const __m128 &invDtSquared, const vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>> &manifolds, 
		vx::array<OnCollisionCallbackData, vx::DelegateAllocator<vx::LinearAllocator>>* callbackData);

	void reset();
	void loadLevel(const LevelFile &level);

public:
	PhysicsAspect();
	~PhysicsAspect();

	bool initialize(const PhysicsAspectBaseDesc &desc) override;
	void shutdow(Allocator::MainAllocator* allocator) override;

	void update(f32 dt) override;

	PhysicsComponent* updateComponent(const PhysicsComponent &old) override;

	PhysicsComponent* addStaticShape(const vx::float2a &p, f32 radius, f32 restitution, void* userData) override;
	PhysicsComponent* addStaticShape(const vx::float2a &p, const vx::float2a &halfDim, f32 restitution, void* userData) override;
	PhysicsComponent* addDynamicShape(const vx::float2a &p, f32 radius, f32 mass, f32 restitution, void* userData) override;

	GravityArea* getGravityArea(size_t i) override;

	void handleEvent(const Event::Event &evt) override;
};