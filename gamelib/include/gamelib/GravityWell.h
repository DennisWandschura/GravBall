#pragma once

struct PhysicsComponent;

#include <vxLib/math/Vector.h>
#include <gamelib/EntityShape.h>
#include <vxLib/StringID.h>

struct GravityWell
{
	vx::float2a position;
	f32 radius;
	f32 maxForce;

	__m128 VX_CALLCONV getForce(__m128 otherPosition, __m128 otherRadius) const
	{
		static const __m128 epsilon = {0.1f, 0.1f, 0.1f, 0.1f};
		//const f32 gravity_const = 6.674e-5f;
		//const __m128 grav = { gravity_const, gravity_const, gravity_const, gravity_const };

		//const f32 div_const = 1.0e-5f;
		//const __m128 vdiv = { div_const, div_const, div_const, div_const };

	//	const __m128 thisMass = { mass, mass, mass, mass };
		const __m128 thisForce = { maxForce, maxForce, maxForce, maxForce };
		__m128 thisRadius = { radius, radius, radius, radius };
		thisRadius = _mm_add_ps(thisRadius, otherRadius);
		__m128 thisPosition = vx::loadFloat2a(&position);

		auto directionFromOtherToThis = _mm_sub_ps(thisPosition, otherPosition);
		auto distance = vx::dot2(directionFromOtherToThis, directionFromOtherToThis);
		distance = _mm_sqrt_ps(distance);
		distance = _mm_max_ps(distance, epsilon);
		directionFromOtherToThis = _mm_div_ps(directionFromOtherToThis, distance);

		distance = _mm_min_ps(distance, thisRadius);
		distance = _mm_sub_ps(thisRadius, distance);

		auto force = _mm_mul_ps(distance, thisForce);
		force = _mm_mul_ps(force, directionFromOtherToThis);

		//auto tmp = _mm_mul_ps(otherMass, thisMass);
		//tmp = _mm_mul_ps(tmp, grav);
		//tmp = _mm_div_ps(tmp, distance);
	//	tmp = _mm_mul_ps(tmp, vdiv);

		return force;
	}
};

struct GravityWellFile : public GravityWell
{
	vx::StringID texture;

	static GravityWellFile create(const vx::float2a &position, f32 radius, f32 maxForce, const vx::StringID &texture)
	{
		GravityWellFile well;
		well.position = position;
		well.radius = radius;
		well.maxForce = maxForce;
		well.texture = texture;

		return well;
	}

	f32 getRadius() const { return radius; }
	vx::float2a getHalfDim() const { return vx::float2a(0, 0); }
	EntityShapeType getShapeType() const { return EntityShapeType::Circle; }
	const vx::float2a& getPosition() const { return position; }
	vx::StringID getTexture() const { return texture; }
};

struct EditorGravityWellFile : public GravityWellFile
{
	PhysicsComponent* physicsComponent;
	u32 transformOffset;
	u32 vertexOffset;
	u32 indexOffset;
	u32 physicsOffset;
	u8 createdPhysics;
	u8 createdRenderer;

	EditorGravityWellFile()
		:GravityWellFile(),
		createdPhysics(0),
		createdRenderer(0)
	{

	}
};