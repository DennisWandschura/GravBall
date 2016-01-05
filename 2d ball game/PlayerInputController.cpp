#include "PlayerInputController.h"
#include <vxLib/RawInput.h>
#include <gamelib/Entity.h>
#include <gamelib/PhysicsComponent.h>

void PlayerInputController::update(__m128 dt, DynamicEntity* entity)
{
	vx::Keyboard keyboard;
	vx::RawInput::getKeyboard(keyboard);

	auto key_a = keyboard[vx::Keyboard::Key_A];
	auto key_d = keyboard[vx::Keyboard::Key_D];

	auto key_w = keyboard[vx::Keyboard::Key_W];
	auto key_s = keyboard[vx::Keyboard::Key_S];

	static const __m128 moveForce = { 0.1f, 0.1f, 0.1f, 0.1f };
	__m128 newDirection = { f32(key_d - key_a), f32(key_w - key_s), 0, 0 };
	newDirection = vx::normalize2(newDirection);

	auto force = _mm_mul_ps(newDirection, moveForce);

	entity->physicsComponent->move(force, dt);
}