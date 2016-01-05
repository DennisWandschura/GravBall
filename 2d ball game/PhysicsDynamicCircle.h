#pragma once

#include <gamelib/Circle.h>

namespace Physics
{
	struct DynamicCircle
	{
		Circle128C cirlce;
		__m128 prevPosition;
	};
}