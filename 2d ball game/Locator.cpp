#include "Locator.h"

EventManager* Locator::m_eventManager{nullptr};
ResourceAspect* Locator::m_resourceAspect{nullptr};

void Locator::reset()
{
	m_eventManager = nullptr;
	m_resourceAspect = nullptr;
}

void Locator::set(EventManager* p)
{
	VX_ASSERT(p);
	m_eventManager = p;
}

EventManager& Locator::getEventManager()
{
	VX_ASSERT(m_eventManager);
	return *m_eventManager;
}

void Locator::set(ResourceAspect* p)
{
	VX_ASSERT(p);
	m_resourceAspect = p;
}

ResourceAspect& Locator::getResourceAspect()
{
	VX_ASSERT(m_resourceAspect);
	return *m_resourceAspect;
}