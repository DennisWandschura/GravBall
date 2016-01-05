#include "EntityAspect.h"
#include <gamelib/Event.h>
#include "RenderAspect.h"
#include <gamelib/LevelFile.h>
#include "Locator.h"
#include "ResourceAspect.h"
#include <gamelib/MainAllocator.h>
#include "PhysicsAspect.h"
#include <gamelib/PhysicsComponent.h>
#include <gamelib/GravityArea.h>
#include "GravityAreaFunctors.h"
#include <gamelib/ControllerLibrary.h>
#include "EventManager.h"
#include "Engine.h"

EntityAspect::EntityAspect()
	:m_dynamicEntities(),
	m_dynamicGravityAreas(),
	m_playerPhysicsComponent(nullptr),
	m_goalPhysicsComponent(nullptr),
	m_allocIndex(0),
	m_entityCapacity(0),
	m_renderDataAllocator(),
	m_entitiesToDestroy(),
	m_entitiesToDestroyCount(0),
	m_physicsAspect(nullptr),
	m_mainAllocator(nullptr),
	m_controllerLibrary(nullptr),
	m_destroyInputControllerFunction(nullptr),
	m_engine(nullptr)
{

}

EntityAspect::~EntityAspect()
{

}

bool EntityAspect::initialize(const EntityAspectDesc &desc)
{
	vx::array<DynamicEntity, DebugAllocatorEntity<Allocator::MainAllocator>> entities(desc.allocator, desc.dynamicEntityCapacity);
	m_dynamicEntities.swap(entities);

	m_tmpDynamicEntities = vx::array<DynamicEntity, DebugAllocatorEntity<Allocator::MainAllocator>>(desc.allocator, desc.dynamicEntityCapacity);

	m_entitiesToDestroy = vx::array<u8, DebugAllocatorEntity<Allocator::MainAllocator>>(desc.allocator, desc.dynamicEntityCapacity);

	vx::array<DynamicGravityArea, DebugAllocatorEntity<Allocator::MainAllocator>> dynamicGravityAreas(desc.allocator, desc.gravityAreaCapacity);
	m_dynamicGravityAreas.swap(dynamicGravityAreas);

	for (u32 i = 0; i < ALLOC_COUNT; ++i)
	{
		m_renderDataAllocator[i].initialize(desc.allocator->allocate(sizeof(RenderComponent) * desc.dynamicEntityCapacity, __alignof(RenderComponent), Allocator::Channels::Entity, "render data"));
	}

	m_entityCapacity = desc.dynamicEntityCapacity;
	m_physicsAspect = desc.physicsAspect;
	m_mainAllocator = desc.allocator;
	m_controllerLibrary = desc.controllerLibrary;
	m_destroyInputControllerFunction = desc.destroyInputControllerFunction;
	m_engine = desc.engine;

	return true;
}

void EntityAspect::shutdown(Allocator::MainAllocator* allocator)
{
	for (u32 i = 0; i < ALLOC_COUNT; ++i)
	{
		allocator->deallocate(m_renderDataAllocator[i].release(), Allocator::Channels::Entity, "render data");
	}

	for (auto &it : m_dynamicGravityAreas)
	{
		size_t size;
		auto ptr = it.changeForceFunctor.release(&size);

		auto tmpAlloc = DebugAllocatorEntity<Allocator::MainAllocator>(m_mainAllocator);
		tmpAlloc.deallocate({ (u8*)ptr, size });
	}
	m_dynamicGravityAreas.release();

	m_entitiesToDestroy.release();

	m_tmpDynamicEntities.release();

	for (auto &it : m_dynamicEntities)
	{
		m_destroyInputControllerFunction(it.inputController);
	}
	m_dynamicEntities.release();

	m_mainAllocator = nullptr;
}

void EntityAspect::updateControllers(f32 dt)
{
	auto vdt = _mm_load1_ps(&dt);
	for (auto &it : m_dynamicEntities)
	{
		if (it.inputController != nullptr)
		{
			it.inputController->update(vdt, &it);
		}
	}
}

