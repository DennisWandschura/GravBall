#pragma once

namespace vx
{
	class Mallocator;
}

#include "Allocator.h"
#include <vxLib/CpuTimer.h>
#include "DebugLayer.h"
#include <gamelib/AllocatorDebugInfo.h>
#include <vxLib/Container/array.h>
#include <vxLib/File/File.h>

namespace Allocator
{
	class MainAllocator;

#if _EDITOR
	class DebugLayer
	{
	public:
		DebugLayer() {}
		~DebugLayer() {}

		void initialize(vx::Mallocator* allocator) {}
		void shutdown(vx::Mallocator* allocator) {}

		void frame() {}

		void pushAllocationEntry(const vx::AllocatedBlock &block, size_t requestedSize, Allocator::Channels channel, const char* msg) {}
		void pushDeallocationEntry(const vx::AllocatedBlock &block, Allocator::Channels channel, const char* msg) {}
	};
#else
	class DebugLayer
	{
		u64 m_index;
		u64 m_frame;
		u64 m_entryCount;
		vx::array<DebugInfo, vx::DelegateAllocator<MemoryUsageLayer<vx::LinearAllocator>>> m_entries;
		vx::CpuTimer m_timer;
		MemoryUsageLayer<vx::LinearAllocator> m_allocator;
		vx::File m_outputFile;

		void writeToFile();

	public:
		DebugLayer();
		~DebugLayer();

		void initialize(vx::Mallocator* allocator);
		void shutdown(vx::Mallocator* allocator);

		void frame();

		void pushAllocationEntry(const vx::AllocatedBlock &block, size_t requestedSize, Allocator::Channels channel, const char* msg);
		void pushDeallocationEntry(const vx::AllocatedBlock &block, Allocator::Channels channel, const char* msg);
	};
#endif
}