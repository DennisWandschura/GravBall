#include "ResourceAspect.h"
#include <vxLib/File/File.h>
#include "Texture.h"
#include <gamelib/MainAllocator.h>
#include "BmpFile.h"
#include <vxLib/File/FileHandle.h>
#include <gamelib/LevelFile.h>
#include <gamelib/Event.h>
#include "EventManager.h"

namespace ResourceAspectCpp
{
	struct LoadFileHelperImpl : public LoadFileHelper
	{
		Allocator::MainAllocator* m_allocator;

		explicit LoadFileHelperImpl(Allocator::MainAllocator* alloc) :m_allocator(alloc) {}

		vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			return m_allocator->allocate(size, alignment, Allocator::Channels::Resource, "bmp");
		}

		vx::AllocatedBlock allocateTemporary(size_t size, size_t alignment)
		{
			return m_allocator->allocate(size, alignment, Allocator::Channels::Resource, "bmp");
		}

		void deallocateTemporary(const vx::AllocatedBlock &block)
		{
			m_allocator->deallocate(block, Allocator::Channels::Resource, "bmp");
		}
	};

	bool loadTexture(const char* file, Allocator::MainAllocator* alloc, Texture* outTexture)
	{
		LoadFileHelperImpl helper(alloc);

		BmpFile bmp;
		if (!bmp.loadFromFile(file, &helper))
			return false;

		Texture texture(std::move(bmp));
		outTexture->swap(texture);

		return true;
	}
}

ResourceAspect::ResourceAspect()
	:m_textures(),
	m_levels(),
	m_allocator(nullptr),
	m_eventManager(nullptr),
	m_rootDir()
{

}

ResourceAspect::~ResourceAspect()
{

}

bool ResourceAspect::initialize(Allocator::MainAllocator* alloc, EventManager* evtManager, const char(&rootDir)[8])
{
	m_textures = vx::sorted_array<vx::StringID, Texture, DebugAllocatorResource<Allocator::MainAllocator>>(alloc, MAX_TEXTURE_COUNT);
	m_levels = vx::sorted_array<vx::StringID, LevelFile, DebugAllocatorResource<Allocator::MainAllocator>>(alloc, MAX_LEVEL_COUNT);

	m_allocator = alloc;
	memcpy(m_rootDir, rootDir, 8);
	m_eventManager = evtManager;

	return true;
}

void ResourceAspect::shutdown()
{
	for (auto &it : m_levels)
	{
		it.release();
	}
	m_levels.release();

	for (auto &it : m_textures)
	{
		auto block = it.release();
		m_allocator->deallocate(block, Allocator::Channels::Resource, "bmp");
	}
	m_textures.release();
}

bool ResourceAspect::loadFile(ResourceType type, const char* file)
{
	if (type == ResourceType::Texture)
	{
		vx::FileHandle handle(file);

		ResourceAspectCpp::LoadFileHelperImpl helper(m_allocator);

		BmpFile bmp;
		if (!bmp.loadFromFile(handle.m_string, &helper))
			return false;

		Texture texture(std::move(bmp));
		m_textures.insert(handle.m_sid,std::move(texture));

		return true;
	}
	else if (type == ResourceType::Level)
	{
		return false;
	}

	return false;
}

void ResourceAspect::addLevel(const vx::StringID &sid, LevelFile &&level)
{
	m_levels.insert(sid, std::move(level));
}

void ResourceAspect::addTexture(const vx::StringID &sid, Texture &&texture)
{
	m_textures.insert(sid, std::move(texture));
}

const LevelFile* ResourceAspect::findLevel(const vx::StringID &sid) const
{
	return m_levels.find(sid);
}

const Texture* ResourceAspect::findTexture(const vx::StringID &sid) const
{
	auto it = m_textures.find(sid);
	return (it == m_textures.end()) ? nullptr : it;
}

void ResourceAspect::handleFileEvent(const Event::Event &evt)
{
	auto type = evt.asFileEvent();
	if (type == Event::Type::FileEventTypes::LoadTexture)
	{
		const vx::FileHandle* filename = (const vx::FileHandle*)evt.getData().ptr;

		char file[256];
		snprintf(file, 256, "%sdata/textures/%s", m_rootDir, filename->m_string);

		Texture texture;
		if (ResourceAspectCpp::loadTexture(file, m_allocator, &texture))
		{
			vx::Variant variant;
			variant.u64 = filename->m_sid.value;

			m_textures.insert(filename->m_sid, std::move(texture));

#if _EDITOR
			m_eventManager->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadedTexture, variant, nullptr));
#else
			m_eventManager->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadedTexture, variant));
#endif
		}
	}
}

void ResourceAspect::handleEditorEvent(const Event::Event &evt)
{
}

void ResourceAspect::handleEvent(const Event::Event &evt)
{
	auto type = evt.getType();
	if (type == Event::Type::EventTypeFile)
	{
		handleFileEvent(evt);
	}
	else if (type == Event::Type::EventTypeEditor)
	{
		handleEditorEvent(evt);
	}
}