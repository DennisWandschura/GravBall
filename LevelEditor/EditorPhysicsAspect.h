#pragma once

struct PhysicsComponent;

#include <gamelib/PhysicsAspectBase.h>
#include <vxLib/Container/array.h>
#include <gamelib/DebugAllocator.h>
#include <vxLib/math/matrix.h>

enum class EntityShapeType : u16;

class EditorPhysicsAspect : public PhysicsAspectBase
{
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_dynamicComponents;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticCircleComponents;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> m_staticRectComponents;

	void handleEditorEvent(const Event::Event &evt);

public:
	EditorPhysicsAspect();
	~EditorPhysicsAspect();

	bool initialize(const PhysicsAspectBaseDesc &desc) override;
	void shutdow(Allocator::MainAllocator* allocator) override;

	void update(f32 dt) override;

	PhysicsComponent* updateComponent(const PhysicsComponent &old) override { return nullptr; }

	PhysicsComponent* addStaticShape(const vx::float2a &p, f32 radius, f32 restitution, void* userData);
	PhysicsComponent* addStaticShape(const vx::float2a &p, const vx::float2a &halfDim, f32 restitution, void* userData);
	PhysicsComponent* addDynamicShape(const vx::float2a &p, f32 radius, f32 mass, f32 restitution, void* userData);

	GravityArea* getGravityArea(size_t i) { return nullptr; };

	void handleEvent(const Event::Event &evt) override;

	PhysicsComponent* getShape(int x, int y, const vx::float2 &resolution, const __m128 &cameraPosition, const vx::mat4 &viewMatrix, const vx::mat4 &projMatrix);
};