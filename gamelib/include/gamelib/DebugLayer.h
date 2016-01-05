#pragma once

#include <vxLib/Allocator/Allocator.h>

template<typename Super>
class AllocationCountLayer : public Super
{
	u32 m_allocationCount;
	u32 m_deallocationCount;

public:
	AllocationCountLayer() :Super(), m_allocationCount(0), m_deallocationCount(0) {}
	explicit AllocationCountLayer(const vx::AllocatedBlock &block) :Super(block), m_allocationCount(0), m_deallocationCount(0){}

	vx::AllocatedBlock allocate(size_t size, size_t alignment)
	{
		auto block = Super::allocate(size, alignment);
		if (block.ptr != nullptr)
		{
			++m_allocationCount;
		}

		return block;
	}

	u32 deallocate(const vx::AllocatedBlock &block)
	{
		auto result = Super::deallocate(block);

		if (block.ptr != nullptr && result != 0)
		{
			++m_deallocationCount;
		}

		return result;
	}

	inline bool contains(const vx::AllocatedBlock &block) const
	{
		return Super::contains(block);
	}

	inline void print() const
	{
		Super::print();
		printf("alloc count: %u, dealloc count: %u\n", m_allocationCount, m_deallocationCount);
	}
};

template<typename Super>
class MemoryUsageLayer : public Super
{
	size_t m_usedMemory;
	size_t m_maxAllocSize;
	size_t m_minAllocSize;
	size_t m_maxUsedMemory;

public:
	MemoryUsageLayer() :Super(),m_usedMemory(0), m_maxAllocSize(0), m_minAllocSize(size_t(-1)), m_maxUsedMemory(0){}
	explicit MemoryUsageLayer(const vx::AllocatedBlock &block) :Super(block), m_usedMemory(0), m_maxAllocSize(0), m_minAllocSize(-1), m_maxUsedMemory(0){}

	void initialize(const vx::AllocatedBlock &block)
	{
		Super::initialize(block);

		m_usedMemory = 0;
		m_maxUsedMemory = 0;
	}

	template<typename ...Args>
	void initialize(Args&& ...args)
	{
		Super::initialize(std::forward<Args>(args)...);

		m_usedMemory = 0;
		m_maxUsedMemory = 0;
	}

	vx::AllocatedBlock allocate(size_t size, size_t alignment)
	{
		auto block = Super::allocate(size, alignment);
		if (block.ptr != nullptr)
		{
			m_usedMemory += block.size;
			m_maxAllocSize = (m_maxAllocSize < block.size) ? block.size : m_maxAllocSize;
			m_minAllocSize = (block.size < m_minAllocSize) ? block.size : m_minAllocSize;

			if (m_maxUsedMemory < m_usedMemory)
			{
				m_maxUsedMemory = m_usedMemory;
			}
		}

		return block;
	}

	u32 deallocate(const vx::AllocatedBlock &block)
	{
		auto result = Super::deallocate(block);

		if (result != 0)
		{
			m_usedMemory -= block.size;
		}

		return result;
	}

	inline bool contains(const vx::AllocatedBlock &block) const
	{
		return Super::contains(block);
	}

	inline void print() const
	{
		Super::print();
		printf("max alloc size: %llu\n", m_maxAllocSize);
	}
};