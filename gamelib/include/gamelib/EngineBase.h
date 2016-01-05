#pragma once

namespace Allocator
{
	class MainAllocator;
	class DebugLayer;
}

namespace Event
{
	class Event;
}

class PhysicsAspectBase;
class ControllerLibrary;
class Logfile;

#include <gamelib/SystemAspectBase.h>
#include <gamelib/PhysicsAspectBase.h>
#include <vxLib/math/matrix.h>
#include <gamelib/InputController.h>

struct EngineBaseDesc
{
	PhysicsAspectBaseDesc physicsAspectDesc;
	vx::uint2 resolution;
	Allocator::MainAllocator* mainAllocator;
	PhysicsAspectBase* physicsAspect;
	SystemAspectBase* systemAspect;
	KeyEventCallbackSignature onKeyPressed;
	KeyEventCallbackSignature onKeyReleased;
	ControllerLibrary* controllerLibrary;
	DestroyInputControllerFn destroyInputControllerFunction;
	Logfile* logFile;
	char rootDir[8];
};

class EngineBase
{
protected:
	EngineBase() {}

public:
	virtual ~EngineBase() {}

	virtual bool initialize(const EngineBaseDesc &desc) = 0;
	virtual void shutdown(Allocator::MainAllocator* mainAllocator) = 0;

	virtual void update(f32 dt, Allocator::MainAllocator* mainAllocator) = 0;
	virtual void render() = 0;

	virtual void pushEvent(const Event::Event &evt) = 0;

	virtual void getViewMatrix(vx::mat4* m) const = 0;
	virtual void getProjectionMatrix(vx::mat4* m) const =0;
	virtual __m128 getCameraPosition() const = 0;
};