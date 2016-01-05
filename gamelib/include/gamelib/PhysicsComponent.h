#pragma once

#include <gamelib/AABB.h>
#include <gamelib/Circle.h>
#include <gamelib/EntityShape.h>

enum class PhysicsType : u8
{
	Static, Dynamic
};

enum PhysicsFlags : u8
{
	PhysicsFlag_None = 0,
	PhysicsFlag_NoCollisionResponse = 1
};

struct alignas(8) PhysicsComponent
{
	union
	{
		vx::float2a m_halfDim;
		vx::float2a m_radius;
	};
	EntityShapeType m_shapeType;
	PhysicsType m_type;
	u8 m_flags;
	f32 m_invMass;
	vx::float2a m_force;
	vx::float2a m_prevPosition;
	vx::float2a m_position;
	f32 m_restitution;
	u32 padding;
	void* m_userData;

	PhysicsComponent() :m_shapeType(), m_type(PhysicsType::Static), m_flags(PhysicsFlag_None),m_invMass(), m_force(),m_position(), m_restitution(), m_userData(nullptr){}

	PhysicsComponent(f32 radius, EntityShapeType shapeType, PhysicsType type, f32 invMass, const vx::float2a &p, f32 restitution, void* userData)
		:m_radius(radius), m_shapeType(shapeType), m_type(type), m_flags(PhysicsFlag_None),m_invMass(invMass), m_force(), m_prevPosition(p), m_position(p), m_restitution(restitution), m_userData(userData) {}

	PhysicsComponent(const vx::float2a &halfDim, EntityShapeType shapeType, PhysicsType type, f32 invMass, const vx::float2a &p, f32 restitution, void* userData)
		:m_halfDim(halfDim), m_shapeType(shapeType), m_type(type), m_flags(PhysicsFlag_None),m_invMass(invMass), m_force(), m_prevPosition(p), m_position(p), m_restitution(restitution), m_userData(userData){}

	void VX_CALLCONV move(vx::CVEC4 offset, vx::CVEC4 dt)
	{
		auto p = vx::loadFloat2a(&m_position);
		auto tmp = _mm_mul_ps(offset, dt);
		p = _mm_add_ps(p, tmp);
		vx::storeFloat2a(&m_position, p);
	}
};