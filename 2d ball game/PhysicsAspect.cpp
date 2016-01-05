#include "PhysicsAspect.h"
#include <gamelib/MainAllocator.h>
#include <vxLib/algorithm.h>
#include <gamelib/PhysicsComponent.h>
#include "PhysicsCollisionGenerator.h"
#include "Manifold.h"
#include "Collision.h"
#include <gamelib/Event.h>
#include "Locator.h"
#include "ResourceAspect.h"
#include <gamelib/LevelFile.h>
#include <gamelib/GravityArea.h>
#include <gamelib/GravityWell.h>
#include <vxLib/print.h>
#include "PhysicsDynamicCircle.h"
#include "StaticGrid.h"
#include "EventManager.h"

namespace PhysicsAspectCpp
{
	template<typename AllocatorType>
	void buildCircles(const vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> &circleComponents, vx::array<Physics::DynamicCircle, vx::DelegateAllocator<AllocatorType>>* circles)
	{
		for (auto &it : circleComponents)
		{
			Physics::DynamicCircle c;
			c.cirlce.p_r = { it.m_position.x,it.m_position.y, it.m_radius.x, it.m_radius.y };
			c.prevPosition = vx::loadFloat2a(&it.m_prevPosition);
			circles->push_back(c);
		}
	}

	int testSphereAABB(const Circle128 &circle, const AABB128 &aabb)
	{
		auto p = circle.p;
		auto r = circle.r;

		auto v = p;
		v = vx::max(v, aabb.min);
		v = vx::min(v, aabb.max);

		v = _mm_sub_ps(v, p);
		auto dp = vx::dot2(v, v);

		auto cmp = _mm_cmple_ps(dp, r);
		auto mask = _mm_movemask_ps(cmp);
		return (mask & 0x1);
	}
}

PhysicsAspect::PhysicsAspect()
	:m_dynamicCircleComponents(),
	m_gravityAreas(),
	m_gravityWells(),
	m_staticCircles(),
	m_staticCircleComponents(),
	m_staticRects(),
	m_staticRectComponents(),
	m_frameAllocator()
{

}

PhysicsAspect::~PhysicsAspect()
{

}

bool PhysicsAspect::initialize(const PhysicsAspectBaseDesc &desc)
{
	m_onCollisionCallback = desc.onCollisionCallback;
	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> dynamicCircleComponents(desc.allocator, (size_t)desc.capacityDynamic);
	m_dynamicCircleComponents.swap(dynamicCircleComponents);

	m_tmpDynamicCircleComponents= vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>(desc.allocator, (size_t)desc.capacityDynamic);

	vx::array<GravityArea, DebugAllocatorPhysics<Allocator::MainAllocator>> gravityAreas(desc.allocator, desc.capacityGravityAreas);
	m_gravityAreas.swap(gravityAreas);

	vx::array<GravityWell, DebugAllocatorEntity<Allocator::MainAllocator>> gravityWells(desc.allocator, desc.capacityGravityWell);
	m_gravityWells.swap(gravityWells);

	vx::array<Circle128C, DebugAllocatorPhysics<Allocator::MainAllocator>> staticCircles(desc.allocator, desc.capacityStatic);
	m_staticCircles.swap(staticCircles);

	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> staticCircleComponents(desc.allocator, (size_t)desc.capacityStatic);
	m_staticCircleComponents.swap(staticCircleComponents);

	vx::array<AABB128, DebugAllocatorPhysics<Allocator::MainAllocator>> staticRects(desc.allocator, (size_t)desc.capacityStatic);
	m_staticRects.swap(staticRects);

	vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>> staticRectComponents(desc.allocator, (size_t)desc.capacityStatic);
	m_staticRectComponents.swap(staticRectComponents);

	const auto dynamicMemory = 1 MBYTE;
	m_frameAllocator.initialize(desc.allocator->allocate(dynamicMemory, 16, Allocator::Channels::Physics, "frame data"));

	return true;
}

void PhysicsAspect::shutdow(Allocator::MainAllocator* allocator)
{
	allocator->deallocate(m_frameAllocator.release(), Allocator::Channels::Physics, "frame data");
	m_staticRectComponents.release();
	m_staticRects.release();
	m_staticCircleComponents.release();
	m_staticCircles.release();
	m_gravityWells.release();
	m_gravityAreas.release();
	m_tmpDynamicCircleComponents.release();
	m_dynamicCircleComponents.release();
}

