#include "test_setClearBits.h"
#include <intrin.h>

void clearBitsTable(u32* bitsPtr, size_t blockIndex, size_t blockCount)
{
	static const u32 table[] =
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

		auto mask = table[count] << bit;
		bitsPtr[byte] &= ~mask;

		remainingBlocks -= count;
		blockIdx += count;
	}
}

void clearBits(u32* bitsPtr, size_t blockIndex, size_t blockCount)
{
	for (size_t i = 0; i < blockCount; ++i)
	{
		auto blockIdx = blockIndex + i;

		auto byte = blockIdx / 32;
		auto bit = blockIdx & 31;

		bitsPtr[byte] &= ~(1 << bit);
	}
}

void setBitsTable(u32* bitsPtr, size_t blockIndex, size_t blockCount)
{
	static const u32 table[] =
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

		auto mask = table[count] << bit;
		bitsPtr[byte] |= mask;

		remainingBlocks -= count;
		blockIdx += count;
	}
}

void setBits(u32* bitsPtr, size_t blockIndex, size_t blockCount)
{
	for (size_t i = 0; i < blockCount; ++i)
	{
		auto blockIdx = blockIndex + i;

		auto byte = blockIdx / 32;
		auto bit = blockIdx & 31;

		bitsPtr[byte] |= (1 << bit);
	}
}

int ntz(u32 x)
{
	static char table[64] =
	{
		32, 0, 1, 12, 2, 6, 0, 13, 3, 0, 7, 0, 0, 0, 0, 14,
		10, 4, 0, 0, 8, 0, 0, 25, 0, 0, 0, 0, 0, 21, 27, 15,
		31, 11, 5, 0, 0, 0, 0, 0, 9, 0, 0, 24, 0, 0, 20, 26,
		30, 0, 0, 0, 0, 23, 0, 19, 29, 0, 22, 18, 28, 17, 16, 0
	};

	x = (x & -int(x)) * 0x0450fbaf;
	return table[x >> 26];
}

int nlz(u32 x)
{
	u32 y;
	int n = 32;

	y = x >> 16; if (y != 0) { n = n - 16; x = y; }
	y = x >> 8; if (y != 0) { n = n - 8; x = y; }
	y = x >> 4; if (y != 0) { n = n - 4; x = y; }
	y = x >> 2; if (y != 0) { n = n - 2; x = y; }
	y = x >> 1; if (y != 0) { n = n - 1; x = y; }
	return n - x;
}

int nlz_table(u32 x)
{
	static char table[64]=
	{
		32, 31, 0, 16, 0, 30, 3, 0, 15, 0, 0, 0, 29, 10, 2, 0,
		0, 0, 12, 14, 21, 0, 19, 0, 0, 28, 0, 25, 0, 9, 1, 0,
		17, 0, 4, 0, 0, 0, 11, 0, 13, 22, 20, 0, 26, 0, 0, 18,
		5, 0, 0, 23, 0, 27, 0, 6, 0, 24, 7, 0, 8, 0, 0, 0
	};

	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	x = x * 0x06eb14f9;
	return table[x >> 26];
}

int ffstrl(u32 x, int n)
{
	int s;

	while (n > 1)
	{
		s = n >> 1;
		x = x & (x << s);
		n = n - s;
	}

	return nlz(x);
}

int ffstrl_table(u32 x, int n)
{
	int s;

	while (n > 1)
	{
		s = n >> 1;
		x = x & (x << s);
		n = n - s;
	}

	return nlz_table(x);
}

u32 rev(u32 x)
{
	x = (x & 0x55555555) << 1 | (x >> 1) & 0x55555555;
	x = (x & 0x33333333) << 2 | (x >> 2) & 0x33333333;
	x = (x & 0x0f0f0f0f) << 4 | (x >> 4) & 0x0f0f0f0f;
	x = (x << 24) | ((x & 0xff00) << 8) |
		((x >> 8) & 0xff00) | (x >> 24);
	return x;
}

bool findEmptyBit(u32* bitPtr, size_t* resultBit, size_t maxBlockCount)
{
	size_t block = 0;
	auto remainingBlocks = maxBlockCount;
	while (remainingBlocks > 0)
	{
		auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;

		auto p = *bitPtr++;
		unsigned long bit = 0;
		auto result = _BitScanForward(&bit, p);
		if (result != 0)
		{
			*resultBit = block + bit;
			return (bit < bitsToCheck);
		}

		block += 32;
		remainingBlocks -= bitsToCheck;
	}

	return false;
}

bool findEmptyBit_ntz(u32* bitPtr, size_t* resultBit, size_t maxBlockCount)
{
	size_t block = 0;
	auto remainingBlocks = maxBlockCount;
	while (remainingBlocks > 0)
	{
		auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;

		auto p = *bitPtr++;

		auto index = ntz(p);
		if (index != 32)
		{
			*resultBit = block + index;
			return index < bitsToCheck;
		}

		remainingBlocks -= bitsToCheck;
		block += 32;
	}

	return false;
}

bool findEmptyBits(u32* bitsPtr, size_t* resultBit, size_t blockCount, size_t maxBlockCount)
{
	size_t freeConiguousBlocks = 0;

	size_t resultBlock = 0;
	size_t block = 0;

	size_t blockTmp = 0;
	auto remainingBlocks = maxBlockCount;
	while (remainingBlocks >= blockCount)
	{
		auto p = *bitsPtr++;;
		auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;

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

		blockTmp += 32;

		remainingBlocks -= bitsToCheck;
	}

	return false;
}

