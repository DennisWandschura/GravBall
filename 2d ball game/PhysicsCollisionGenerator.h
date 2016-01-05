#pragma once

struct Manifold;
struct PhysicsComponent;
struct AABB128;

namespace vx
{
	class LinearAllocator;
}

namespace Allocator
{
	class MainAllocator;
}



#include <vxLib/Container/array.h>
#include <gamelib/DebugAllocator.h>

namespace Physics
{
	struct DynamicCircle;

	struct CollisionGeneratorDesc
	{
		const vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>* dynamicCircleComponents;
		const vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>* staticRectComponents;
		const vx::array<Physics::DynamicCircle, vx::DelegateAllocator<vx::LinearAllocator>>* dynamicCircles;
		const vx::array<AABB128, DebugAllocatorPhysics<Allocator::MainAllocator>>* staticRects;
		vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>>* manifolds;
	};

	class CollisionGenerator
	{
	public:
		//static void generate(const vx::array<PhysicsComponent, MainAllocator> &dynamicCircle, const vx::array<PhysicsComponent, MainAllocator> &staticCircle, vx::array<Manifold, vx::LinearAllocator>* manifolds);

		static void generate_dynamicCircle_staticRect(const CollisionGeneratorDesc &desc);
		static void generate_dynamicCircle_dynamicCircle(const CollisionGeneratorDesc &desc);
	};
}