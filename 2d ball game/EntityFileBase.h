#pragma once

#include "EntityShape.h"

struct EntityFileBase
{
	virtual f32 getRadius() const = 0;
	virtual vx::float2a getHalfDim() const = 0;
	virtual EntityShapeType getShapeType() const = 0;
	virtual const vx::float2a& getPosition() const = 0;
};