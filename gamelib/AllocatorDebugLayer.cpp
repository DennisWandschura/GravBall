#include <gamelib/AllocatorDebugLayer.h>
#if !_EDITOR
#include <cstring>
#include <vxlib/Allocator/Mallocator.h>

namespace Allocator
{
	namespace DebugLayerCpp
	{
		template<typename List>
		void writeEntriesToFile(List &entries, vx::File &f)
		{
			while (!entries.empty())
			{
				auto node = entries.begin();
				f.write(node->m_data);
				entries.pop_front();
			}
		}

		template<typename List>
		void pushEntry(const vx::AllocatedBlock &block, size_t requestedSize, Allocator::Channels channel, Allocator::DebugInfoType type, const char* msg, List &entries, vx::CpuTimer &timer, u64 index, u64 frame)
		{
			auto time = timer.getTimeSeconds();

			const auto maxBufferSize = DebugInfo::BufferSize - 1;
			auto msgSize = strlen(msg);
			msgSize = (msgSize <= maxBufferSize) ? msgSize : maxBufferSize;

			DebugInfo info;
			info.index = index;
			info.channel = channel;
			info.type = type;
			info.frame = frame;
			memcpy(info.msg, msg, msgSize);
			info.msg[msgSize] = '\0';
			info.ptr = (size_t)block.ptr;
			info.allocatedSize = block.size;
			info.requestedSize = requestedSize;
			info.time = time;

			auto b = entries.push_back(info);
			VX_ASSERT(b != 0);
		}
	}

	DebugLayer::DebugLayer()
		:m_index(0),
		m_frame(0),
		m_entryCount(0),
		m_entries(),
		m_timer(),
		m_allocator(),
		m_outputFile()
	{

	}

	DebugLayer::~DebugLayer()
	{

	}

	void DebugLayer::initialize(vx::Mallocator* allocator)
	{
		const auto arrayCapacity = 512u;
		auto block = allocator->allocate(sizeof(DebugInfo) * arrayCapacity, __alignof(DebugInfo));

		m_timer.reset();
		m_allocator.initialize(block);

		m_entries = vx::array<DebugInfo, vx::DelegateAllocator<MemoryUsageLayer<vx::LinearAllocator>>>(&m_allocator, arrayCapacity);

		m_outputFile.create("allocation.info", vx::FileAccess::Write);

		DebugInfoFileHeader dummyHeader{};
		m_outputFile.write(dummyHeader);
	}

	void DebugLayer::shutdown(vx::Mallocator* allocator)
	{
		writeToFile();

		DebugInfoFileHeader header{};
		header.entryCount = m_entryCount;
		header.endTime = m_timer.getTimeSeconds();

		m_outputFile.seek(0, vx::FileSeekPosition::Begin);
		m_outputFile.write(header);

		m_outputFile.close();

		m_entries.clear();

		allocator->deallocate(m_allocator.release());
	}

	void DebugLayer::writeToFile()
	{
		auto count = m_entries.size();
		m_entryCount += count;

		m_outputFile.write((u8*)m_entries.data(), count * sizeof(DebugInfo));

		m_entries.clear();
	}

	void DebugLayer::frame()
	{
		++m_frame;

		if (m_entries.size() >= 384)
		{
			writeToFile();
		}
	}

	void DebugLayer::pushAllocationEntry(const vx::AllocatedBlock &block, size_t requestedSize, Allocator::Channels channel, const char* msg)
	{
		DebugLayerCpp::pushEntry(block, requestedSize, channel, Allocator::DebugInfoType::Allocation, msg, m_entries, m_timer, m_index++, m_frame);
	}

	void DebugLayer::pushDeallocationEntry(const vx::AllocatedBlock &block, Allocator::Channels channel, const char* msg)
	{
		DebugLayerCpp::pushEntry(block, 0, channel, Allocator::DebugInfoType::Deallocation, msg, m_entries, m_timer, m_index++, m_frame);
	}
}
#endif