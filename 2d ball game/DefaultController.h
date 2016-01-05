#pragma once

#include <gamelib/InputController.h>

class DefaultController : public InputController
{
	DynamicEntity* m_player;

public:
	explicit DefaultController(DynamicEntity* player);
	~DefaultController();

	void VX_CALLCONV update(__m128 dt, DynamicEntity* entity) override;

	void setValue(void* p) { m_player = (DynamicEntity*)p; }
};