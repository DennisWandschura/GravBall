#pragma once

#include <vxLib/math/Vector.h>

enum class EntityShapeType : u16
{
	Rectangle,
	Circle
};

struct EntityShape
{
	union
	{
		vx::float2 halfDim;
		vx::float2 radius;
	};
	EntityShapeType type;

	EntityShape() {}
	EntityShape(const vx::float2 &v, EntityShapeType t) :halfDim(v), type(t) {}
};