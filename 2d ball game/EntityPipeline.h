#pragma once

struct DynamicEntity;
struct PhysicsComponent;
struct RenderComponent;

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

class EntityPipeline
{


	void executeInput(f32 dt);

public:

	void execute(f32 dt);
};