void EntityAspect::updateDynamicGravityAreas(f32 dt)
{
	for (auto &it : m_dynamicGravityAreas)
	{
		if (it.timer.update(dt))
		{
			(*it.changeForceFunctor)(&it.currentForce);

			it.physicsComponent->force = it.currentForce;
		}
	}
}

void EntityAspect::update(RenderAspect* renderAspect)
{
	auto entityCount = m_dynamicEntities.size();
	if (entityCount == 0)
		return;

	if (m_entitiesToDestroyCount != 0)
	{
		removeAndUpdateEntities();
	}

	auto allocator = &m_renderDataAllocator[m_allocIndex];
	vx::array<RenderComponent, vx::DelegateAllocator<vx::LinearAllocator>> components(allocator, entityCount);

	for (auto &it : m_dynamicEntities)
	{
		auto position = it.physicsComponent->m_position;

		RenderComponent component;
		component.position = vx::float2(position.x, position.y);
		component.rendererIndex = it.rendererIndex;
		components.push_back(component);
	}

	RenderFrameDynamicData frameData;
	frameData.components.swap(components);
	frameData.cameraPosition = m_dynamicEntities[0].physicsComponent->m_position;

	renderAspect->pushFrameData(std::move(frameData));
	frameData.components.release();

	m_allocIndex = (m_allocIndex + 1) % ALLOC_COUNT;
}

void EntityAspect::removeAndUpdateEntities()
{
	auto cap = m_dynamicEntities.capacity();

	auto size = m_dynamicEntities.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (m_entitiesToDestroyCount == 0)
			break;

		auto mask = m_entitiesToDestroy[i];
		if (mask == 0)
		{
			auto newIndex = m_tmpDynamicEntities.size();
			m_tmpDynamicEntities.push_back(m_dynamicEntities[i]);

			auto &newEntity = m_tmpDynamicEntities.back();
			newEntity.physicsComponent->m_userData = &newEntity;

			newEntity.physicsComponent = m_physicsAspect->updateComponent(*newEntity.physicsComponent);
		}
		else
		{
			m_destroyInputControllerFunction(m_dynamicEntities[i].inputController);
			m_dynamicEntities[i].inputController = nullptr;
			m_entitiesToDestroy[i] = 0;
			--m_entitiesToDestroyCount;
		}
	}

	m_dynamicEntities.swap(m_tmpDynamicEntities);
	m_tmpDynamicEntities.clear();

	auto newSize = m_dynamicEntities.size();

	auto playerEntity = &m_dynamicEntities[0];
	m_playerPhysicsComponent = m_dynamicEntities[0].physicsComponent;

	for (auto &it : m_dynamicEntities)
	{
		it.inputController->setValue(playerEntity);
	}

	auto evtData = new Event::RendererRebuildDynamicEntitiesData();
	evtData->data = m_dynamicEntities.data();
	evtData->count = m_dynamicEntities.size();

	auto& evtManager = Locator::getEventManager();

	vx::Variant data;
	data.ptr = evtData;
#if _EDITOR
	evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RendererRebuildDynamicEntities, data, nullptr));
#else
	evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RendererRebuildDynamicEntities, data));
#endif
}

void EntityAspect::handleFileEvent(const Event::Event &evt)
{
	auto fileEvent = evt.asFileEvent();
	if (fileEvent == Event::Type::FileEventTypes::LoadedLevel)
	{
		vx::StringID sid(evt.getData().u64);
		auto level = Locator::getResourceAspect().findLevel(sid);

		m_levelSid = sid;
		loadLevel(*level);
	}
}

void EntityAspect::handleEditorEvent(const Event::Event &evt)
{
	auto type = evt.asEditorEvent();
	switch (type)
	{
	case Event::Type::EditorEventTypes::RendererCreatedPlayer:
	{
		auto data = (EditorDynamicEntityFile*)evt.getData().ptr;

		DynamicEntity player{};
		player.inputController = m_controllerLibrary->createController(data->controller, nullptr);
		player.rendererIndex = data->transformOffset;
		player.textureSid = data->texture;

		m_dynamicEntities.push_back(player);
		m_entitiesToDestroy.push_back((u8)0);

		auto entity = &m_dynamicEntities.back();
		m_playerPhysicsComponent = entity->physicsComponent = m_physicsAspect->addDynamicShape(vx::float2a(0, 0), 1, 1, 1.0f, entity);

		data->createdPhysics = 1;

	}break;
	default:
		break;
	}
}