bool findEmptyBits_ntz(u32* bitsPtr, size_t* resultBit, size_t blockCount, size_t maxBlockCount)
{
	size_t freeConiguousBlocks = 0;

	size_t resultBlock = 0;
	size_t block = 0;

	auto blocksNeeded = blockCount;
	auto remainingBlocks = maxBlockCount;
	while (remainingBlocks >= blockCount)
	{
		auto p = *bitsPtr++;;
		auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;

		int offset = (freeConiguousBlocks == 0) ? ntz(p) : 0;
		auto blocks = bitsToCheck - offset;
		blocks = (blocksNeeded < blocks) ? blocksNeeded : blocks;

		auto index = ffstrl(rev(p >> offset), blocks);
		if (index != 32)
		{
			if (freeConiguousBlocks == 0)
			{
				resultBlock = block + offset;
			}

			freeConiguousBlocks += blocks;
			blocksNeeded -= blocks;
		}
		else
		{
			freeConiguousBlocks = 0;
			blocksNeeded = blockCount;
		}

		if (freeConiguousBlocks >= blockCount)
		{
			*resultBit = resultBlock;
			return true;
		}

		block += 32;

		remainingBlocks -= bitsToCheck;
	}

	return false;
}

bool findEmptyBits_ntz_table(u32* bitsPtr, size_t* resultBit, size_t blockCount, size_t maxBlockCount)
{
	size_t freeConiguousBlocks = 0;

	size_t resultBlock = 0;
	size_t block = 0;

	auto blocksNeeded = blockCount;
	auto remainingBlocks = maxBlockCount;
	while (remainingBlocks >= blockCount)
	{
		auto p = *bitsPtr++;;
		auto bitsToCheck = (remainingBlocks < 32) ? remainingBlocks : 32;

		int offset = (freeConiguousBlocks == 0) ? ntz(p) : 0;
		auto blocks = bitsToCheck - offset;
		blocks = (blocksNeeded < blocks) ? blocksNeeded : blocks;

		auto index = ffstrl_table(rev(p >> offset), blocks);
		if (index != 32)
		{
			if (freeConiguousBlocks == 0)
			{
				resultBlock = block + offset;
			}

			freeConiguousBlocks += blocks;
			blocksNeeded -= blocks;
		}
		else
		{
			freeConiguousBlocks = 0;
			blocksNeeded = blockCount;
		}

		if (freeConiguousBlocks >= blockCount)
		{
			*resultBit = resultBlock;
			return true;
		}

		block += 32;

		remainingBlocks -= bitsToCheck;
	}

	return false;
}

bool simple()
{
	u32 value0 = 1 | 1 << 9;
	u32 value1 = value0;

	u32 blockCount = 6;
	u32 blockIndex = 1;

	setBitsTable(&value0, blockIndex, blockCount);
	setBitsTable(&value1, blockIndex, blockCount);

	if (value0 != value1)
		return false;

	clearBitsTable(&value0, blockIndex, blockCount);
	clearBits(&value1, blockIndex, blockCount);

	if (value0 != value1)
		return false;

	return true;
}

bool test_array()
{
	u32 value0[2];
	u32 value1[2];

	u32 blockIndex = 29;
	u32 blockCount = 4;
	value1[0] = value0[0] = 1 << 29 | 1 << 30 | 1 << 31;
	value1[1] = value0[1] = 1 << 0 | 1 << 1 | 1 << 2;

	setBitsTable(value0, blockIndex, blockCount);
	setBitsTable(value1, blockIndex, blockCount);

	if (value0[0] != value1[0] ||
		value0[1] != value1[1])
		return false;

	clearBitsTable(value0, blockIndex, blockCount);
	clearBits(value1, blockIndex, blockCount);

	if (value0[0] != value1[0] ||
		value0[1] != value1[1])
		return false;

	return true;
}

#include <Windows.h>
#include <cstdio>

bool test_setClearBits()
{
	//if (!simple())
	//	return false;

	//if (!test_array())
	//	return false;

	const auto count = 1024 * 2;
	u32* bits = new u32[count];
	memset(bits, 0xff, sizeof(u32)*count);

	bits[1500] = ~(1 << 0 | 1 << 1 | 1 << 2);
	bits[1899] = ~(1 << 27 | 1 << 28 | 1 << 29 | 1 << 30 | 1 << 31);
	bits[1900] = ~(1 << 0 | 1 << 1 | 1 << 2);

	const size_t maxBlockCount = count * 32;
	const size_t blockCount = 3;

	LARGE_INTEGER t0, t1;

	QueryPerformanceCounter(&t0);
	size_t result0 = 0;
	auto b0 = findEmptyBits(bits, &result0, blockCount, maxBlockCount);
	QueryPerformanceCounter(&t1);

	printf("%lld\n", t1.QuadPart - t0.QuadPart);

	memset(bits, 0, sizeof(u32)*count);

	bits[1500] = (1 << 0 | 1 << 1 | 1 << 2);
	bits[1899] = (1 << 27 | 1 << 28 | 1 << 29 | 1 << 30 | 1 << 31);
	bits[1900] = (1 << 0 | 1 << 1 | 1 << 2);

	QueryPerformanceCounter(&t0);
	size_t result1 = 0;
	auto b1 = findEmptyBits_ntz(bits, &result1, blockCount, maxBlockCount);
	QueryPerformanceCounter(&t1);

	printf("%lld\n", t1.QuadPart - t0.QuadPart);

	printf("%d\n", result0 == result1);

	delete(bits);

	u32 value = (1 << 0 | 1 << 1 | 1 << 2);
	auto i0=nlz_table(value);
	auto i1 = nlz(value);

	return true;
}