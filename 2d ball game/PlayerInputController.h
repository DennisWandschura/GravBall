#pragma once

#include <gamelib/InputController.h>

class PlayerInputController : public InputController
{
public:
	void VX_CALLCONV update(__m128 dt, DynamicEntity* entity) override;
};