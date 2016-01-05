#pragma once

#include <vxLib/types.h>

namespace vx
{
	namespace Input
	{
		enum class Keys : u16;
	}
}

#include <gamelib/EngineBase.h>
#include "SystemAspect.h"
#include "RenderAspect.h"
#include "ResourceAspect.h"
#include "EventManager.h"
#include "EntityAspect.h"
#include "AudioAspect.h"
#include "CpuProfiler.h"
#include <thread>
#include <atomic>

class Engine : public EngineBase
{
	u64 m_isRunning;
	Logfile* m_logFile;
	CpuProfiler m_cpuProfiler;
	EventManager m_eventManager;
	SystemAspectBase* m_systemAspect;
	RenderAspect m_renderAspect;
	EntityAspect m_entityAspect;
	PhysicsAspectBase* m_physicsAspect;
	AudioAspect m_audioAspect;
	ResourceAspect m_resourceAspect;
	std::atomic_uint* m_threadIsRunning;
	std::thread m_helperThread;
	ThreadWriteToFileTask m_threadData;

	void mainLoop(Allocator::MainAllocator* mainAllocator, Allocator::DebugLayer* debugLayer);
	void updateImpl(f32 dt, Allocator::MainAllocator* mainAllocator);

public:
	Engine();
	~Engine();

	bool initialize(const EngineBaseDesc &desc) override;
	void shutdown(Allocator::MainAllocator* mainAllocator) override;

	void start(Allocator::MainAllocator* mainAllocator, Allocator::DebugLayer* debugLayer);
	void stop();

	void onKeyPressed(vx::Input::Keys keyCode);
	void onKeyReleased(vx::Input::Keys keyCode);

	void addLevel(const vx::StringID &sid, LevelFile &&level);
	void addTexture(const vx::StringID &sid, Texture &&texture);

	void pushEvent(const Event::Event &evt) override;

	void update(f32 dt, Allocator::MainAllocator* mainAllocator) override { updateImpl(dt, mainAllocator); }
	void render() override;

	void VX_CALLCONV onCollisionCallback(__m128 dir, PhysicsComponent* c0, PhysicsComponent* c1);

	void getViewMatrix(vx::mat4* m) const;
	void getProjectionMatrix(vx::mat4* m) const;

	__m128 getCameraPosition() const;
};