void PhysicsAspect::accumulateForces(f32 dt)
{
	for (auto &it : m_gravityAreas)
	{
		auto appliedForce = it.force;
		for (auto &other : m_dynamicCircleComponents)
		{
			Circle c;
			c.p = other.m_position;
			c.r = other.m_radius.x;

			if (Physics::intersect(it.bounds, c))
			{
				other.m_force += appliedForce;
			}
		}
	}

	for (auto &it : m_gravityWells)
	{
		__m128 position = vx::loadFloat2a(&it.position);
		__m128 radius = _mm_load1_ps(&it.radius);

		for (auto &other : m_dynamicCircleComponents)
		{
			__m128 otherPosition = vx::loadFloat2a(&other.m_position);
			__m128 otherRadius = _mm_load1_ps(&other.m_radius.x);

			if (Physics::intersectCircleCircle(position, radius, otherPosition, otherRadius))
			{
				auto otherForce = vx::loadFloat2a(&other.m_force);
				auto otherRadius = _mm_load1_ps(&other.m_radius.x);

				f32 mass = 1.0f / other.m_invMass;
				auto otherMass = _mm_load1_ps(&mass);

				auto force = it.getForce(otherPosition, otherRadius);

				otherForce = _mm_add_ps(otherForce, force);
				vx::storeFloat2a(&other.m_force, otherForce);
			}
		}
	}
}

void PhysicsAspect::verlet(f32 dt)
{
	const __m128 drag = { 0.99f,0.99f,0.99f,0.99f };
	const __m128 drag1 = { 1.99f,1.99f,1.99f,1.99f };

	__m128 vDt = _mm_load1_ps(&dt);
	auto dt2 = _mm_mul_ps(vDt, vDt);

	for (auto &it : m_dynamicCircleComponents)
	{
		__m128 force = vx::loadFloat2a(&it.m_force);
		__m128 position = vx::loadFloat2a(&it.m_position);
		__m128 prevPosition = vx::loadFloat2a(&it.m_prevPosition);
		auto invMass = _mm_load1_ps(&it.m_invMass);
		it.m_prevPosition = it.m_position;

		force = _mm_mul_ps(force, invMass);
		auto offset = _mm_mul_ps(force, dt2);

		auto tmpPos = _mm_mul_ps(drag1, position);
		auto tmpOldPos = _mm_mul_ps(drag, prevPosition);
		tmpPos = _mm_sub_ps(tmpPos, tmpOldPos);
		position = _mm_add_ps(tmpPos, offset);

		vx::storeFloat2a(&it.m_position, position);
		it.m_force.x = 0;
		it.m_force.y = 0;
	}
}

void PhysicsAspect::generateCollisions(const vx::array<Physics::DynamicCircle, vx::DelegateAllocator<vx::LinearAllocator>> &dynamicCircles,
	vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>>* manifolds)
{
	Physics::CollisionGeneratorDesc desc;
	desc.dynamicCircleComponents = &m_dynamicCircleComponents;
	desc.dynamicCircles = &dynamicCircles;
	desc.manifolds = manifolds;
	desc.staticRectComponents = &m_staticRectComponents;
	desc.staticRects = &m_staticRects;

	Physics::CollisionGenerator::generate_dynamicCircle_staticRect(desc);
	Physics::CollisionGenerator::generate_dynamicCircle_dynamicCircle(desc);
}

