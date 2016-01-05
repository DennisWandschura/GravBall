#pragma once

#include <gamelib/SystemAspectBase.h>
#include <vxLib/Window.h>
#include <vxLib/Allocator/LinearAllocator.h>

class SystemAspect : public SystemAspectBase
{
	vx::Window m_window;
	vx::LinearAllocator m_keyEvtAllocator;

public:
	SystemAspect();
	~SystemAspect();

	bool initialize(const vx::uint2 &resolution, bool fullscreen, KeyEventCallbackSignature onKeyPressed, KeyEventCallbackSignature onKeyReleased, Allocator::MainAllocator* mainAllocator) override;
	void shutdown(Allocator::MainAllocator* mainAllocator) override;

	void update() override;

	void* getHwnd() const override;
};