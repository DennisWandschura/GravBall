#pragma once

#include <gamelib/AABB.h>
#include <gamelib/Circle.h>

namespace Physics
{
	inline void closestPointAABB(const vx::float2a &p, const AABB &b, vx::float2a* q)
	{
		auto v = p;
		v = vx::max(v, b.min);
		v = vx::min(v, b.max);

		*q = v;
	}

	inline f32 sqDistPointAABB(const vx::float2a &p, const AABB &b)
	{
		auto vmin = vx::min(p, b.min);
		auto vmax = vx::max(p, b.max);

		vmin = b.min - vmin;
		vmax = vmax - b.max;

		return vx::dot2(vmin, vmin) + vx::dot2(vmax, vmax);
	}

	inline bool intersect(const AABB &b, const Circle &s)
	{
		auto sqDist = sqDistPointAABB(s.p, b);

		return sqDist <= s.r * s.r;
	}

	inline bool VX_CALLCONV intersect(const __m128 bmin, const __m128 bmax, const __m128 p, const __m128 r, __m128* intersectionPoint, __m128* penetrationDepth, __m128* direction)
	{
		auto v = p;
		v = vx::max(v, bmin);
		v = vx::min(v, bmax);

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

	inline bool VX_CALLCONV intersect(const AABB &b, const Circle &s, vx::float2a* intersectionPoint, vx::float2a* penetrationDepth, vx::float2a* direction)
	{
		auto bmin = vx::loadFloat2a(&b.min);
		auto bmax = vx::loadFloat2a(&b.max);

		auto center = vx::loadFloat2a(&s.p);
		__m128 radius = { s.r, s.r, s.r, s.r };

		__m128 vintersectionPoint;
		__m128 vpenetrationDepth;
		__m128 vdirection;

		auto result = intersect(bmin, bmax, center, radius,&vintersectionPoint, &vpenetrationDepth, &vdirection);
		if(result)
		{
			vx::storeFloat2a(intersectionPoint, vintersectionPoint);
			vx::storeFloat2a(penetrationDepth, vpenetrationDepth);
			vx::storeFloat2a(direction, vdirection);
		}

		return result;
	}

	inline bool VX_CALLCONV intersectCircleCircle(__m128 p0, __m128 r0, __m128 p1, __m128 r1)
	{
		auto directionToP1 = _mm_sub_ps(p1, p0);
		auto dist2 = vx::dot2(directionToP1, directionToP1);

		auto radius_sum = _mm_add_ps(r0, r1);
		radius_sum = _mm_mul_ps(radius_sum, radius_sum);

		auto cmp = _mm_cmple_ss(dist2, radius_sum);
		auto mask = _mm_movemask_ps(cmp);

		return (mask & 0x1);;
	}

	inline bool VX_CALLCONV intersectCircleCircle(__m128 p0, __m128 r0, __m128 p1, __m128 r1, __m128* intersectionPoint, __m128* penetrationDepth, __m128* directionOut)
	{
		auto directionToP1 = _mm_sub_ps(p1, p0);
		auto dist2 = vx::dot2(directionToP1, directionToP1);

		auto radius_sum = _mm_add_ps(r0, r1);
		radius_sum = _mm_mul_ps(radius_sum, radius_sum);

		auto cmp = _mm_cmple_ss(dist2, radius_sum);
		auto mask = _mm_movemask_ps(cmp);
		bool result = (mask & 0x1);
		if (result)
		{
			auto dist = _mm_sqrt_ps(dist2);
			directionToP1 = _mm_div_ps(directionToP1, dist);

			radius_sum = _mm_add_ps(r0, r1);

			auto depth = _mm_sub_ps(dist, radius_sum);
			//printf("%f %f\n",dist.m128_f32[0], depth.m128_f32[0]);

			*penetrationDepth = depth;
			*intersectionPoint = _mm_add_ps(p0, _mm_mul_ps(directionToP1, _mm_sub_ps(r0, depth)));
			*directionOut = directionToP1;
		}

		return result;
	}
}