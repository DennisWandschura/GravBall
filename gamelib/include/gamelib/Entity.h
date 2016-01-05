#pragma once

class InputController;
struct PhysicsComponent;

#include <vxLib/math/Vector.h>
#include <vxLib/StringID.h>

enum class EntityShapeType : u16;

struct RenderComponent
{
	vx::float2 position;
	u32 rendererIndex;
};

struct DynamicEntity
{
	InputController* inputController;
	PhysicsComponent* physicsComponent;
	vx::StringID textureSid;
	u32 rendererIndex;

	vx::float2a getPosition() const;
	f32 getRadius() const;
	vx::float2a getHalfDim() const;
	EntityShapeType getShapeType() const;
	vx::StringID getTexture() const { return textureSid; }
};

struct StaticEntity
{
	vx::float2a position;
};