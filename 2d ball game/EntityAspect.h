#pragma once

class RenderAspect;
class PhysicsAspectBase;
struct DynamicGravityArea;
class ControllerLibrary;
class Engine;
class LevelFile;

namespace Allocator
{
	class MainAllocator;
}

#include <gamelib/EventListener.h>
#include <gamelib/Entity.h>
#include <vxLib/Container/array.h>
#include <vxLib/Allocator/LinearAllocator.h>
#include <gamelib/DebugAllocator.h>
#include <gamelib/InputController.h>

struct EntityAspectDesc
{
	Allocator::MainAllocator* allocator;
	u32 dynamicEntityCapacity; 
	u32 gravityAreaCapacity; 
	PhysicsAspectBase* physicsAspect;
	ControllerLibrary* controllerLibrary;
	DestroyInputControllerFn destroyInputControllerFunction;
	Engine* engine;
};

class EntityAspect : public Event::ListenerInterface
{
	static const u32 ALLOC_COUNT = 2u;

	vx::array<DynamicEntity, DebugAllocatorEntity<Allocator::MainAllocator>> m_dynamicEntities;
	vx::array<DynamicGravityArea, DebugAllocatorEntity<Allocator::MainAllocator>> m_dynamicGravityAreas;
	PhysicsComponent* m_playerPhysicsComponent;
	PhysicsComponent* m_goalPhysicsComponent;
	u32 m_allocIndex;
	u32 m_entityCapacity;
	vx::LinearAllocator m_renderDataAllocator[ALLOC_COUNT];
	vx::array<u8, DebugAllocatorEntity<Allocator::MainAllocator>> m_entitiesToDestroy;
	size_t m_entitiesToDestroyCount;
	vx::array<DynamicEntity, DebugAllocatorEntity<Allocator::MainAllocator>> m_tmpDynamicEntities;
	PhysicsAspectBase* m_physicsAspect;
	Allocator::MainAllocator* m_mainAllocator;
	ControllerLibrary* m_controllerLibrary;
	DestroyInputControllerFn m_destroyInputControllerFunction;
	vx::StringID m_levelSid;
	Engine* m_engine;

	void handleFileEvent(const Event::Event &evt);
	void handleEditorEvent(const Event::Event &evt);
	void handleEventIngame(const Event::Event &evt);

	void removeAndUpdateEntities();

	void reset();
	void loadLevel(const LevelFile &level);

public:
	EntityAspect();
	~EntityAspect();

	bool initialize(const EntityAspectDesc &desc);
	void shutdown(Allocator::MainAllocator* allocator);

	void updateControllers(f32 dt);
	void updateDynamicGravityAreas(f32 dt);

	void update(RenderAspect* renderAspect);

	void handleEvent(const Event::Event &evt) override;

	void VX_CALLCONV onCollisionCallback(__m128 direction, PhysicsComponent* c0, PhysicsComponent* c1);
};