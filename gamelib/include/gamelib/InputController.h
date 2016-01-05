#pragma once

struct DynamicEntity;

#include <vxLib/math/Vector.h>

class InputController
{
public:
	virtual ~InputController() {}

	virtual void VX_CALLCONV update(__m128 dt, DynamicEntity* entity) = 0;

	virtual void setValue(void* p) {};
};

typedef InputController* (*CreateInputControllerFn)(void*);
typedef void (*DestroyInputControllerFn)(InputController*);