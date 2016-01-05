#include "SystemAspect.h"
#include <vxLib/RawInput.h>
#include <gamelib/MainAllocator.h>

SystemAspect::SystemAspect()
	:m_window(),
	m_keyEvtAllocator()
{

}

SystemAspect::~SystemAspect()
{

}

bool SystemAspect::initialize(const vx::uint2 &resolution, bool fullscreen, KeyEventCallbackSignature onKeyPressed, KeyEventCallbackSignature onKeyReleased, Allocator::MainAllocator* mainAllocator)
{
	const size_t keyEvtCount = 32;
	auto block = mainAllocator->allocate(sizeof(vx::KeyEvent) * keyEvtCount, __alignof(vx::KeyEvent), Allocator::Channels::General, "key events");
	m_keyEvtAllocator = vx::LinearAllocator(block);

	if (!m_window.initialize(L"Game", resolution, fullscreen, vx::DelegateAllocator<vx::LinearAllocator>(&m_keyEvtAllocator), keyEvtCount))
		return false;

	vx::RawInput::setCallbackKeyPressed(onKeyPressed);
	vx::RawInput::setCallbackKeyReleased(onKeyReleased);

	return true;
}

void SystemAspect::shutdown(Allocator::MainAllocator* mainAllocator)
{
	m_window.shutdown();
	m_keyEvtAllocator.deallocateAll();

	auto block = m_keyEvtAllocator.release();
	mainAllocator->deallocate(block, Allocator::Channels::General, "key events");
}

void SystemAspect::update()
{
	m_window.update();
}

void* SystemAspect::getHwnd() const
{
	return m_window.getHwnd();
}