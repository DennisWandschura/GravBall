#include <gamelib/LevelFile.h>
#include <gamelib/MainAllocator.h>
#include <gamelib/EntityFile.h>
#include "gamelib/GravityWell.h"
#include "gamelib/GravityArea.h"
#include <vxLib/string.h>

namespace LevelFileCpp
{
	template<typename T, typename Alloc>
	const u8* loadArray(vx::array<T, Alloc>* dst, const u8* p, Allocator::MainAllocator* allocator)
	{
		u64 count = 0;
		p = vx::read(count, p);

		if (count != 0)
		{
			vx::array<T, Alloc> tmpData(Alloc(allocator), count);
			tmpData.assign(p, count);

			dst->swap(tmpData);
		}

		return p + sizeof(T) * count;
	};
}

LevelFile::LevelFile()
	:m_staticEntities(),
	m_dynamicEntities(),
	m_textures(),
	m_gravityAreas(),
	m_gravityWells(),
	m_playerEntity(),
	m_goalArea()
{

}

LevelFile::LevelFile(LevelFile &&rhs)
	:m_staticEntities(std::move(rhs.m_staticEntities)),
	m_dynamicEntities(std::move(rhs.m_dynamicEntities)),
	m_textures(std::move(rhs.m_textures)),
	m_gravityAreas(std::move(rhs.m_gravityAreas)),
	m_gravityWells(std::move(rhs.m_gravityWells)),
	m_playerEntity(rhs.m_playerEntity),
	m_goalArea(rhs.m_goalArea)
{

}

LevelFile::~LevelFile()
{

}

LevelFile& LevelFile::operator=(LevelFile &&rhs)
{
	if (this != &rhs)
	{
		swap(rhs);
	}
	return *this;
}

void LevelFile::swap(LevelFile &other)
{
	m_staticEntities.swap(other.m_staticEntities);
	m_dynamicEntities.swap(other.m_dynamicEntities);
	m_textures.swap(other.m_textures);
	m_gravityAreas.swap(other.m_gravityAreas);
	m_gravityWells.swap(other.m_gravityWells);
	std::swap(m_playerEntity, other.m_playerEntity);
	std::swap(m_goalArea, other.m_goalArea);
}

void LevelFile::release()
{
	m_gravityWells.release();
	m_gravityAreas.release();
	m_textures.release();
	m_dynamicEntities.release();
	m_staticEntities.release();
}

bool LevelFile::loadFromMemory(const vx::AllocatedBlock &block, Allocator::MainAllocator* allocator)
{
	auto ptr = LevelFileCpp::loadArray(&m_staticEntities, block.ptr, allocator);
	ptr = LevelFileCpp::loadArray(&m_dynamicEntities, ptr, allocator);
	ptr = LevelFileCpp::loadArray(&m_textures, ptr, allocator);
	ptr = LevelFileCpp::loadArray(&m_gravityAreas, ptr, allocator);
	ptr = LevelFileCpp::loadArray(&m_gravityWells, ptr, allocator);

	memcpy(&m_playerEntity, ptr, sizeof(m_playerEntity));
	ptr += sizeof(m_playerEntity);

	memcpy(&m_goalArea, ptr, sizeof(m_goalArea));
	ptr += sizeof(m_goalArea);

	VX_ASSERT(ptr <= (block.ptr + block.size));

	return true;
}

vx::AllocatedBlock LevelFile::saveToMemory(Allocator::MainAllocator* allocator, u64* size)
{
	auto totalSize =
		m_staticEntities.sizeInBytes() + m_dynamicEntities.sizeInBytes() + m_textures.sizeInBytes() + m_gravityAreas.sizeInBytes() + m_gravityWells.sizeInBytes()
		+ sizeof(u64) * 5 +
		+sizeof(m_playerEntity)
		+ sizeof(m_goalArea);

	auto block = allocator->allocate(totalSize, 4, Allocator::Channels::Resource, "level save data");

	auto saveArrayToMemory = [&](const auto &data, u8* p) 
	{
		u64 count = data.size();

		memcpy(p, &count, sizeof(count));
		p += sizeof(count);

		if (count != 0)
		{
			memcpy(p, data.data(), data.sizeInBytes());
			p += data.sizeInBytes();
		}

		return p;
	};

	auto ptr = saveArrayToMemory(m_staticEntities, block.ptr);
	ptr = saveArrayToMemory(m_dynamicEntities, ptr);
	ptr = saveArrayToMemory(m_textures, ptr);
	ptr = saveArrayToMemory(m_gravityAreas, ptr);
	ptr = saveArrayToMemory(m_gravityWells, ptr);

	memcpy(ptr, &m_playerEntity, sizeof(m_playerEntity));
	ptr += sizeof(m_playerEntity);

	memcpy(ptr, &m_goalArea, sizeof(m_goalArea));
	ptr += sizeof(m_goalArea);

	//auto writtenSie = ptr - block.ptr;
	VX_ASSERT(ptr <= (block.ptr + block.size));

	*size = (ptr - block.ptr);

	return block;
}

void LevelFile::setPlayerEntity(const DynamicEntityFile &data)
{
	m_playerEntity = data;
}

const DynamicEntityFile& LevelFile::getPlayerEntity() const
{
	return m_playerEntity;
}

void LevelFile::setStaticEntities(vx::array<StaticEntityFile, DebugAllocatorResource<Allocator::MainAllocator>>* staticEntities)
{
	m_staticEntities.swap(*staticEntities);
}

const StaticEntityFile* LevelFile::getStaticEntities(size_t* count) const
{
	*count = m_staticEntities.size();
	return m_staticEntities.data();
}

void LevelFile::setDynamicEntities(vx::array<DynamicEntityFile, DebugAllocatorResource<Allocator::MainAllocator>>* dynamicEntities)
{
	dynamicEntities->swap(m_dynamicEntities);
}

const DynamicEntityFile* LevelFile::getDynamicEntities(size_t* count) const
{
	*count = m_dynamicEntities.size();
	return m_dynamicEntities.data();
}

void LevelFile::setTextures(vx::array<vx::StringID, DebugAllocatorResource<Allocator::MainAllocator>>* textures)
{
	m_textures.swap(*textures);
}

const vx::StringID* LevelFile::getTextures(size_t* count) const
{
	*count = m_textures.size();
	return m_textures.data();
}

void LevelFile::setGravityAreas(vx::array<GravityAreaFile, DebugAllocatorResource<Allocator::MainAllocator>>* gravityAreas)
{
	gravityAreas->swap(m_gravityAreas);
}

const GravityAreaFile* LevelFile::getGravityAreas(size_t* count) const
{
	*count = m_gravityAreas.size();
	return m_gravityAreas.data();
}

void LevelFile::setGravityWells(vx::array<GravityWellFile, DebugAllocatorResource<Allocator::MainAllocator>>* gravityWells)
{
	gravityWells->swap(m_gravityWells);
}

const GravityWellFile* LevelFile::getGravityWells(size_t* count) const
{
	*count = m_gravityWells.size();
	return m_gravityWells.data();
}

u32 LevelFile::getDynamicOffset() const
{
	return (u32)(m_gravityWells.size() + m_gravityAreas.size() + m_staticEntities.size()) + 1;
}