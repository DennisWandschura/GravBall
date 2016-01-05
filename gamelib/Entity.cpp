#include "include\gamelib\Entity.h"
#include "include\gamelib\PhysicsComponent.h"
#include "include/gamelib/EntityShape.h"

vx::float2a DynamicEntity::getPosition() const
{
	return physicsComponent->m_position;
}

f32 DynamicEntity::getRadius() const
{
	return physicsComponent->m_radius.x;
}

vx::float2a DynamicEntity::getHalfDim() const
{
	return physicsComponent->m_halfDim;
}

EntityShapeType DynamicEntity::getShapeType() const
{
	return physicsComponent->m_shapeType;
}