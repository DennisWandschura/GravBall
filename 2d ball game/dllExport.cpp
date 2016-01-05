#include "include/game/dllExport.h"
#if _EDITOR
#include "Engine.h"

EngineBase* createEngine(const EngineBaseDesc &desc)
{
	if (desc.physicsAspect == nullptr || desc.systemAspect == nullptr)
		return nullptr;

	auto engine = new Engine();

	if (!engine->initialize(desc))
	{
		engine->shutdown(desc.mainAllocator);
		delete(engine);
		engine = nullptr;
	}

	return engine;
}

void destroyEngine(EngineBase* engine, Allocator::MainAllocator* mainAllocator)
{
	if (engine)
	{
		engine->shutdown(mainAllocator);
		delete(engine);
	}
}
#endif