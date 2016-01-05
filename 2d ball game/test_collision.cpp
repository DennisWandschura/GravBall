#include "test_collision.h"
#include <gamelib/Circle.h>
#include <gamelib/AABB.h>

int testSphereAABB(const Circle128 &circle, const AABB128 &aabb, __m128* intersectionPoint, __m128* penetrationDepth, __m128* direction)
{
	auto p = circle.p;
	auto r = circle.r;

	auto v = p;
	v = vx::max(v, aabb.min);
	v = vx::min(v, aabb.max);

	*intersectionPoint = v;

	v = _mm_sub_ps(v, p);
	auto dp = vx::dot2(v, v);

	auto cmp = _mm_cmple_ps(dp, r);
	auto mask = _mm_movemask_ps(cmp);
	bool result = (mask & 0x1);
	if (result)
	{
		auto distance = _mm_sqrt_ps(dp);
		auto depth = _mm_sub_ps(r, distance);

		*penetrationDepth = depth;
		*direction = vx::normalize2(v);
	}

	return (mask & 0x1);
}

int testMovingSphereStaticAABBImpl(const __m128 &p0, const __m128 &p1, const __m128 &r, const AABB128 &aabb, __m128* intersectionPoint, __m128* penetrationDepth, __m128* direction)
{
	static const __m128 EPSILON = { 0.01f, 0.01f, 0.01f, 0.01f };
	auto offset = _mm_sub_ps(p1, p0);
	auto distance = vx::length2(offset);
	auto oneHalf = vx::g_VXOneHalf;

	auto halfOffset = _mm_mul_ps(offset, oneHalf);
	auto halfDistance = _mm_mul_ps(distance, oneHalf);

	Circle128 circle;
	circle.p = _mm_add_ps(p0, halfOffset);
	circle.r = _mm_add_ps(r, halfDistance);

	if (testSphereAABB(circle, aabb, intersectionPoint, penetrationDepth, direction) == 0)
		return 0;

	auto cmp = _mm_cmplt_ss(distance, EPSILON);
	auto mask = _mm_movemask_ps(cmp);
	if ((mask & 0x1) != 0)
	{
		return 1;
	}

	auto newP1 = _mm_add_ps(p0, halfOffset);

	if (testMovingSphereStaticAABBImpl(p0, newP1, r, aabb, intersectionPoint, penetrationDepth, direction) != 0)
	{
		return 1;
	}

	return testMovingSphereStaticAABBImpl(newP1, p1, r, aabb, intersectionPoint, penetrationDepth, direction);
}

bool test_collision()
{
	AABB128 bounds;
	bounds.min = { -1, -1, 0, 0 };
	bounds.max = { 1, 1, 0, 0 };

	__m128 p0 = { -2, 0, 0, 0 };
	__m128 p1 = { 2, 0, 0, 0 };
	__m128 r = { 1, 1, 1, 1 };

	__m128 intersectionPoint; __m128 penetrationDepth; __m128 direction;

	auto result = (testMovingSphereStaticAABBImpl(p0, p1, r, bounds, &intersectionPoint, &penetrationDepth, &direction) != 0);

	return result;
}