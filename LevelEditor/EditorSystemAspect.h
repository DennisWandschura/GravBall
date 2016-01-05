#pragma once

#include <gamelib/SystemAspectBase.h>

class EditorSystemAspect : public SystemAspectBase
{
	void* m_hwnd;

public:
	EditorSystemAspect():m_hwnd(nullptr) {}
	~EditorSystemAspect() {}

	bool initialize(const vx::uint2 &resolution, bool fullscreen, KeyEventCallbackSignature onKeyPressed, KeyEventCallbackSignature onKeyReleased, Allocator::MainAllocator* mainAllocator) override
	{
		return true;
	}

	virtual void shutdown(Allocator::MainAllocator* mainAllocator) override {}

	void update() override {}

	void* getHwnd() const override  {return m_hwnd;}

	void setHwnd(void* h) { m_hwnd = h; }
};