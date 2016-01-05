#pragma once

namespace Allocator
{
	class MainAllocatorHelper;
}

#include <vxLib/Allocator/Allocator.h>
#include <vxLib/Allocator/LinearAllocator.h>
#include <gamelib/DebugLayer.h>
#include <vxLib/Allocator/BitmapBlock.h>
#include <gamelib/Segregator.h>
#include <vxLib/Allocator/MultiBlockAllocator.h>
#include <gamelib/CascadingAllocator.h>
#include <vxLib/Allocator/DelegateAllocator.h>

namespace Allocator
{
	enum HeapSize : size_t
	{
		HeapSizeSmall = 64 KBYTE,
		HeapSizeMedium = 4 MBYTE,
		HeapSizeLarge = 64 MBYTE,
		TotalHeapSize = 128 MBYTE
	};

	enum MaxObjectSizes : size_t
	{
		MaxSmallObjSize = 2 KBYTE,
		MaxMediumObjSize = 1 MBYTE,
		MaxLargeObjSize = 32 MBYTE
	};

#define CASCADING_BLOCK_ALLOCATOR(PARENT, DATA_ALLOC, SIZE, ALIGN, CONSTRUCTION_HELPER) typedef AllocationCountLayer<vx::CascadingAllocator<PARENT, DATA_ALLOC, vx::BitmapBlock<vx::LinearAllocator, SIZE, ALIGN>, CONSTRUCTION_HELPER>> CascadingBlockAllocator##SIZE##Type

	typedef vx::MultiBlockAllocator<512, 64, 32> MultiBlockAllocator512;
	typedef vx::MultiBlockAllocator<4 MBYTE, 64, TotalHeapSize / (4 MBYTE)> MultiBlockAllocator4MB;

	class LargeAllocator
	{
		MemoryUsageLayer<AllocationCountLayer<MultiBlockAllocator4MB>> m_alloc;

	public:
		LargeAllocator() :m_alloc() {}
		~LargeAllocator() {}

		void initialize(const vx::AllocatedBlock &block)
		{
			m_alloc.initialize(block);
		}

		vx::AllocatedBlock release() { return m_alloc.release(); }

		vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			return m_alloc.allocate(size, alignment);
		}

		u32 deallocate(const vx::AllocatedBlock &block)
		{
			return m_alloc.deallocate(block);
		}
	};

	template<size_t HEAP_SIZE>
	struct CascadingConstructionHelper
	{
		template<typename Parent, typename Allocator>
		static void onConstruct(Parent &parent, Allocator* allocator)
		{
			auto block = parent.allocate(HEAP_SIZE, 64);
			if (block.ptr != nullptr)
			{
				allocator->initialize(block);
			}
			else
			{
				printf("out of memory\n");
			}
		}

		template<typename Parent, typename Allocator>
		static void onDestruct(Parent &parent, Allocator* allocator)
		{
			parent.deallocate(allocator->release());
		}
	};

	namespace detail
	{
		typedef CascadingConstructionHelper<HeapSizeSmall> CascadingConstructionHelperSmall;
		typedef CascadingConstructionHelper<HeapSizeMedium> CascadingConstructionHelperMedium;
		typedef CascadingConstructionHelper<HeapSizeLarge> CascadingConstructionHelperLarge;

		typedef vx::DelegateAllocator<MultiBlockAllocator512> DelegateObjectHelperAllocator;
		typedef vx::DelegateAllocator<LargeAllocator> DelegateLargeDataAllocator;

		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 8 KBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator8KB;
		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 16 KBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator16KB;
		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 64 KBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator64KB;
		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 256 KBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator256KB;
		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 512 KBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator512KB;
		typedef AllocationCountLayer<vx::CascadingAllocator<DelegateObjectHelperAllocator, DelegateLargeDataAllocator, vx::BitmapBlock<vx::LinearAllocator, 1 MBYTE, 64>, CascadingConstructionHelperMedium>> CascadingBlockAllocator1MB;
	}
}