#pragma once

#include <vxLib/Allocator/Allocator.h>
#include <algorithm>

namespace vx
{
	struct DefaultConstructionHelper
	{
		template<typename P, typename T>
		static void onConstruct(P&, T*)
		{

		}

		template<typename P, typename T>
		static void onDestruct(P&, T*)
		{

		}
	};

	template<typename ArrayAllocator, typename DataAllocator, typename AllocatorType, typename ConstructionHelper = DefaultConstructionHelper>
	class CascadingAllocator
	{
		AllocatorType* m_lastAllocation;
		AllocatorType* m_lastDealloc;
		AllocatorType* m_list;
		size_t m_size;
		size_t m_capacity;
		ArrayAllocator m_arrayAllocator;
		DataAllocator m_dataAllocator;
		size_t m_listBlockSize;

		AllocatorType* pushAllocator()
		{
			auto capacity = m_capacity;
			auto currentSize = m_size;

			bool allocatorFits = true;
			if (currentSize >= capacity)
			{
				allocatorFits = false;

				//resize
				auto newCapacity = capacity + 2;
				auto block = m_arrayAllocator.allocate(sizeof(AllocatorType) * newCapacity, __alignof(AllocatorType));

				if (block.ptr != nullptr)
				{
					auto oldList = m_list;
					auto newList = reinterpret_cast<AllocatorType*>(block.ptr);

					for (size_t i = 0; i < currentSize; ++i)
					{
						new (&newList[i]) AllocatorType{ std::move(oldList[i]) };
					}

					vx::AllocatedBlock oldListBlock{ (u8*)oldList, m_listBlockSize };
					m_arrayAllocator.deallocate(oldListBlock);

					m_list = newList;
					m_capacity = newCapacity;
					m_listBlockSize = block.size;
					allocatorFits = true;
				}
				// else out of memory

			}

			AllocatorType* ptr = nullptr;
			if (allocatorFits)
			{
				ptr = &m_list[currentSize++];
				new (ptr) AllocatorType{};
				ConstructionHelper::onConstruct(m_dataAllocator, ptr);
				m_size = currentSize;
			}

			return ptr;
		}

		AllocatorType* findAllocator(const vx::AllocatedBlock &block) const
		{
			auto list = m_list;
			auto size = m_size;

			AllocatorType* dealloc = nullptr;
			for (size_t i = 0; i < size; ++i)
			{
				if (list[i].contains(block))
				{
					dealloc = &list[i];
					break;
				}
			}

			return dealloc;
		}

	public:
		CascadingAllocator() : m_lastAllocation(nullptr), m_lastDealloc(nullptr), m_list(nullptr), m_size(0), m_capacity(0), m_arrayAllocator(), m_dataAllocator(), m_listBlockSize(0) {}

		CascadingAllocator(ArrayAllocator &&parent, DataAllocator &&dataAllocator)
			: m_lastAllocation(nullptr),
			m_lastDealloc(nullptr),
			m_list(nullptr),
			m_size(0),
			m_capacity(0),
			m_arrayAllocator(std::move(parent)),
			m_dataAllocator(std::move(dataAllocator)),
			m_listBlockSize(0)
		{
			m_lastDealloc = m_lastAllocation = pushAllocator();
		}

		CascadingAllocator(CascadingAllocator &&rhs)
			: m_lastAllocation(rhs.m_lastAllocation),
			m_lastDealloc(rhs.m_lastDealloc),
			m_list(rhs.m_list),
			m_size(rhs.m_size),
			m_capacity(rhs.m_capacity),
			m_arrayAllocator(std::move(rhs.m_arrayAllocator)),
			m_dataAllocator(std::move(rhs.m_dataAllocator)),
			m_listBlockSize(rhs.m_listBlockSize)
		{
			rhs.m_list = nullptr;
			rhs.m_size = 0;
			rhs.m_capacity = 0;
		}

		CascadingAllocator(const CascadingAllocator&) = delete;

		~CascadingAllocator()
		{
			release();
		}

		CascadingAllocator& operator=(const CascadingAllocator&) = delete;

		void initialize(ArrayAllocator &&parent, DataAllocator &&dataAllocator)
		{
			m_arrayAllocator = std::move(parent);
			m_dataAllocator = std::move(dataAllocator);
			m_lastDealloc = m_lastAllocation = pushAllocator();
		}

		template<typename Constructor>
		void initialize(Constructor constructor)
		{
			constructor(m_arrayAllocator, m_dataAllocator);
			m_lastDealloc = m_lastAllocation = pushAllocator();
		}

		vx::AllocatedBlock allocate(size_t size, size_t alignment)
		{
			vx::AllocatedBlock block = m_lastAllocation->allocate(size, alignment);
			if (block.ptr == nullptr)
			{
				auto list = m_list;
				auto listSize = m_size;

				for (size_t i = 0; i < listSize; ++i)
				{
					auto b = list[i].allocate(size, alignment);
					if (b.ptr != nullptr)
					{
						m_lastAllocation = &list[i];
						block = b;
						break;
					}
				}
			}

			if (block.ptr == nullptr)
			{
				auto alloc = pushAllocator();
				if (alloc)
				{
					block = alloc->allocate(size, alignment);
					m_lastAllocation = alloc;
				}
			}

			return block;
		}

		u32 deallocate(const vx::AllocatedBlock &block)
		{
			if (block.ptr == nullptr)
				return 1;

			if (m_lastDealloc->contains(block))
			{
				return m_lastDealloc->deallocate(block);
			}

			auto dealloc = findAllocator(block);

			if (dealloc)
			{
				dealloc->deallocate(block);
				m_lastDealloc = dealloc;
				return 1;
			}

			return 0;
		}

		bool contains(const vx::AllocatedBlock &block) const
		{
			return (findAllocator(block) != nullptr);
		}

		void release()
		{
			if (m_size != 0)
			{
				auto size = m_size;
				for (size_t i = 0; i < size; ++i)
				{
					ConstructionHelper::onDestruct(m_dataAllocator, &m_list[i]);
					m_list[i].~AllocatorType();
				}
				m_size = 0;

				m_arrayAllocator.deallocate(vx::AllocatedBlock{ (u8*)m_list, m_listBlockSize });
			}
		}

		void print() const
		{
			AllocatorType::printStatic();
		}
	};
}