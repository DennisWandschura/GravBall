#pragma once

struct PhysicsComponent;

#include <gamelib/AABB.h>
#include <gamelib/EntityShape.h>
#include <vxLib/StringID.h>
#include "Timer.h"
#include "unique_ptr.h"

struct DynamicGravityFunctor
{
	virtual ~DynamicGravityFunctor() {}

	virtual void operator()(vx::float2a* currentForce) = 0;
};

struct GravityArea
{
	AABB bounds;
	vx::float2a force;
};

struct DynamicGravityArea
{
	vx::float2a currentForce;
	Timer timer;
	vx::unique_ptr<DynamicGravityFunctor> changeForceFunctor;
	GravityArea* physicsComponent;

	DynamicGravityArea() :currentForce(), timer(), changeForceFunctor(), physicsComponent() {}

	DynamicGravityArea(const DynamicGravityArea&) = delete;

	DynamicGravityArea(DynamicGravityArea &&rhs)
		:currentForce(rhs.currentForce),
		timer(rhs.timer),
		changeForceFunctor(std::move(rhs.changeForceFunctor)),
		physicsComponent(rhs.physicsComponent)
	{}

	~DynamicGravityArea() {}

	DynamicGravityArea& operator=(const DynamicGravityArea&) = delete;
	DynamicGravityArea& operator=(DynamicGravityArea &&rhs) = delete;
};

enum class GravityAreaType : u32
{
	Static, Dynamic
};

struct GravityAreaFile
{
	GravityAreaType type;
	f32 time;
	vx::float2 position;
	vx::float2 halfDim;
	vx::float2 force;
	vx::StringID texture;

	static GravityAreaFile createStatic(const vx::float2 &position, const vx::float2 &halfDim,const vx::float2 &force, const vx::StringID &texture)
	{
		return { GravityAreaType::Static, 0.0f, position, halfDim, force, texture };
	}

	static GravityAreaFile createDynamic(f32 time, const vx::float2 &position, const vx::float2 &halfDim, const vx::float2 &force, const vx::StringID &texture)
	{
		return{ GravityAreaType::Dynamic, time, position, halfDim, force, texture };
	}

	f32 getRadius() const { return 0; }
	vx::float2a getHalfDim() const { return halfDim; }
	EntityShapeType getShapeType() const { return EntityShapeType::Rectangle; }
	vx::StringID getTexture() const { return texture; }
	const vx::float2& getPosition() const { return position; }
};

struct EditorGravityAreaFile : public GravityAreaFile
{
	PhysicsComponent* physicsComponent;
	u32 transformOffset;
	u32 vertexOffset;
	u32 indexOffset;
	u32 physicsOffset;
	u8 createdRenderer;
	u8 createdPhysics;

	EditorGravityAreaFile()
		:GravityAreaFile(),
		createdRenderer(0),
		createdPhysics(0)
	{

	}
};