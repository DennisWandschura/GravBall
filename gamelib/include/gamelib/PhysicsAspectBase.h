#pragma once

namespace Allocator
{
	class MainAllocator;
}

struct PhysicsComponent;
struct GravityArea;

#include <vxLib/math/Vector.h>

typedef void (VX_CALLCONV *OnCollisionCallbackSignature)(__m128 dir, PhysicsComponent* c0, PhysicsComponent* c1);

#include <gamelib/EventListener.h>

struct PhysicsAspectBaseDesc
{
	OnCollisionCallbackSignature onCollisionCallback;
	Allocator::MainAllocator* allocator;
	u32 capacityDynamic;
	u32 capacityStatic;
	u32 capacityGravityAreas;
	u32 capacityGravityWell;
};

class PhysicsAspectBase : public Event::ListenerInterface
{
protected:
	PhysicsAspectBase() {}

public:
	virtual ~PhysicsAspectBase() {}

	virtual bool initialize(const PhysicsAspectBaseDesc &desc) = 0;
	virtual void shutdow(Allocator::MainAllocator* allocator) = 0;

	virtual void update(f32 dt) = 0;

	virtual PhysicsComponent* updateComponent(const PhysicsComponent &old) = 0;

	virtual PhysicsComponent* addStaticShape(const vx::float2a &p, f32 radius, f32 restitution, void* userData) = 0;
	virtual PhysicsComponent* addStaticShape(const vx::float2a &p, const vx::float2a &halfDim, f32 restitution, void* userData) = 0;
	virtual PhysicsComponent* addDynamicShape(const vx::float2a &p, f32 radius, f32 mass, f32 restitution, void* userData) = 0;

	virtual GravityArea* getGravityArea(size_t i) = 0;

	virtual void handleEvent(const Event::Event &evt) = 0;
};