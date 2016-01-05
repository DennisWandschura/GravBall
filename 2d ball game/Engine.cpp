#include "Engine.h"
#include <vxLib/CpuTimer.h>
#include <cstdio>
#include "Keyboard.h"
#include <gamelib/Event.h>
#include "Locator.h"
#include <gamelib/AllocatorDebugLayer.h>
#include <vxLib/print.h>
#include <gamelib/MainAllocator.h>
#include <gamelib/PhysicsAspectBase.h>
#include <gamelib/Logfile.h>

namespace EngineCpp
{
	struct ThreadData
	{
		std::atomic_uint** isRunning;
		ThreadWriteToFileTask* task;
	};

	void helperThreadMain(ThreadData* data)
	{
		std::atomic_uint isRunning{ 1 };

		ThreadWriteToFileTask* task = data->task;
		*(data->isRunning) = &isRunning;

		u8 buffer[2048];
		vx::allocate_console({ buffer, 2048 });

		//vx::printf("thread start\n");

		while (isRunning.load(std::memory_order_acquire) != 0)
		{
			if (task->hasData.load(std::memory_order_acquire) != 0)
			{
				//vx::printf("write to file\n");

				task->file->write(task->data.ptr, task->dataSize);
				task->file->flush();
				task->hasData.store(0, std::memory_order_release);
			}
		}

		//vx::printf("thread close\n");

		vx::deallocate_console();
	}
}

Engine::Engine()
	:m_isRunning(0),
	m_logFile(nullptr),
	m_eventManager(),
	m_systemAspect(nullptr),
	m_renderAspect(),
	m_physicsAspect(nullptr),
	m_audioAspect(),
	m_resourceAspect(),
	m_threadIsRunning(nullptr),
	m_helperThread(),
	m_threadData()
{
}

Engine::~Engine()
{

}

bool Engine::initialize(const EngineBaseDesc &desc)
{
	auto logfile = desc.logFile;

	logfile->append("trying to initializing cpu profiler\n");
	if (!m_cpuProfiler.initialize(desc.mainAllocator))
	{
		logfile->append("error initializing cpu profiler\n");
		return false;
	}
	logfile->append("initialized cpu profiler\n");

	logfile->append("trying to initializing event manager\n");
	if (!m_eventManager.initialize(desc.mainAllocator, 64, 8))
	{
		logfile->append("error initializing event manager\n");
		return false;
	}
	logfile->append("initialized event manager\n");

	logfile->append("trying to initializing resource aspect\n");
	if (!m_resourceAspect.initialize(desc.mainAllocator, &m_eventManager, desc.rootDir))
	{
		logfile->append("error initializing resource aspect\n");
		return false;
	}
	logfile->append("initialized resource aspect\n");

	logfile->append("trying to initializing system aspect\n");
	if (!desc.systemAspect->initialize(desc.resolution, false, desc.onKeyPressed, desc.onKeyReleased, desc.mainAllocator))
	{
		logfile->append("error initializing system aspect\n");
		return false;
	}
	logfile->append("initialized system aspect\n");

	m_systemAspect = desc.systemAspect;

	auto dynamicEntityCapacity = desc.physicsAspectDesc.capacityDynamic;
	auto staticEntityCapacity = desc.physicsAspectDesc.capacityStatic;// +desc.physicsAspectDesc.capacityGravityAreas + desc.physicsAspectDesc.capacityGravityWell + 1;

	logfile->append("trying to initializing render aspect\n");
	if (!m_renderAspect.initialize(m_systemAspect->getHwnd(), desc.resolution, true, desc.mainAllocator, desc.rootDir, dynamicEntityCapacity, staticEntityCapacity))
	{
		logfile->append("error initializing render aspect\n");
		return false;
	}
	logfile->append("initialized render aspect\n");

	logfile->append("trying to initializing physics aspect\n");
	if (!desc.physicsAspect->initialize(desc.physicsAspectDesc))
	{
		logfile->append("error initializing physics aspect\n");
		return false;
	}
	logfile->append("initialized physics aspect\n");

	m_physicsAspect = desc.physicsAspect;

	logfile->append("trying to initializing audio aspect\n");
	if (!m_audioAspect.initialize())
	{
		logfile->append("error initializing audio aspect\n");
		return false;
	}
	logfile->append("initialized audio aspect\n");

	EntityAspectDesc entityAspectDesc;
	entityAspectDesc.allocator = desc.mainAllocator;
	entityAspectDesc.dynamicEntityCapacity = desc.physicsAspectDesc.capacityDynamic;
	entityAspectDesc.gravityAreaCapacity = desc.physicsAspectDesc.capacityGravityAreas;
	entityAspectDesc.physicsAspect = m_physicsAspect;
	entityAspectDesc.destroyInputControllerFunction = desc.destroyInputControllerFunction;
	entityAspectDesc.controllerLibrary = desc.controllerLibrary;
	entityAspectDesc.engine = this;
	logfile->append("trying to initializing entity aspect\n");
	if (!m_entityAspect.initialize(entityAspectDesc))
	{
		logfile->append("error initializing entity aspect\n");
		return false;
	}
	logfile->append("initialized entity aspect\n");

	m_threadData.data = desc.mainAllocator->allocate(64 KBYTE, 64, Allocator::Channels::General, "write file data");

	logfile->append("starting helper thread\n");
	EngineCpp::ThreadData threadData;
	threadData.isRunning = &m_threadIsRunning;
	threadData.task = &m_threadData;
	m_helperThread = std::thread(EngineCpp::helperThreadMain, &threadData);

	logfile->append("registering aspects for events\n");
	m_eventManager.registerListener(&m_renderAspect, Event::Type::EventTypeFile | Event::Type::EventTypeIngame | Event::Type::EventTypeEditor);
	m_eventManager.registerListener(&m_entityAspect, Event::Type::EventTypeFile | Event::Type::EventTypeIngame | Event::Type::EventTypeEditor);
	m_eventManager.registerListener(m_physicsAspect, Event::Type::EventTypeFile | Event::Type::EventTypeIngame | Event::Type::EventTypeEditor);
	m_eventManager.registerListener(&m_resourceAspect, Event::Type::EventTypeFile);

	Locator::set(&m_eventManager);
	Locator::set(&m_resourceAspect);

	logfile->append("finished initializing\n");
	m_logFile = logfile;
	return true;
}