void PhysicsAspect::resolveCollisions(const __m128 &dt, const __m128 &invDt, const vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>> &manifolds,
	vx::array<OnCollisionCallbackData, vx::DelegateAllocator<vx::LinearAllocator>>* callbackData)
{
	const auto OnCollisionCallbackDataCmp = [](const OnCollisionCallbackData &l, const OnCollisionCallbackData &r)
	{
		if (l.c0 < r.c0 || (l.c0 == r.c0 && l.c1 < r.c1))
			return true;

		return false;
	};

	for (auto &it : manifolds)
	{
		auto flags0 = it.c0->m_flags;
		auto flags1 = it.c0->m_flags;

		auto collisionNormal = vx::loadFloat2a(&it.d);

		if (flags0 == 0 && flags1 == 0)
		{
			//auto intersectionPoint = vx::loadFloat2a(&it.p);

			auto invMass0 = _mm_load1_ps(&it.c0->m_invMass);
			auto p0 = vx::loadFloat2a(&it.c0->m_position);
			auto u0 = _mm_sub_ps(p0, vx::loadFloat2a(&it.c0->m_prevPosition));
			u0 = _mm_div_ps(u0, dt);

			auto invMass1 = _mm_load1_ps(&it.c1->m_invMass);
			auto p1 = vx::loadFloat2a(&it.c1->m_position);
			auto u1 = _mm_sub_ps(p1, vx::loadFloat2a(&it.c1->m_prevPosition));
			u1 = _mm_div_ps(u1, dt);

			__m128 c0 = _mm_load1_ps(&it.c0->m_restitution);
			__m128 c1 = _mm_load1_ps(&it.c1->m_restitution);

			__m128 c = _mm_min_ps(c0, c1);

			auto velocity = _mm_sub_ps(u1, u0);
			auto velocityAlongNormal = vx::dot2(velocity, collisionNormal);
			//if (forceAlongNormal > 0)
			//	return;

			//auto impactForce = vx::abs(force);

			auto invTotalMass = _mm_add_ps(invMass1, invMass0);


			auto j = _mm_add_ps(vx::g_VXOne, c);
			j = vx::negate(j);
			j = _mm_mul_ps(j, velocityAlongNormal);
			j = _mm_div_ps(j, invTotalMass); // m s-1 kg
			//f32 j = (-(1.0f + c) * velocityAlongNormal) / invTotalMass; 
			auto impulse = _mm_mul_ps(j, collisionNormal);

			const __m128 percent = { 0.2f,0.2f,0.2f,0.2f }; // usually 20% to 80%
			const __m128 slop = { 0.01f,0.01f,0.01f,0.01f }; // usually 0.01 to 0.1
			auto depth = _mm_load1_ps(&it.depth);
			//auto correction = vx::max(_mm_sub_ps(depth, slop), vx::g_VXZero); / invTotalMass * percent * it.d;
			auto correction = vx::max(_mm_sub_ps(depth, slop), vx::g_VXZero);
			auto tmp = _mm_mul_ps(percent, collisionNormal);
			correction = _mm_div_ps(correction, invTotalMass);
			correction = _mm_mul_ps(correction, tmp);

			auto offset = vx::fma(impulse, dt, correction); // m kg

			auto offset0 = _mm_mul_ps(offset, invMass0);
			auto offset1 = _mm_mul_ps(offset, invMass1);

			p0 = _mm_sub_ps(p0, offset0);
			p1 = _mm_add_ps(p1, offset1);

			vx::storeFloat2a(&it.c0->m_position, p0);
			vx::storeFloat2a(&it.c1->m_position, p1);
		}

		if (it.c0->m_type == PhysicsType::Dynamic ||
			it.c1->m_type == PhysicsType::Dynamic)
		{
			OnCollisionCallbackData data;
			data.c0 = it.c0;
			data.c1 = it.c1;
			data.dir = collisionNormal;

			auto begin = callbackData->begin();
			auto end = callbackData->end();
			auto iter = std::lower_bound(begin, end, data, OnCollisionCallbackDataCmp);
			if (iter == end || OnCollisionCallbackDataCmp(data, *iter))
			{
				callbackData->push_back(data);

				end = callbackData->end();
				std::rotate(iter, end - 1, end);
			}
		}
	}
}

void PhysicsAspect::update(f32 deltaTime)
{
	const u32 stepCount = 4;
	auto dt = deltaTime / stepCount;

	if (m_tmpDynamicCircleComponents.size() != 0)
	{
		auto oldSize = m_dynamicCircleComponents.size();
		m_dynamicCircleComponents.swap(m_tmpDynamicCircleComponents);
		auto newSize = m_dynamicCircleComponents.size();

		memset(m_tmpDynamicCircleComponents.data(), 0, sizeof(PhysicsComponent)* m_tmpDynamicCircleComponents.size());
		m_tmpDynamicCircleComponents.clear();
	}

	vx::array<Manifold, vx::DelegateAllocator<vx::LinearAllocator>> manifolds(&m_frameAllocator, 64);
	vx::array<Physics::DynamicCircle, vx::DelegateAllocator<vx::LinearAllocator>> dynamicCircles(&m_frameAllocator, m_dynamicCircleComponents.size());
	vx::array<OnCollisionCallbackData, vx::DelegateAllocator<vx::LinearAllocator>> callbackData(&m_frameAllocator, m_dynamicCircleComponents.size());

	auto invDt = 1.0f / dt;

	__m128 vDt = _mm_load1_ps(&dt);
	__m128 vInvDt = _mm_load1_ps(&invDt);

	for (u32 i = 0; i < stepCount; ++i)
	{
		accumulateForces(dt);
		verlet(dt);

		PhysicsAspectCpp::buildCircles(m_dynamicCircleComponents, &dynamicCircles);

		generateCollisions(dynamicCircles, &manifolds);

		resolveCollisions(vDt, vInvDt, manifolds, &callbackData);

		manifolds.clear();
		dynamicCircles.clear();
	}

	auto callBack = m_onCollisionCallback;
	for (auto &it : callbackData)
	{
		callBack(it.dir, it.c0, it.c1);
	}
}

