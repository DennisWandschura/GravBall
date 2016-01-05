#pragma once

struct PhysicsComponent;

#include <vxLib/StringID.h>
#include "EntityShape.h"

struct StaticEntityFile
{
	vx::StringID texture;
	vx::float2 position;
	EntityShape shape;
	f32 restitution;

	vx::float2 getPosition() const { return position; }
	f32 getRadius() const { return shape.radius.x; }
	vx::float2a getHalfDim() const { return shape.halfDim; }
	EntityShapeType getShapeType() const { return shape.type; }
	vx::StringID getTexture() const { return texture; }
};

struct DynamicEntityFile
{
	vx::StringID controller;
	vx::StringID texture;
	vx::float2 position;
	EntityShape shape;
	f32 restitution;
	f32 mass;

	vx::float2 getPosition() const { return position; }
	f32 getRadius() const { return shape.radius.x; }
	vx::float2a getHalfDim() const { return shape.halfDim; }
	EntityShapeType getShapeType() const { return shape.type; }
	vx::StringID getTexture() const { return texture; }
};

struct EditorStaticEntityFile : public StaticEntityFile
{
	PhysicsComponent* physicsComponent;
	u32 transformOffset;
	u32 physicsOffset;
	u32 vertexOffset;
	u32 indexOffset;
	u8 createdPhysics;
	u8 createdRenderer;

	EditorStaticEntityFile()
		:StaticEntityFile(),
		physicsComponent(nullptr),
		transformOffset(0),
		physicsOffset(0),
		vertexOffset(0),
		indexOffset(0),
		createdPhysics(0),
		createdRenderer(0)
	{
	}
};

struct EditorDynamicEntityFile : public DynamicEntityFile
{
	PhysicsComponent* physicsComponent;
	u32 transformOffset;
	u32 physicsIndex;
	u32 vertexOffset;
	u32 indexOffset;
	u8 createdPhysics;
	u8 createdRenderer;

	EditorDynamicEntityFile()
		:DynamicEntityFile(),
		physicsComponent(nullptr),
		transformOffset(0),
		physicsIndex(0),
		vertexOffset(0),
		indexOffset(0),
		createdPhysics(0),
		createdRenderer(0)
	{
	}
};

struct GoalEntityFile
{
	vx::float2a position;
	vx::float2a halfDim;
	vx::StringID textureSid;

	StaticEntityFile asStaticEntity()
	{
		EntityShape shape;
		shape.halfDim = { halfDim.x,halfDim.y };
		shape.type = EntityShapeType::Rectangle;

		return 
		{
			textureSid,
			{ position.x,position.y},
			shape,
			0
		};
	}
};

namespace Editor
{
	struct GoalEntityFile : public ::GoalEntityFile
	{
		PhysicsComponent* physicsComponent;
		u32 vertexOffset;
		u32 indexOffset;
		u32 transformOffset;
		u32 physicsOffset;
		u8 createdPhysics;
		u8 createdRenderer;

		GoalEntityFile()
			: ::GoalEntityFile(),
			physicsComponent(nullptr),
			vertexOffset(0),
			indexOffset(0),
			transformOffset(0),
			physicsOffset(0),
			createdPhysics(0),
			createdRenderer(0)
		{

		}

		::EditorStaticEntityFile asStaticEntity()
		{
			::EditorStaticEntityFile entity;
			entity.physicsComponent = physicsComponent;
			entity.position = {position.x, position.y};
			entity.shape.halfDim = { halfDim.x,halfDim.y };
			entity.shape.type = EntityShapeType::Rectangle;
			entity.restitution = 0;
			entity.texture = textureSid;
			entity.vertexOffset = vertexOffset;
			entity.indexOffset = indexOffset;
			entity.transformOffset = transformOffset;

			return entity;
		}
	};

	struct SpawnEntityFile
	{
		PhysicsComponent* physicsComponent;
		vx::float2a position;
		vx::StringID textureSid;
		u32 vertexOffset;
		u32 indexOffset;
		u32 transformOffset;
		u32 physicsOffset;
		u8 createdPhysics;
		u8 createdRenderer;

		SpawnEntityFile()
			: physicsComponent(nullptr),
			position(),
			textureSid(),
			vertexOffset(0),
			indexOffset(0),
			transformOffset(0),
			physicsOffset(0),
			createdPhysics(0),
			createdRenderer(0)
		{

		}
	};
}