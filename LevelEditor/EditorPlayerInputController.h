#pragma once

#include <gamelib/InputController.h>

class EditorPlayerInputController : public InputController
{
	vx::float2a m_accum;

public:
	EditorPlayerInputController():m_accum(){}
	~EditorPlayerInputController() {}

	void VX_CALLCONV update(__m128 dt, DynamicEntity* entity) override;

	void move(f32 x, f32 y);
};