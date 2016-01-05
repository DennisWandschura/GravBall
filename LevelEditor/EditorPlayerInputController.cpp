#include "EditorPlayerInputController.h"
#include <gamelib/Entity.h>
#include <gamelib/PhysicsComponent.h>

void EditorPlayerInputController::update(__m128 dt, DynamicEntity* entity)
{
	entity->physicsComponent->m_force = m_accum;
	m_accum = { 0, 0 };
}

void EditorPlayerInputController::move(f32 x, f32 y)
{
	m_accum.x = x;
	m_accum.y = y;
}