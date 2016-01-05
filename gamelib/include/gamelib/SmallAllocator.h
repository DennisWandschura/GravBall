#pragma once

#include "Allocator.h"

namespace Allocator
{
	class MediumAllocator;

	namespace detail
	{
		typedef vx::DelegateAllocator<MediumAllocator> DelegateMediumDataAllocator;

		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 8, 8, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 16, 16, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 32, 16, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 64, 64, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 128, 64, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 256, 64, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 512, 64, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 1024, 64, CascadingConstructionHelperSmall);
		CASCADING_BLOCK_ALLOCATOR(DelegateObjectHelperAllocator, DelegateMediumDataAllocator, 2048, 64, CascadingConstructionHelperSmall);

		class CascadingBlockAllocator8
		{
			CascadingBlockAllocator8Type alloc;

		public:
			CascadingBlockAllocator8() :alloc() {}
			~CascadingBlockAllocator8() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator16
		{
			CascadingBlockAllocator16Type alloc;

		public:
			CascadingBlockAllocator16() :alloc() {}
			~CascadingBlockAllocator16() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator32
		{
			CascadingBlockAllocator32Type alloc;

		public:
			CascadingBlockAllocator32() :alloc() {}
			~CascadingBlockAllocator32() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator64
		{
			CascadingBlockAllocator64Type alloc;

		public:
			CascadingBlockAllocator64() :alloc() {}
			~CascadingBlockAllocator64() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator128
		{
			CascadingBlockAllocator128Type alloc;

		public:
			CascadingBlockAllocator128() :alloc() {}
			~CascadingBlockAllocator128() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator256
		{
			CascadingBlockAllocator256Type alloc;

		public:
			CascadingBlockAllocator256() :alloc() {}
			~CascadingBlockAllocator256() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator512
		{
			CascadingBlockAllocator512Type alloc;

		public:
			CascadingBlockAllocator512() :alloc() {}
			~CascadingBlockAllocator512() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator1024
		{
			CascadingBlockAllocator1024Type alloc;

		public:
			CascadingBlockAllocator1024() :alloc() {}
			~CascadingBlockAllocator1024() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		class CascadingBlockAllocator2048
		{
			CascadingBlockAllocator2048Type alloc;

		public:
			CascadingBlockAllocator2048() :alloc() {}
			~CascadingBlockAllocator2048() {}

			template<typename Constructor>
			void initialize(Constructor constructor)
			{
				alloc.initialize(constructor);
			}

			void release()
			{
				alloc.release();
			}

			vx::AllocatedBlock allocate(size_t size, size_t alignment)
			{
				return alloc.allocate(size, alignment);
			}

			u32 deallocate(const vx::AllocatedBlock &block)
			{
				return alloc.deallocate(block);
			}
		};

		typedef Segregator<CascadingBlockAllocator2048,
			Segregator<CascadingBlockAllocator1024,
			Segregator<CascadingBlockAllocator512,
			Segregator<CascadingBlockAllocator256,
			Segregator<CascadingBlockAllocator128,
			Segregator<CascadingBlockAllocator64,
			Segregator<CascadingBlockAllocator32,
			Segregator<CascadingBlockAllocator16, CascadingBlockAllocator8, 8>, 16>, 32>, 64>, 128>, 256>, 512>, 1024> SmallAllocatorType;
	}

	class SmallAllocator
	{
		AllocationCountLayer<MemoryUsageLayer<detail::SmallAllocatorType>> m_alloc;

	public:
		SmallAllocator() :m_alloc() {}
		inline ~SmallAllocator() {}

		template<typename ...Args>
		inline void initialize(Args&& ...args)
		{
			m_alloc.initialize(std::forward<Args>(args)...);
		}

		inline void release()
		{
			m_alloc.release();
		}

		inline vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			return m_alloc.allocate(size, alignment);
		}

		u32 deallocate(const vx::AllocatedBlock &block)
		{
			return m_alloc.deallocate(block);
		}
	};
}