PhysicsComponent* PhysicsAspect::updateComponent(const PhysicsComponent &old)
{
	PhysicsComponent* p = nullptr;

	if (old.m_shapeType == EntityShapeType::Circle)
	{
		m_tmpDynamicCircleComponents.push_back(old);
		p = &m_tmpDynamicCircleComponents.back();
	}

	return p;
}

PhysicsComponent* PhysicsAspect::addStaticShape(const vx::float2a &p, f32 radius, f32 restitution, void* userData)
{
	f32 invMass = 0;

	m_staticCircleComponents.push_back(PhysicsComponent(radius, EntityShapeType::Circle, PhysicsType::Static, invMass, p, restitution, userData));

	Circle128C c;
	c.p_r = { p.x, p.y, radius, 0 };
	m_staticCircles.push_back(c);

	return &m_staticCircleComponents.back();
}

PhysicsComponent* PhysicsAspect::addStaticShape(const vx::float2a &p, const vx::float2a &halfDim, f32 restitution, void* userData)
{
	f32 invMass = 0;
	PhysicsType::Static,
	m_staticRectComponents.push_back(PhysicsComponent(halfDim, EntityShapeType::Rectangle, PhysicsType::Static, invMass, p, restitution, userData));

	AABB128 bounds;
	bounds.min = { p.x - halfDim.x, p.y - halfDim.y, 0, 0 };
	bounds.max = { p.x + halfDim.x, p.y + halfDim.y, 0, 0 };
	m_staticRects.push_back(bounds);

	return &m_staticRectComponents.back();
}

PhysicsComponent* PhysicsAspect::addDynamicShape(const vx::float2a &p, f32 radius, f32 mass, f32 restitution, void* userData)
{
	f32 invMass = 0.0f;
	if (mass != 0.0f)
	{
		invMass = 1.0f / mass;
	}

	m_dynamicCircleComponents.push_back(PhysicsComponent(radius, EntityShapeType::Circle, PhysicsType::Dynamic, invMass, p, restitution, userData));

	return &m_dynamicCircleComponents.back();
}

void PhysicsAspect::reset()
{
	m_dynamicCircleComponents.clear();
	m_staticRectComponents.clear();
	m_staticCircleComponents.clear();
	m_staticCircles.clear();
	m_staticRects.clear();
	m_tmpDynamicCircleComponents.clear();
}

void PhysicsAspect::loadLevel(const LevelFile &level)
{
	size_t gravityAreaCount = 0;
	auto gravityAreas = level.getGravityAreas(&gravityAreaCount);
	for (size_t i = 0; i < gravityAreaCount; ++i)
	{
		auto &it = gravityAreas[i];

		GravityArea area;
		area.bounds.min = it.position - it.halfDim;
		area.bounds.max = it.position + it.halfDim;
		area.force = it.force;

		m_gravityAreas.push_back(area);
	}

	size_t gravityWellCount = 0;
	auto gravityWells = level.getGravityWells(&gravityWellCount);
	for (size_t i = 0; i < gravityWellCount; ++i)
	{
		m_gravityWells.push_back(gravityWells[i]);
	}
}

void PhysicsAspect::handleEvent(const Event::Event &evt)
{
	auto type = evt.getType();

	if (type == Event::Type::EventTypeFile)
	{
		auto fileEvent = evt.asFileEvent();
		if (fileEvent == Event::Type::FileEventTypes::LoadedLevel)
		{
			auto &resourceAspect = Locator::getResourceAspect();

			auto levelSid = vx::StringID(evt.getData().u64);
			auto level = resourceAspect.findLevel(levelSid);

			loadLevel(*level);
		}
	}
	else if (type == Event::Type::EventTypeIngame)
	{
		auto type = evt.asIngameEvent();
		if (type == Event::Type::IngameEventTypes::RestartLevelBegin)
		{
			Event::RestartLevelData &p = *((Event::RestartLevelData*)&evt.getData().ptr);
			if (p.restartPhysics == 0)
			{
				reset();
				p.restartPhysics = 1;
			}

			if (p.finished())
			{
				auto &evtManager = Locator::getEventManager();
				evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RestartLevel, vx::Variant()));
			}
		}
		else if(type == Event::Type::IngameEventTypes::RestartLevelFinished)
		{
			vx::StringID sid(evt.getData().u64);
			auto level = Locator::getResourceAspect().findLevel(sid);

			loadLevel(*level);
		}
	}
}

GravityArea* PhysicsAspect::getGravityArea(size_t i)
{
	return &m_gravityAreas[i];
}