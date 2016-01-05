#include "CreateLevel.h"
#include <gamelib/LevelFile.h>
#include "Engine.h"
#include <gamelib/EntityFile.h>
#include <vxLib/File/FileHandle.h>
#include <gamelib/MainAllocator.h>
#include "BmpFile.h"
#include <gamelib/Event.h>
#include <gamelib/GravityWell.h>
#include <gamelib/GravityArea.h>

namespace dev
{
	struct LoadFileHelperImpl : public LoadFileHelper
	{
		Allocator::MainAllocator* m_allocator;

		explicit LoadFileHelperImpl(Allocator::MainAllocator* alloc) :m_allocator(alloc) {}

		vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			return m_allocator->allocate(size, alignment, Allocator::Channels::General, "create level bmp");
		}

		vx::AllocatedBlock allocateTemporary(size_t size, size_t alignment)
		{
			return m_allocator->allocate(size, alignment, Allocator::Channels::General, "create level bmp tmp");
		}

		void deallocateTemporary(const vx::AllocatedBlock &block)
		{
			m_allocator->deallocate(block, Allocator::Channels::General, "create level bmp tmp");
		}
	};

	bool loadTexture(const char* dir, const vx::FileHandle &handle, Allocator::MainAllocator* alloc, Texture* outTexture)
	{
		char file[256];
		snprintf(file, 256, "%s%s", dir, handle.m_string);

		LoadFileHelperImpl helper(alloc);

		BmpFile bmp;
		if (!bmp.loadFromFile(file, &helper))
			return false;

		Texture texture(std::move(bmp));
		outTexture->swap(texture);

		return true;
	}

	bool loadTextures(Engine* engine, Allocator::MainAllocator* allocator)
	{
		auto texGreenHandle = vx::FileHandle("green.bmp");
		auto texPlayerHandle = vx::FileHandle("player.bmp");
		auto texEnemyHandle = vx::FileHandle("enemy.bmp");
		auto texGravityAreaStaticHandle = vx::FileHandle("gravityAreaStatic.bmp");
		auto texGravityWellHandle = vx::FileHandle("gravityWell.bmp");
		auto texGoalHandle = vx::FileHandle("goal.bmp");
		Texture texGreen;
		if (!loadTexture("../data/textures/", texGreenHandle, allocator, &texGreen))
		{
			return false;
		}

		Texture texPlayer;
		if (!loadTexture("../data/textures/", texPlayerHandle, allocator, &texPlayer))
		{
			return false;
		}

		Texture texArea;
		if (!loadTexture("../data/textures/", texGravityAreaStaticHandle, allocator, &texArea))
		{
			return false;
		}

		Texture texGravWell;
		if (!loadTexture("../data/textures/", texGravityWellHandle, allocator, &texGravWell))
		{
			return false;
		}

		Texture texEnemy;
		if (!loadTexture("../data/textures/", texEnemyHandle, allocator, &texEnemy))
		{
			return false;
		}

		Texture texGoal;
		if (!loadTexture("../data/textures/", texGoalHandle, allocator, &texGoal))
		{
			return false;
		}

		engine->addTexture(texGreenHandle.m_sid, std::move(texGreen));
		engine->addTexture(texPlayerHandle.m_sid, std::move(texPlayer));
		engine->addTexture(texGravityAreaStaticHandle.m_sid, std::move(texArea));
		engine->addTexture(texGravityWellHandle.m_sid, std::move(texGravWell));
		engine->addTexture(texEnemyHandle.m_sid, std::move(texEnemy));
		engine->addTexture(texGoalHandle.m_sid, std::move(texGoal));

		return true;
	}

	bool createLevel(Engine* engine, Allocator::MainAllocator* allocator)
	{
		auto texGreenHandle = vx::FileHandle("green.bmp");
		auto texPlayerHandle = vx::FileHandle("player.bmp");
		auto texEnemyHandle = vx::FileHandle("enemy.bmp");
		auto texGravityAreaStaticHandle = vx::FileHandle("gravityAreaStatic.bmp");
		auto texGravityWellHandle = vx::FileHandle("gravityWell.bmp");
		auto texGoalHandle = vx::FileHandle("goal.bmp");

		vx::array<vx::StringID, DebugAllocatorResource<Allocator::MainAllocator>> textures(allocator, 8);
		textures.push_back(texGreenHandle.m_sid);
		textures.push_back(texPlayerHandle.m_sid);
		textures.push_back(texGravityAreaStaticHandle.m_sid);
		textures.push_back(texGravityWellHandle.m_sid);
		textures.push_back(texEnemyHandle.m_sid);
		textures.push_back(texGoalHandle.m_sid);

		vx::array<StaticEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> staticEntities(allocator, 64);
		staticEntities.push_back(StaticEntityFile
		{
			texGreenHandle.m_sid,
			{0, 5},
			{
				vx::float2{5, 1},
				EntityShapeType::Rectangle
			},
			1.0f
		});

		staticEntities.push_back(StaticEntityFile
		{
			texGreenHandle.m_sid,
			{ 3, 0 },
			{
				vx::float2{ 1, 1 },
				EntityShapeType::Rectangle
			},
			1.0f
		});

		staticEntities.push_back(StaticEntityFile
		{
			texGreenHandle.m_sid,
			{ -3, 0 },
			{
				vx::float2{ 1, 1 },
				EntityShapeType::Rectangle
			},
			1.0f
		});

		vx::array<DynamicEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> dynamicEntities(allocator, 8);
		dynamicEntities.push_back(DynamicEntityFile(
		{
			vx::make_sid("default"),
			texEnemyHandle.m_sid,
			{ -7, 0 },
			{
				vx::float2{ 1, 1 },
				EntityShapeType::Circle
			},
			1.0f,
			50.0f
		}));

		LevelFile level;
		level.setTextures(&textures);
		level.setStaticEntities(&staticEntities);
		level.setDynamicEntities(&dynamicEntities);
		level.setPlayerEntity(DynamicEntityFile
		{
			vx::make_sid("player"),
			texPlayerHandle.m_sid,
			{ 0, 0 },
			{
				vx::float2{ 1, 1 },
				EntityShapeType::Circle
			},
			1.0f,
			100.0f
		});

		vx::array<GravityAreaFile, DebugAllocatorResource<Allocator::MainAllocator>> gravityAreas(allocator, 4);
		gravityAreas.push_back(
			GravityAreaFile::createDynamic(2.0f, { 0, 0 }, { 5, 5 }, { 0, -500 }, texGravityAreaStaticHandle.m_sid));

		level.setGravityAreas(&gravityAreas);

		vx::array<GravityWellFile, DebugAllocatorResource<Allocator::MainAllocator>> gravityWells(allocator, 4);
		gravityWells.push_back(GravityWellFile::create(vx::float2a(10, 0), 5.f, 100.f, texGravityWellHandle.m_sid));
		level.setGravityWells(&gravityWells);

		GoalEntityFile goalEntity;
		goalEntity.position = {-5, 0};
		goalEntity.halfDim = {1, 1};
		goalEntity.textureSid = texGoalHandle.m_sid;
		level.setGoalArea(goalEntity);

		auto sidLevel = vx::make_sid("default");
		engine->addLevel(sidLevel, std::move(level));

		vx::Variant evtData;
		evtData.u64 = sidLevel.value;
		Event::Event evt(Event::Type::FileEventTypes::LoadedLevel, evtData);
		engine->pushEvent(evt);

		return true;
	}
}