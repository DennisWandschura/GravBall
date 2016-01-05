#pragma once

struct StaticEntityFile;
struct GravityAreaFile;
struct GravityWellFile;

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/Container/array.h>
#include <vxLib/math/Vector.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <gamelib/EntityFile.h>
#include <gamelib/DebugAllocator.h>
#include <gamelib/AABB.h>

struct PlayerData
{
	vx::StringID texture;
	vx::float2 position;
	f32 radius;
};

class LevelFile
{
	vx::array<StaticEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> m_staticEntities;
	vx::array<DynamicEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> m_dynamicEntities;
	vx::array<vx::StringID, DebugAllocatorResource<Allocator::MainAllocator>> m_textures;
	vx::array<GravityAreaFile, DebugAllocatorResource<Allocator::MainAllocator>> m_gravityAreas;
	vx::array<GravityWellFile, DebugAllocatorResource<Allocator::MainAllocator>> m_gravityWells;
	DynamicEntityFile m_playerEntity;
	GoalEntityFile m_goalArea;

public:
	LevelFile();
	LevelFile(const LevelFile&) = delete;
	LevelFile(LevelFile &&rhs);
	~LevelFile();

	LevelFile& operator=(const LevelFile&) = delete;

	LevelFile& operator=(LevelFile &&rhs);

	void swap(LevelFile &other);

	void release();

	void setGoalArea(const GoalEntityFile &bounds) { m_goalArea = bounds; }
	const GoalEntityFile& getGoalArea() const { return m_goalArea; }

	bool loadFromMemory(const vx::AllocatedBlock &block, Allocator::MainAllocator* allocator);
	vx::AllocatedBlock saveToMemory(Allocator::MainAllocator* allocator, u64* size);

	void setPlayerEntity(const DynamicEntityFile &data);
	const DynamicEntityFile& getPlayerEntity() const;

	void setStaticEntities(vx::array<StaticEntityFile, DebugAllocatorResource<Allocator::MainAllocator>>* staticEntities);
	const StaticEntityFile* getStaticEntities(size_t* count) const;
	size_t getStaticEntityCount() const { return m_staticEntities.size(); }

	void setDynamicEntities(vx::array<DynamicEntityFile, DebugAllocatorResource<Allocator::MainAllocator>>* dynamicEntities);
	const DynamicEntityFile* getDynamicEntities(size_t* count) const;
	size_t getDynamicEntityCount() const { return m_dynamicEntities.size(); }

	void setTextures(vx::array<vx::StringID, DebugAllocatorResource<Allocator::MainAllocator>>* textures);
	const vx::StringID* getTextures(size_t* count) const;

	void setGravityAreas(vx::array<GravityAreaFile, DebugAllocatorResource<Allocator::MainAllocator>>* gravityAreas);
	const GravityAreaFile* getGravityAreas(size_t* count) const;

	void setGravityWells(vx::array<GravityWellFile, DebugAllocatorResource<Allocator::MainAllocator>>* gravityWells);
	const GravityWellFile* getGravityWells(size_t* count) const;

	u32 getDynamicOffset() const;
};