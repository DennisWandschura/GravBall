#include "DefaultController.h"
#include <gamelib/Entity.h>
#include <gamelib/PhysicsComponent.h>

DefaultController::DefaultController(DynamicEntity* player)
	:m_player(player)
{

}

DefaultController::~DefaultController()
{

}

void DefaultController::update(__m128 dt, DynamicEntity* entity)
{
	auto playerPosition = vx::loadFloat2a(&m_player->physicsComponent->m_position);
	auto thisPosition = vx::loadFloat2a(&entity->physicsComponent->m_position);

	auto dir = _mm_sub_ps(playerPosition, thisPosition);
	auto distance2ToPlayer = vx::dot2(dir, dir);

	if (distance2ToPlayer.m128_f32[0] <= 10.f)
	{
		auto distance = _mm_sqrt_ps(distance2ToPlayer);
		dir = _mm_div_ps(dir, distance);

		const __m128 velocity_multiplier = { 0.1f, 0.1f, 0.1f, 0.1f };
		auto velocity = _mm_mul_ps(dir, velocity_multiplier);

		entity->physicsComponent->move(velocity, dt);
	}
}