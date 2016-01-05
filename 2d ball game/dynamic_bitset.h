#pragma once

#include <vxLib/Allocator/Allocator.h>
#include <algorithm>

class dynamic_bitset
{
	union
	{
		u64 m_bits;
		u8* m_ptr;
	};
	size_t m_bitCount;
	size_t m_allocatedSize;

	u8* getPtr()
	{
		return (m_bitCount <= 64) ? (u8*)&m_bits : m_ptr;
	}

public:
	dynamic_bitset() :m_bits(0), m_bitCount(0), m_allocatedSize(0) {}
	explicit dynamic_bitset(size_t bitCount) :m_bits(0), m_bitCount(bitCount), m_allocatedSize(0) { VX_ASSERT(bitCount <= 64); }

	template<typename Allocator>
	dynamic_bitset(size_t bitCount, Allocator* allocator) : m_ptr(nullptr), m_bitCount(0), m_allocatedSize(0)
	{
		auto size = (bitCount + 7) / 8;

		auto block = allocator->allocate(size, 4);
		if (block.ptr != nullptr)
		{
			memset(block.ptr, 0, sizeof(u8) * block.size);

			m_ptr = block.ptr;
			m_bitCount = bitCount;
			m_allocatedSize = block.size;
		}
	}

	~dynamic_bitset()
	{
	}

	void swap(dynamic_bitset &other)
	{
		std::swap(m_ptr, other.m_ptr);
		std::swap(m_bitCount, other.m_bitCount);
		std::swap(m_allocatedSize, other.m_allocatedSize);
	}

	vx::AllocatedBlock release()
	{
		if (m_bitCount <= 64)
			return{ nullptr, 0 };

		vx::AllocatedBlock block = { m_ptr, m_allocatedSize };
		m_ptr = nullptr;
		m_allocatedSize = 0;
		m_bitCount = 0;

		return block;
	}

	size_t getBitCount() const { return m_bitCount; }

	bool allocatedMemory() const { return m_bitCount > 64; }

	bool findEmptyBit(size_t* resultBit)
	{
		return findEmptyBit(getPtr(), resultBit);
	}

	bool findEmptyBit(u8* bitPtr, size_t* resultBit)
	{
		auto byte = 0;

		size_t block = 0;
		auto remainingBlocks = m_bitCount;
		while (remainingBlocks > 0)
		{
			auto p = bitPtr[byte];
			auto bitsToCheck = (remainingBlocks < 8) ? remainingBlocks : 8;

			for (size_t bit = 0; bit < bitsToCheck; ++bit)
			{
				auto mask = 1 << bit;
				auto tmp = (p & mask);
				if (tmp == 0)
				{
					*resultBit = block;
					return true;
				}

				++block;
			}

			remainingBlocks -= bitsToCheck;
		}

		return false;
	}

	void setBit(size_t idx)
	{
		auto bitPtr = getPtr();
		setBit(bitPtr, idx);
	}

	void setBit(u8* bitPtr, size_t idx)
	{
		auto byte = idx >> 3;
		auto bit = idx & 7;

		bitPtr[byte] |= (1 << bit);
	}

	void clearBit(size_t idx)
	{
		auto byte = idx >> 3;
		auto bit = idx & 7;

		auto bitPtr = getPtr();
		bitPtr[byte] &= ~(1 << bit);
	}

	void clear()
	{
		if (m_bitCount <= 64)
		{
			m_bits = 0;
		}
		else
		{
			memset(m_ptr, 0, sizeof(u8) * m_allocatedSize);
		}
	}
};