void EntityAspect::handleEventIngame(const Event::Event &evt)
{
	auto type = evt.asIngameEvent();
	switch (type)
	{
	case Event::Type::IngameEventTypes::DestroyDynamicEntity:
	{
		DynamicEntity* ptr = (DynamicEntity*)evt.getData().ptr;
		u32 index = (u32)(ptr - m_dynamicEntities.data());

		m_entitiesToDestroy[index] = 1;
		++m_entitiesToDestroyCount;
	}break;
#if !_EDITOR
	case Event::Type::IngameEventTypes::FinishedLevel:
	{
		m_engine->stop();
	}break;
	case Event::Type::IngameEventTypes::RestartLevelBegin:
	{
		Event::RestartLevelData &p = *((Event::RestartLevelData*)&evt.getData().ptr);
		if (p.restartEntity == 0)
		{
			reset();
			p.restartEntity = 1;
		}

		if (p.finished())
		{
			auto &evtManager = Locator::getEventManager();
			evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RestartLevel, vx::Variant()));
		}
	}break;
	case Event::Type::IngameEventTypes::RestartLevel:
	{
		vx::Variant data;
		data.u64 = m_levelSid.value;
		auto &evtManager = Locator::getEventManager();
		evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RestartLevelFinished, data));
	}break;
	case Event::Type::IngameEventTypes::RestartLevelFinished:
	{
		vx::StringID sid(evt.getData().u64);
		auto level = Locator::getResourceAspect().findLevel(sid);

		m_levelSid = sid;
		loadLevel(*level);
	}break;
#endif
	default:
		break;
	}
}

void EntityAspect::handleEvent(const Event::Event &evt)
{
	if (evt.getType() == Event::Type::EventTypeFile)
	{
		handleFileEvent(evt);
	}
	else if (evt.getType() == Event::Type::EventTypeIngame)
	{
		handleEventIngame(evt);
	}
	else if (evt.getType() == Event::Type::EventTypeEditor)
	{
		handleEditorEvent(evt);
	}
}

void EntityAspect::onCollisionCallback(__m128 direction, PhysicsComponent* c0, PhysicsComponent* c1)
{
	PhysicsComponent* playerComponent = nullptr;
	PhysicsComponent* otherComponent = nullptr;
	if (c0 == m_playerPhysicsComponent)
	{
		playerComponent = c0;
		otherComponent = c1;
	}
	else if (c1 == m_playerPhysicsComponent)
	{
		playerComponent = c1;
		otherComponent = c0;
	}
#if !_EDITOR
	if (playerComponent)
	{
		if (otherComponent->m_type == PhysicsType::Dynamic)
		{
			auto mass = 1.0f / otherComponent->m_invMass;
			playerComponent->m_force.x -= direction.m128_f32[0] * 10000 * mass;
			playerComponent->m_force.y -= direction.m128_f32[1] * 10000 * mass;

			auto& evtManager = Locator::getEventManager();

			//data.ptr = otherComponent->m_userData;
			//evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::DestroyDynamicEntity, data));

			//data.u64 = m_levelSid.value;
			vx::Variant data;
			data.u64 = 0xffffffff;
			Event::RestartLevelData &p = *((Event::RestartLevelData*)&data.ptr);
			p.restartEntity = 0;
			p.restartPhysics = 0;
			p.restartRender = 0;
			evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RestartLevelBegin, data));
		}
		else if (otherComponent == m_goalPhysicsComponent)
		{
			auto& evtManager = Locator::getEventManager();
			evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::FinishedLevel, vx::Variant()));
		}
	}
	else if (c0->m_type == PhysicsType::Dynamic && c1->m_type == PhysicsType::Dynamic)
	{

	}
#endif
}

