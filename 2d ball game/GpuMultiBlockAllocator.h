#pragma once

/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vxLib/Allocator/StackAllocator.h>

namespace vx
{
	struct GpuAllocatedBlock
	{
		size_t offset;
		size_t size;
	};
	/*
	BLOCK_SIZE: size of allocation, needs to be aligned by ALIGNMENT
	ALIGNMENT: alignment of blocks
	MAX_BLOCK_COUNT: maximum amount of blocks used by an allocation
	*/
	template<size_t BLOCK_SIZE, size_t ALIGNMENT, size_t MAX_BLOCK_COUNT, typename ParentAllocator>
	class GpuMultiBlockAllocator
	{
		static_assert(GetAlignedSize<BLOCK_SIZE, ALIGNMENT>::size == BLOCK_SIZE, "");

		enum :size_t { BitsSizeT = sizeof(size_t) * 8 };

		union
		{
			u32* m_bitsPtr;
			size_t m_bits;
		};
		size_t m_remainingBlocks;
		size_t m_blockCount;
		size_t m_allocatedBitSize;
		ParentAllocator m_parent;

		u32* getBitPtr()
		{
			return (m_blockCount <= BitsSizeT) ? (u32*)&m_bits : m_bitsPtr;
		}

		size_t countBlocks(u64 size)
		{
			return size / BLOCK_SIZE;
		}

		bool findEmptyBit(u32* bitsPtr, size_t* resultBit, size_t blockCount)
		{
			auto byte = 0;

			size_t freeConiguousBlocks = 0;

			size_t resultBlock = 0;
			size_t block = 0;
			auto remainingBlocks = m_blockCount;
			while (remainingBlocks >= blockCount)
			{
				auto p = bitsPtr[byte];
				auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;
				//if (bitsToCheck < blockCount)
				//	break;

				for (size_t bit = 0; bit < bitsToCheck; ++bit)
				{
					auto mask = 1 << bit;
					auto tmp = (p & mask);
					if (tmp == 0)
					{
						++freeConiguousBlocks;

						if (freeConiguousBlocks >= blockCount)
						{
							*resultBit = resultBlock;
							return true;
						}
					}
					else
					{
						resultBlock = block + 1;
						freeConiguousBlocks = 0;
					}

					++block;
				}

				remainingBlocks -= bitsToCheck;
				++byte;
			}

			return false;
		}

		void setBits(u32* bitsPtr, size_t blockIndex, size_t blockCount)
		{
			static const u32 TABLE[] =
			{
				0, 1, 3, 7, 15,
				31, 63, 127, 255,
				511, 1023,2047, 4095,
				8191, 16383, 32767, 65535,
				131071, 262143, 524287, 1048575,
				2097151, 4194303, 8388607, 16777215,
				33554431, 67108863, 134217727, 268435455,
				536870911, 1073741823, 2147483647, 4294967295
			};

			auto remainingBlocks = blockCount;
			auto blockIdx = blockIndex;
			while (remainingBlocks != 0)
			{
				auto byte = blockIdx >> 5;
				auto bit = blockIdx & 31;
				auto lastBit = (remainingBlocks + bit);
				lastBit = (lastBit < 32) ? lastBit : 32;
				auto count = lastBit - bit;

				auto mask = TABLE[count] << bit;
				bitsPtr[byte] |= mask;

				remainingBlocks -= count;
				blockIdx += count;
			}
		}

		void clearBit(size_t blockIndex, size_t blockCount)
		{
			static const u32 TABLE[] =
			{
				0, 1, 3, 7, 15,
				31, 63, 127, 255,
				511, 1023,2047, 4095,
				8191, 16383, 32767, 65535,
				131071, 262143, 524287, 1048575,
				2097151, 4194303, 8388607, 16777215,
				33554431, 67108863, 134217727, 268435455,
				536870911, 1073741823, 2147483647, 4294967295
			};

			auto bitsPtr = getBitPtr();

			auto remainingBlocks = blockCount;
			auto blockIdx = blockIndex;
			while (remainingBlocks != 0)
			{
				auto byte = blockIdx >> 5;
				auto bit = blockIdx & 31;
				auto lastBit = (remainingBlocks + bit);
				lastBit = (lastBit < 32) ? lastBit : 32;
				auto count = lastBit - bit;

				auto mask = TABLE[count] << bit;
				bitsPtr[byte] &= ~mask;

				remainingBlocks -= count;
				blockIdx += count;
			}
		}

	public:
		GpuMultiBlockAllocator() : m_bitsPtr(nullptr), m_remainingBlocks(0), m_blockCount(0), m_allocatedBitSize(0), m_parent() { }

		GpuMultiBlockAllocator(u64 size, ParentAllocator &&parent)
			: m_bitsPtr(nullptr), m_remainingBlocks(0), m_blockCount(0), m_allocatedBitSize(0), m_parent(std::move(parent)) {
			initialize(size);
		}

		~GpuMultiBlockAllocator() {}

		void initialize(u64 size, ParentAllocator &&parent)
		{
			auto blockCount = countBlocks(size);
			if (blockCount == 0)
				return;

			if (blockCount <= BitsSizeT)
			{
				m_bits = 0;
			}
			else
			{
				auto requiredBytes = (blockCount + 7) / 8;

				auto bitsBlock = parent.allocate(vx::getAlignedSize(requiredBytes, __alignof(u32)), __alignof(u32));

				m_bitsPtr = (u32*)bitsBlock.ptr;
				memset(m_bitsPtr, 0, bitsBlock.size);

				m_allocatedBitSize = bitsBlock.size;

				m_parent = std::move(parent);
			}

			m_remainingBlocks = blockCount;
			m_blockCount = blockCount;
		}

		void release()
		{
			if (m_bitsPtr)
			{
				m_remainingBlocks = 0;
				m_blockCount = 0;
				m_parent.deallocate({ (u8*)m_bitsPtr, m_allocatedBitSize });
				m_bitsPtr = nullptr;
				m_allocatedBitSize = 0;
			}
		}

		vx::GpuAllocatedBlock allocate(size_t size)
		{
			auto alignedSize = getAlignedSize(size, ALIGNMENT);

			auto blockCount = (alignedSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
			if (blockCount > m_remainingBlocks || blockCount > MAX_BLOCK_COUNT)
				return{ 0, 0 };

			auto bitsPtr = getBitPtr();
			size_t resultBlock = 0;
			if (!findEmptyBit(bitsPtr, &resultBlock, blockCount))
				return{ 0, 0 };

			setBits(bitsPtr, resultBlock, blockCount);
			m_remainingBlocks -= blockCount;

			auto offset = BLOCK_SIZE * resultBlock;

			return{ offset, alignedSize };
		}

		void deallocate(const vx::GpuAllocatedBlock &block)
		{
			if (block.size == 0)
				return;

			auto alignedOffset = vx::getAlignedSize(block.offset, ALIGNMENT);
			if (alignedOffset != block.offset)
				return;

			auto blockIndex = block.offset / BLOCK_SIZE;
			VX_ASSERT(blockIndex < m_blockCount);

			auto blockCount = (block.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
			clearBit(blockIndex, blockCount);

			m_remainingBlocks += blockCount;
		}

		void deallocateAll()
		{
		}

		bool contains(const AllocatedBlock &block) const
		{
			auto last = m_firstBlock + BLOCK_SIZE * m_blockCount;
			return (block.ptr >= m_firstBlock) && (block.ptr < last);
		}

		void print() const
		{
			printStatic();
		}

		static void printStatic()
		{
			printf("blocksize: %llu, alignment: %llu\n", BLOCK_SIZE, ALIGNMENT);
		}
	};
}