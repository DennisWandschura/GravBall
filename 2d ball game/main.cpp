#ifndef _EDITOR
#include <gamelib/MainAllocator.h>
#include "Engine.h"
#include <csignal>
#include <cstdlib>
#include <vxLib/Allocator/Mallocator.h>
#include "CreateLevel.h"
#include <vxLib/ScopeGuard.h>
#include <gamelib/AllocatorDebugLayer.h>
#include <gamelib/Allocator.h>
#include <vxLib/print.h>
#include "PhysicsAspect.h"
#include <gamelib/Logfile.h>
#include "SystemAspect.h"
#include <gamelib/InputController.h>
#include "PlayerInputController.h"
#include <gamelib/ControllerLibrary.h>
#include "DefaultController.h"
#include <gamelib/LevelFile.h>
#include "StaticController.h"

Logfile* g_logfile{nullptr};
Engine* g_engine{ nullptr };
Allocator::MainAllocator* g_mainAlloc{ nullptr };
Allocator::DebugLayer* g_allocationDebugLayer{nullptr};
ControllerLibrary* g_controllerLibrary{nullptr};

void onKeyPressed(u16 keyCode)
{
	g_engine->onKeyPressed((vx::Input::Keys)keyCode);
}

void onKeyReleased(u16 keyCode)
{
	g_engine->onKeyReleased((vx::Input::Keys)keyCode);
}

void VX_CALLCONV OnCollisionCallback(__m128 dir, PhysicsComponent* c0, PhysicsComponent* c1)
{
	g_engine->onCollisionCallback(dir, c0, c1);
}

InputController* createPlayerInputController(void*)
{
	return new PlayerInputController();
}

InputController* createDefaultInputController(void* p)
{
	return new DefaultController((DynamicEntity*)p);
}

InputController* createStaticInputController(void* p)
{
	return new StaticController();
}

void destroyInputController(InputController* p)
{
	delete(p);
}

void shutdown()
{
	if (g_engine)
	{
		g_engine->shutdown(g_mainAlloc);
		g_engine = nullptr;
	}

	if (g_controllerLibrary)
	{
		g_controllerLibrary->release();
		g_controllerLibrary = nullptr;
	}

	AllocationCountLayer<vx::Mallocator> malloc;

	if (g_mainAlloc)
	{
		g_mainAlloc->shutdown(&malloc);
		g_mainAlloc = nullptr;
	}

	if (g_allocationDebugLayer)
	{
		g_allocationDebugLayer->shutdown(&malloc);
		g_allocationDebugLayer = nullptr;
	}

	if (g_logfile)
	{
		malloc.deallocate(g_logfile->close());
		g_logfile = nullptr;
	}

	malloc.deallocate(vx::deallocate_console());
}

void __cdecl signal_handler(int signal)
{
	if (signal == SIGABRT)
	{
		shutdown();
	}
	else
	{

	}

	std::exit(1);
}

bool loadLevel(const char* file, Engine* engine, Allocator::MainAllocator* mainAlloc)
{
	vx::File inFile;
	if (!inFile.open(file, vx::FileAccess::Read))
		return false;

	auto fileSize = inFile.getSize();
	auto block = mainAlloc->allocate(fileSize, 4, Allocator::Channels::General, "load level data");
	if (block.ptr == nullptr)
		return false;

	inFile.read(block.ptr, fileSize);

	LevelFile level;
	auto loaded = level.loadFromMemory(block, mainAlloc);
	mainAlloc->deallocate(block, Allocator::Channels::General, "load level data");

	if (!loaded)
	{
		level.release();
		return false;
	}

	auto levelSid = vx::make_sid(file);
	engine->addLevel(levelSid, std::move(level));

	vx::Variant evtData;
	evtData.u64 = levelSid.value;
	Event::Event evt(Event::Type::FileEventTypes::LoadedLevel, evtData);
	engine->pushEvent(evt);

	return true;
}

int __cdecl main(int argc, char *argv[])
{
	if (argc <= 0)
		return 1;

	vx::Mallocator malloc;

	vx::allocate_console(malloc.allocate(64 KBYTE, 64));

	Logfile logfile;
	logfile.create("logfile.txt", malloc.allocate(2048, 64));
	g_logfile = &logfile;

	Allocator::DebugLayer allocatorDebugLayer;
	g_allocationDebugLayer = &allocatorDebugLayer;
	allocatorDebugLayer.initialize(&malloc);

	Allocator::MainAllocator mainAlloc;
	g_mainAlloc = &mainAlloc;

	mainAlloc.initialize(&malloc, &allocatorDebugLayer);

	auto previous_handler = std::signal(SIGABRT, signal_handler);
	if (previous_handler == SIG_ERR)
	{
		shutdown();
		return 1;
	}

	ControllerLibrary controllerLibrary;
	g_controllerLibrary = &controllerLibrary;
	controllerLibrary.initialize(8, &mainAlloc);
	controllerLibrary.registerFunction(vx::make_sid("player"), createPlayerInputController);
	controllerLibrary.registerFunction(vx::make_sid("default"), createDefaultInputController);
	controllerLibrary.registerFunction(vx::make_sid("static"), createStaticInputController);

	Engine engine;
	g_engine = &engine;

	PhysicsAspect physicsAspect;
	SystemAspect systemAspect;

	EngineBaseDesc desc;
	desc.physicsAspectDesc.allocator = &mainAlloc;
	desc.physicsAspectDesc.capacityDynamic = 128;
	desc.physicsAspectDesc.capacityGravityAreas = 64;
	desc.physicsAspectDesc.capacityGravityWell = 64;
	desc.physicsAspectDesc.capacityStatic = 128;
	desc.physicsAspectDesc.onCollisionCallback = OnCollisionCallback;
	desc.mainAllocator = &mainAlloc;
	desc.onKeyPressed = onKeyPressed;
	desc.onKeyReleased = onKeyReleased;
	desc.physicsAspect = &physicsAspect;
	desc.resolution = vx::uint2(1280, 720);
	desc.logFile = &logfile;
	sprintf(desc.rootDir, "../");
	desc.systemAspect = &systemAspect;
	desc.controllerLibrary = &controllerLibrary;
	desc.destroyInputControllerFunction = destroyInputController;
	if (!engine.initialize(desc))
	{
		logfile.append("error initializing engine !\n");
		shutdown();
		return 1;
	}

	if (!dev::loadTextures(&engine, &mainAlloc))
	{
		logfile.append("could not load textures\n");
		shutdown();
		return 1;
	}

	const char *levelFile = argv[0];
	if (!loadLevel("untitled.level", &engine, &mainAlloc))
	{
		logfile.append("could not load level\n");
		shutdown();
		return 1;
	}

	/*if (!dev::createLevel(&engine, &mainAlloc))
	{
		shutdown();
		return 1;
	}

	/*
	
	*/

	engine.start(&mainAlloc, &allocatorDebugLayer);
	shutdown();

	return 0;
}
#endif