void EntityAspect::reset()
{
	for (auto &it : m_dynamicGravityAreas)
	{
		auto tmpAlloc = DebugAllocatorEntity<Allocator::MainAllocator>(m_mainAllocator);

		vx::AllocatedBlock block;
		block.ptr = (u8*)it.changeForceFunctor.release(&block.size);

		tmpAlloc.deallocate(block);
	}
	m_dynamicGravityAreas.clear();

	for (auto &it : m_dynamicEntities)
	{
		m_destroyInputControllerFunction(it.inputController);
		it.inputController = nullptr;
	}
	m_dynamicEntities.clear();

	m_entitiesToDestroy.clear();
	m_playerPhysicsComponent = nullptr;

	m_goalPhysicsComponent = nullptr;
}

void EntityAspect::loadLevel(const LevelFile &level)
{
	size_t gravityAreaCount = 0;
	auto gravityAreas = level.getGravityAreas(&gravityAreaCount);
	for (size_t i = 0; i < gravityAreaCount; ++i)
	{
		auto &it = gravityAreas[i];
		if (it.type == GravityAreaType::Dynamic)
		{
			auto physicsComponent = m_physicsAspect->getGravityArea(i);

			auto tmpAlloc = DebugAllocatorEntity<Allocator::MainAllocator>(m_mainAllocator);
			auto functor = vx::make_unique<GravityAreaInvertForce>(&tmpAlloc);
			size_t allocatedSize;
			auto ptr = functor.release(&allocatedSize);

			DynamicGravityArea dynamicEntity{};
			dynamicEntity.changeForceFunctor = { ptr, allocatedSize };
			dynamicEntity.currentForce = it.force;
			dynamicEntity.physicsComponent = physicsComponent;
			dynamicEntity.timer = Timer(it.time);

			m_dynamicGravityAreas.push_back(std::move(dynamicEntity));
		}
	}

	auto dynamicOffset = level.getDynamicOffset();

	{
		auto &playerEntity = level.getPlayerEntity();
		DynamicEntity player{};
		player.rendererIndex = dynamicOffset++;
		player.inputController = m_controllerLibrary->createController(playerEntity.controller, nullptr);
		player.textureSid = playerEntity.texture;

		m_dynamicEntities.push_back(player);
		m_entitiesToDestroy.push_back((u8)0);

		auto entity = &m_dynamicEntities.back();
		m_playerPhysicsComponent = entity->physicsComponent = m_physicsAspect->addDynamicShape(playerEntity.position, playerEntity.shape.radius.x, playerEntity.mass, playerEntity.restitution, entity);
	}

	auto goalArea = level.getGoalArea();
	m_goalPhysicsComponent = m_physicsAspect->addStaticShape(goalArea.position, goalArea.halfDim, 0, nullptr);

	size_t staticEntityCount;
	auto staticEntities = level.getStaticEntities(&staticEntityCount);
	for (size_t i = 0; i < staticEntityCount; ++i)
	{
		auto &it = staticEntities[i];
		auto type = it.shape.type;
		switch (type)
		{
		case EntityShapeType::Rectangle:
			m_physicsAspect->addStaticShape(it.position, it.shape.halfDim, it.restitution, nullptr);
			break;
		case EntityShapeType::Circle:
			m_physicsAspect->addStaticShape(it.position, it.shape.radius.x, it.restitution, nullptr);
			break;
		default:
			break;
		}
	}

	size_t dynamicEntityCount = 0;
	auto dynamicEntities = level.getDynamicEntities(&dynamicEntityCount);
	for (size_t i = 0; i < dynamicEntityCount; ++i)
	{
		auto &it = dynamicEntities[i];

		auto shapeType = it.getShapeType();

		DynamicEntity entity{};
		entity.rendererIndex = dynamicOffset++;
		entity.inputController = m_controllerLibrary->createController(it.controller, &m_dynamicEntities[0]);
		entity.textureSid = it.texture;

		m_dynamicEntities.push_back(entity);
		m_entitiesToDestroy.push_back((u8)0);

		auto entityPtr = &m_dynamicEntities.back();

		switch (shapeType)
		{
		case EntityShapeType::Rectangle:
			VX_ASSERT(false);
			break;
		case EntityShapeType::Circle:
			entityPtr->physicsComponent = m_physicsAspect->addDynamicShape(it.position, it.shape.radius.x, it.mass, it.restitution, entityPtr);
			break;
		default:
			break;
		}
	}
}