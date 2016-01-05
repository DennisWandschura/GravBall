#pragma once

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/math/Vector.h>

typedef void(*KeyEventCallbackSignature)(u16 key);

class SystemAspectBase
{
protected:
	SystemAspectBase() {}

public:
	virtual ~SystemAspectBase() {}

	virtual bool initialize(const vx::uint2 &resolution, bool fullscreen, KeyEventCallbackSignature onKeyPressed, KeyEventCallbackSignature onKeyReleased, Allocator::MainAllocator* mainAllocator) = 0;
	virtual void shutdown(Allocator::MainAllocator* mainAllocator) = 0;

	virtual void update() = 0;

	virtual void* getHwnd() const = 0;
};