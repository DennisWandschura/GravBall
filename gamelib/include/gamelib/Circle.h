#pragma once

#include <vxLib/math/Vector.h>

struct Circle
{
	vx::float2a p;
	f32 r;
	f32 padding;

	Circle() :p(), r() {}
};

struct Circle128
{
	__m128 p;
	__m128 r;
};

struct Circle128C
{
	__m128 p_r;

	inline __m128 VX_CALLCONV getCenter() const
	{
		return VX_PERMUTE_PS(p_r, _MM_SHUFFLE(1, 0, 1, 0));
	}

	inline __m128 VX_CALLCONV getRadius() const
	{
		return VX_PERMUTE_PS(p_r, _MM_SHUFFLE(2, 2, 2, 2));
	}
};