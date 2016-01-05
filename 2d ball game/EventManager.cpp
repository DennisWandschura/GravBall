#include "EventManager.h"
#include <gamelib/MainAllocator.h>
#include <gamelib/EventListener.h>
#include "algorithm.h"

EventManager::EventManager()
	:m_events(),
	m_listeners()
{

}

EventManager::~EventManager()
{

}

bool EventManager::initialize(Allocator::MainAllocator* allocator, size_t evtCapacity, size_t listenerCapacity)
{
	vx::DoubleBuffer<Event::Event, DebugAllocatorEvent<Allocator::MainAllocator>> events(allocator, evtCapacity);
	vx::array<ListenerEntry, DebugAllocatorEvent<Allocator::MainAllocator>> listeners(allocator, listenerCapacity);

	m_events.swap(events);
	m_listeners.swap(listeners);

	return true;
}

void EventManager::shutdown()
{
	m_listeners.release();
	m_events.release();
}

void EventManager::swap(EventManager &other)
{
	m_listeners.swap(other.m_listeners);
	m_events.swap(other.m_events);
}

void EventManager::update()
{
	m_events.swapBuffers();
	auto sizeBack = m_events.sizeBack();
	for (size_t i = 0; i < sizeBack; ++i)
	{
		Event::Event evt;
		m_events.pop_backBuffer(evt);

		for (auto &listener : m_listeners)
		{
			auto result = listener.mask & evt.getType();
			if (result != 0)
			{
				listener.listener->handleEvent(evt);
			}
		}

#if _EDITOR
		evt.callback();
#endif
	}
}

void EventManager::pushEvent(const Event::Event &evt)
{
	m_events.push(evt);
}

void EventManager::registerListener(Event::ListenerInterface* listener, u32 mask)
{
	m_listeners.push_back(listener, mask );
}