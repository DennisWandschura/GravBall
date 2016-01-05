#include "PhysicsCollisionGenerator.h"
#include <gamelib/PhysicsComponent.h>
#include "Manifold.h"
#include "Collision.h"
#include "PhysicsDynamicCircle.h"

namespace Physics
{
	int testSphereAABB(const Circle128 &circle, const AABB128 &aabb, __m128* penetrationDepth, __m128* direction)
	{
		auto p = circle.p;
		auto r = circle.r;

		auto v = p;
		v = vx::max(v, aabb.min);
		v = vx::min(v, aabb.max);

		//*intersectionPoint = v;

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

	int testMovingSphereStaticAABBImpl(const __m128 &p0, const __m128 &p1, const __m128 &r, const AABB128 &aabb, __m128* penetrationDepth, __m128* direction)
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

		if (testSphereAABB(circle, aabb, penetrationDepth, direction) == 0)
			return 0;

		auto cmp = _mm_cmplt_ss(distance, EPSILON);
		auto mask = _mm_movemask_ps(cmp);
		if ((mask & 0x1) != 0)
		{
			return 1;
		}

		auto newP1 = _mm_add_ps(p0, halfOffset);

		if (testMovingSphereStaticAABBImpl(p0, newP1, r, aabb, penetrationDepth, direction) != 0)
		{
			return 1;
		}

		return testMovingSphereStaticAABBImpl(newP1, p1, r, aabb, penetrationDepth, direction);
	}

	int testMovingSphereStaticAABB(const Physics::DynamicCircle &dynCircle, const AABB128 &aabb, __m128* penetrationDepth, __m128* direction)
	{
		auto p1 = dynCircle.cirlce.getCenter();
		auto r = dynCircle.cirlce.getRadius();
		auto p0 = dynCircle.prevPosition;

		Circle128 circle;
		circle.p = p0;
		circle.r = r;
		if (testSphereAABB(circle, aabb, penetrationDepth, direction) != 0)
			return 1;

		return testMovingSphereStaticAABBImpl(p0, p1, r, aabb, penetrationDepth, direction);
	}

	int intersectRaySphere(__m128 p, __m128 d, const Circle128 &s, __m128* t, __m128* intersectionPoint)
	{
		auto m = _mm_sub_ps(p, s.p);
		auto b = vx::dot2(m, d);
		auto c = _mm_sub_ps(vx::dot2(m, m), _mm_mul_ps(s.r, s.r));

		auto zero = vx::g_VXZero;
		auto cmp_b = _mm_cmpgt_ss(b, zero);
		auto cmp_c = _mm_cmpgt_ss(c, zero);

		auto mask_b = _mm_movemask_ps(cmp_b) & 0x1;
		auto mask_c = _mm_movemask_ps(cmp_c) & 0x1;
		if (mask_c && mask_b)
		{
			return 0;
		}

		auto discr = _mm_sub_ps(_mm_mul_ps(b, b), c);
		auto cmp_discr = _mm_cmplt_ss(discr, zero);
		auto mask_discr = _mm_movemask_ps(cmp_discr) & 0x1;
		if (mask_discr)
		{
			return 0;
		}

		auto tmp = _mm_sub_ps(vx::negate(b), _mm_sqrt_ps(discr));
		auto cmp_t = _mm_cmplt_ss(tmp, zero);
		auto mask_t = _mm_movemask_ps(cmp_t) & 0x1;
		if (mask_t)
		{
			tmp = zero;
		}

		*t = tmp;
		*intersectionPoint = vx::fma(tmp,d,p);//;:_mm_add_ps(p, _mm_mul_ps(tmp, d));

		return 1;
	}

	int testMovingSphereMovingSphere(const Physics::DynamicCircle &s0, const Physics::DynamicCircle &s1, __m128* penetrationDepth, __m128* direction)
	{
		auto c0 = s0.cirlce.getCenter();
		auto r0 = s0.cirlce.getRadius();
		auto v0 = _mm_sub_ps(c0, s0.prevPosition);

		auto c1 = s1.cirlce.getCenter();
		auto r1 = s1.cirlce.getRadius();
		auto v1 = _mm_sub_ps(c1, s1.prevPosition);

		Circle128 sphere;
		sphere.p = c1;
		sphere.r = _mm_add_ps(r0, r1);

		auto v = _mm_sub_ps(v0, v1);
		auto vLen = vx::length2(v);
		auto dir = _mm_div_ps(v, vLen);

		__m128 q;
		__m128 t;
		if (intersectRaySphere(c0, dir, sphere, &t, &q))
		{
			auto result = (t.m128_f32[0] <= vLen.m128_f32[0]);
			if (result)
			{
				auto depth = vx::length2(_mm_sub_ps(q, c0));
				//*intersectionPoint = q;
				*penetrationDepth = depth;
				*direction = dir;
			}

			return result;
		}

		return 0;
	}

	void CollisionGenerator::generate_dynamicCircle_staticRect(const CollisionGeneratorDesc &desc)
	{
		auto &dynamicCircles = desc.dynamicCircles;
		auto &staticRects = desc.staticRects;
		auto manifolds = desc.manifolds;

		auto dynamicComponents = desc.dynamicCircleComponents->data();
		auto staticComponents = desc.staticRectComponents->data();

		auto circleCount = dynamicCircles->size();
		auto rectCount = staticRects->size();

		for (size_t i = 0; i < circleCount; ++i)
		{
			auto &circle = (*dynamicCircles)[i];

		//	__m128 intersectionPoint;
			__m128 depth;
			__m128 direction;
			for (size_t j = 0; j < rectCount; ++j)
			{
				auto &bounds = (*staticRects)[j];

				if (testMovingSphereStaticAABB(circle, bounds, &depth, &direction))
				{
					Manifold manifold;
					manifold.c0 = &dynamicComponents[i];
					manifold.c1 = &staticComponents[j];

					vx::storeFloat2a(&manifold.d, direction);
					//vx::storeFloat2a(&manifold.p, intersectionPoint);
					_mm_store_ss(&manifold.depth, depth);

					manifolds->push_back(manifold);
				}
			}
		}
	}

	void CollisionGenerator::generate_dynamicCircle_dynamicCircle(const CollisionGeneratorDesc &desc)
	{
		auto &dynamicCircles = desc.dynamicCircles;
		auto dynamicComponents = desc.dynamicCircleComponents->data();
		auto circleCount = dynamicCircles->size();
		auto manifolds = desc.manifolds;

		for (size_t i = 0; i < circleCount; ++i)
		{
			auto &it = (*dynamicCircles)[i];

			__m128 depth;
			__m128 direction;

			for (size_t k = i + 1; k < circleCount; ++k)
			{
				auto &other = (*dynamicCircles)[k];

				if (Physics::testMovingSphereMovingSphere(it, other, &depth, &direction))
				{
					Manifold manifold;
					manifold.c0 = &dynamicComponents[i];
					manifold.c1 = &dynamicComponents[k];

					vx::storeFloat2a(&manifold.d, direction);
					//vx::storeFloat2a(&manifold.p, intersectionPoint);
					_mm_store_ss(&manifold.depth, depth);

					manifolds->push_back(manifold);
				}
			}
		}
	}
}