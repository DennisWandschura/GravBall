#pragma once

#include <gamelib/GravityArea.h>

struct GravityAreaInvertForce : public DynamicGravityFunctor
{
	~GravityAreaInvertForce() {}

	void operator()(vx::float2a* currentForce) override
	{
		currentForce->x = -currentForce->x;
		currentForce->y = -currentForce->y;
	}
};