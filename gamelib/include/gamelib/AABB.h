#pragma once

#include <vxLib/math/Vector.h>
#include <float.h>

struct AABB
{
	vx::float2a min;
	vx::float2a max;

	AABB() :min(FLT_MAX), max(-FLT_MAX) {}
	AABB(f32 minx, f32 miny, f32 maxx, f32 maxy) :min(minx, miny), max(maxx, maxy) {}
};

struct AABB128
{
	__m128 min;
	__m128 max;
};