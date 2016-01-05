#pragma once

#include "Allocator.h"

namespace Allocator
{
	namespace detail
	{
		typedef Segregator<detail::CascadingBlockAllocator1MB,
			Segregator<detail::CascadingBlockAllocator512KB,
			Segregator<detail::CascadingBlockAllocator256KB,
			Segregator<detail::CascadingBlockAllocator64KB,
			Segregator<detail::CascadingBlockAllocator16KB, detail::CascadingBlockAllocator8KB, 8 KBYTE>, 16 KBYTE>, 64 KBYTE>, 256 KBYTE>, 512 KBYTE> MediumObjectAllocator;
	}

	class MediumAllocator
	{
		MemoryUsageLayer<detail::MediumObjectAllocator> m_allocator;

	public:
		MediumAllocator() {}
		~MediumAllocator() {}

		template<typename Constructor>
		void initialize(Constructor constructor)
		{
			m_allocator.initialize(constructor);
		}

		void release()
		{
			m_allocator.release();
		}

		vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			return m_allocator.allocate(size, alignment);
		}

		u32 deallocate(const vx::AllocatedBlock &block)
		{
			return m_allocator.deallocate(block);
		}
	};

	typedef vx::DelegateAllocator<MediumAllocator> DelegateMediumDataAllocator;
}