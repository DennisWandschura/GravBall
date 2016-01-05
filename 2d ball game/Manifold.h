#pragma once

struct PhysicsComponent;

#include <vxlib/math/Vector.h>

struct Manifold
{
	PhysicsComponent* c0;
	PhysicsComponent* c1;
	vx::float2a d;
	f32 depth;
	u32 padding;
};