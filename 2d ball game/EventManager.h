#pragma once

namespace Event
{
	class ListenerInterface;
	class Event;
}

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/Container/array.h>
#include <vxLib/Container/DoubleBuffer.h>
#include <gamelib/DebugAllocator.h>
#include <gamelib/Event.h>

class EventManager
{
	struct ListenerEntry
	{
		Event::ListenerInterface* listener;
		u32 mask;
	};

	vx::DoubleBuffer<Event::Event, DebugAllocatorEvent<Allocator::MainAllocator>> m_events;
	vx::array<ListenerEntry, DebugAllocatorEvent<Allocator::MainAllocator>> m_listeners;

public:
	EventManager();
	~EventManager();

	bool initialize(Allocator::MainAllocator* allocator, size_t evtCapacity, size_t listenerCapacity);
	void shutdown();

	void swap(EventManager &other);

	void update();

	void pushEvent(const Event::Event &evt);

	void registerListener(Event::ListenerInterface* listener, u32 mask);
};