#pragma once

class EventManager;
class ResourceAspect;

#include <vxLib/types.h>

class Locator
{
	static EventManager* m_eventManager;
	static ResourceAspect* m_resourceAspect;

public:
	Locator() {}
	~Locator() {}

	static void reset();

	static void set(EventManager* p);
	static EventManager& getEventManager();

	static void set(ResourceAspect* p);
	static ResourceAspect& getResourceAspect();
};