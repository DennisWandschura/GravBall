#pragma once

class Texture;
class LevelFile;
class EventManager;

namespace Allocator
{
	class MainAllocator;
}

#include <gamelib/EventListener.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <gamelib/DebugAllocator.h>

enum class ResourceType
{
	Texture,
	Level
};

class ResourceAspect : public Event::ListenerInterface
{
	static const u32 MAX_TEXTURE_COUNT = 32u;
	static const u32 MAX_LEVEL_COUNT = 8u;

	vx::sorted_array<vx::StringID, Texture, DebugAllocatorResource<Allocator::MainAllocator>> m_textures;
	vx::sorted_array<vx::StringID, LevelFile, DebugAllocatorResource<Allocator::MainAllocator>> m_levels;
	EventManager* m_eventManager;
	Allocator::MainAllocator* m_allocator;
	char m_rootDir[8];

	void handleFileEvent(const Event::Event &evt);
	void handleEditorEvent(const Event::Event &evt);

public:
	ResourceAspect();
	~ResourceAspect();

	bool initialize(Allocator::MainAllocator* alloc, EventManager* evtManager, const char(&rootDir)[8]);
	void shutdown();

	bool loadFile(ResourceType type, const char* file);

	void addLevel(const vx::StringID &sid, LevelFile &&level);
	void addTexture(const vx::StringID &sid, Texture &&texture);

	const LevelFile* findLevel(const vx::StringID &sid) const;
	const Texture* findTexture(const vx::StringID &sid) const;

	void handleEvent(const Event::Event &evt) override;
};