void Engine::shutdown(Allocator::MainAllocator* mainAllocator)
{
	m_logFile->append("starting shutdown\n");
	Locator::reset();

	if (m_threadIsRunning != nullptr)
	{
		m_threadIsRunning->store(0, std::memory_order_release);

		while (!m_helperThread.joinable())
			;

		m_helperThread.join();
	}

	mainAllocator->deallocate(m_threadData.data, Allocator::Channels::General, "write file data");

	m_entityAspect.shutdown(mainAllocator);
	m_audioAspect.shutdown();
	if (m_physicsAspect)
	{
		m_physicsAspect->shutdow(mainAllocator);
		m_physicsAspect = nullptr;
	}
	m_renderAspect.shutdown(mainAllocator);
	if (m_systemAspect != nullptr)
	{
		m_systemAspect->shutdown(mainAllocator);
		m_systemAspect = nullptr;
	}
	m_resourceAspect.shutdown();
	m_eventManager.shutdown();

	m_cpuProfiler.shutdown(mainAllocator);
	m_logFile->append("finished shutdown\n");
}

void Engine::updateImpl(f32 dt, Allocator::MainAllocator*)
{
	m_eventManager.update();

	m_systemAspect->update();

	CpuSample()(m_cpuProfiler, "entity controllers", [&]()
	{
		m_entityAspect.updateControllers(dt);
	});

	CpuSample()(m_cpuProfiler, "dynamic gravity", [&]()
	{
		m_entityAspect.updateDynamicGravityAreas(dt);
	});

	CpuSample()(m_cpuProfiler, "physics", [&]()
	{
		m_physicsAspect->update(dt);
	});

	CpuSample()(m_cpuProfiler, "entity render", [&]()
	{
		m_entityAspect.update(&m_renderAspect);
	});
}

void Engine::start(Allocator::MainAllocator* mainAllocator, Allocator::DebugLayer* debugLayer)
{
	m_isRunning = 1;
	mainLoop(mainAllocator, debugLayer);
}

void Engine::stop()
{
	m_logFile->append("stopping engine");
	m_isRunning = 0;
}

void Engine::mainLoop(Allocator::MainAllocator* mainAllocator, Allocator::DebugLayer* debugLayer)
{
	const f32 dt = 1.0f / 60.0f;

	vx::CpuTimer timer;

	f32 accum = 0.0f;

	while (m_isRunning != 0)
	{
		m_cpuProfiler.frameStart(&m_threadData);

		auto frameTime = timer.getTimeSeconds();
		timer.reset();

		accum += frameTime;

		debugLayer->frame();

		while (accum >= dt)
		{
			CpuSample()(m_cpuProfiler, "update", [&]()
			{
				m_cpuProfiler.update(&m_threadData);
				updateImpl(dt, mainAllocator);
			});

			accum -= dt;
		}

		CpuSample()(m_cpuProfiler, "render update", [&]()
		{
			m_renderAspect.update(&m_cpuProfiler);
		});

		m_cpuProfiler.frameEnd();
	}
}

void Engine::render()
{
	m_renderAspect.update(&m_cpuProfiler);
}

void Engine::onKeyPressed(vx::Input::Keys)
{

}

void Engine::onKeyReleased(vx::Input::Keys keyCode)
{
	if (keyCode == vx::Input::Keys::Esc)
	{
		stop();
	}
}

void Engine::addLevel(const vx::StringID &sid, LevelFile &&level)
{
	m_logFile->append("adding level ");
	m_logFile->append(sid.value);
	m_logFile->append('\n');
	m_resourceAspect.addLevel(sid, std::move(level));
}

void Engine::addTexture(const vx::StringID &sid, Texture &&texture)
{
	m_logFile->append("adding texture ");
	m_logFile->append(sid.value);
	m_logFile->append('\n');
	m_resourceAspect.addTexture(sid, std::move(texture));
}

void Engine::pushEvent(const Event::Event &evt)
{
	m_eventManager.pushEvent(evt);
}

void Engine::onCollisionCallback(__m128 dir, PhysicsComponent* c0, PhysicsComponent* c1)
{
	m_entityAspect.onCollisionCallback(dir, c0, c1);
}

void Engine::getViewMatrix(vx::mat4* m) const
{
	return m_renderAspect.getViewMatrix(m);
}

void Engine::getProjectionMatrix(vx::mat4* m) const
{
	return m_renderAspect.getProjectionMatrix(m);
}

__m128 Engine::getCameraPosition() const
{
	return m_renderAspect.getCameraPosition();
}