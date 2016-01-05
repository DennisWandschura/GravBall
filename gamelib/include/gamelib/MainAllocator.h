#pragma once

namespace vx
{
	class Mallocator;
}

namespace Allocator
{
	class DebugLayer;
}

#include <gamelib/Allocator.h>
#include <vxLib/Allocator/MultiBlockAllocator.h>
#include <gamelib/AllocatorDebugInfo.h>
#include <gamelib/SmallAllocator.h>
#include <gamelib/MediumAllocator.h>

namespace Allocator
{
	class MainAllocator
	{
		DebugLayer* m_debugLayer;
		SmallAllocator m_smallAllocator;
		MediumAllocator m_mediumAllocator;
		LargeAllocator m_largeAllocator;
		MultiBlockAllocator512 m_allocatorAllocator;

	public:
		MainAllocator();
		~MainAllocator();

		void initialize(vx::Mallocator* mallocator, DebugLayer* debugLayer);
		void shutdown(vx::Mallocator* mallocator);

		vx::AllocatedBlock allocate(size_t size, size_t alignment, Channels channel, const char* msg);
		u32 deallocate(const vx::AllocatedBlock &block, Channels channel, const char* msg);
	};
}