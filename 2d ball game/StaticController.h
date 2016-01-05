#pragma once

#include <gamelib/InputController.h>

class StaticController : public InputController
{
public:
	StaticController():InputController(){}
	~StaticController() {}

	void VX_CALLCONV update(__m128 dt, DynamicEntity* entity) override {}

	void setValue(void* p) {  }
};