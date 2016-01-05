#include <gamelib/MainAllocator.h>
#include <vxLib/Allocator/Mallocator.h>
#include <utility>
#include <gamelib/AllocatorDebugLayer.h>

namespace Allocator
{
	struct MediumConstructor
	{
		detail::DelegateObjectHelperAllocator m_arrayAlloc;
		detail::DelegateLargeDataAllocator m_dataAlloc;

		MediumConstructor(detail::DelegateObjectHelperAllocator &&arrayAlloc, detail::DelegateLargeDataAllocator &&dataAlloc)
			:m_arrayAlloc(std::move(arrayAlloc)), m_dataAlloc(std::move(dataAlloc))
		{}

		void operator()(detail::DelegateObjectHelperAllocator& arrayAlloc, detail::DelegateLargeDataAllocator& dataAlloc)
		{
			arrayAlloc = m_arrayAlloc;
			dataAlloc = m_dataAlloc;
		}
	};

	struct SmallConstructor
	{
		detail::DelegateObjectHelperAllocator m_arrayAlloc;
		detail::DelegateMediumDataAllocator m_dataAlloc;

		SmallConstructor(detail::DelegateObjectHelperAllocator &&arrayAlloc, detail::DelegateMediumDataAllocator &&dataAlloc)
			:m_arrayAlloc(std::move(arrayAlloc)), m_dataAlloc(std::move(dataAlloc))
		{}

		void operator()(detail::DelegateObjectHelperAllocator& arrayAlloc, detail::DelegateMediumDataAllocator& dataAlloc)
		{
			arrayAlloc = m_arrayAlloc;
			dataAlloc = m_dataAlloc;
		}
	};

	MainAllocator::MainAllocator()
		:m_debugLayer(nullptr),
		m_smallAllocator(),
		m_mediumAllocator(),
		m_largeAllocator(),
		m_allocatorAllocator()
	{

	}

	MainAllocator::~MainAllocator()
	{

	}

	void MainAllocator::initialize(vx::Mallocator* mallocator, Allocator::DebugLayer* debugLayer)
	{
		m_debugLayer = debugLayer;
		m_allocatorAllocator.initialize(mallocator->allocate(2 MBYTE, 64));
		m_largeAllocator.initialize(mallocator->allocate(128 MBYTE, 64));

		MediumConstructor mediumConstructor(std::move(detail::DelegateObjectHelperAllocator(&m_allocatorAllocator)), std::move(detail::DelegateLargeDataAllocator(&m_largeAllocator)));
		m_mediumAllocator.initialize(mediumConstructor);

		SmallConstructor smallConstructor(std::move(detail::DelegateObjectHelperAllocator(&m_allocatorAllocator)), std::move(detail::DelegateMediumDataAllocator(&m_mediumAllocator)));
		m_smallAllocator.initialize(smallConstructor);
	}

	void MainAllocator::shutdown(vx::Mallocator* mallocator)
	{
		m_smallAllocator.release();
		m_mediumAllocator.release();
		mallocator->deallocate(m_largeAllocator.release());
		mallocator->deallocate(m_allocatorAllocator.release());
	}

	vx::AllocatedBlock MainAllocator::allocate(size_t size, size_t alignment, Channels channel, const char* msg)
	{
		auto alignedSize = vx::getAlignedSize(size, alignment);

		vx::AllocatedBlock result{ nullptr, 0 };
		if (alignedSize <= MaxSmallObjSize)
		{
			result = m_smallAllocator.allocate(alignedSize, alignment);
		}
		else if (alignedSize <= MaxMediumObjSize)
		{
			result = m_mediumAllocator.allocate(alignedSize, alignment);
		}
		else
		{
			result = m_largeAllocator.allocate(alignedSize, alignment);
		}

		if (result.ptr != nullptr)
		{
			m_debugLayer->pushAllocationEntry(result, size, channel, msg);
		}

		return result;
	}

	u32 MainAllocator::deallocate(const vx::AllocatedBlock &block, Allocator::Channels channel, const char* msg)
	{
		u32 result = 0;
		if (block.size <= MaxSmallObjSize)
		{
			result = m_smallAllocator.deallocate(block);
		}
		else if (block.size <= MaxMediumObjSize)
		{
			result = m_mediumAllocator.deallocate(block);
		}
		else
		{
			result = m_largeAllocator.deallocate(block);
		}

		if (result != 0)
		{
			m_debugLayer->pushDeallocationEntry(block, channel, msg);
		}

		return result;
	}
}