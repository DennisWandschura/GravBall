#include "BmpFile.h"
#include <vxLib/Allocator/Mallocator.h>
#include <cstdio>
#include "Texture.h"
#include <utility>

struct LoadFileHelperImpl : public LoadFileHelper
{
	vx::Mallocator malloc;

	vx::AllocatedBlock allocate(size_t size, size_t alignment)
	{
		return malloc.allocate(size, alignment);
	}

	vx::AllocatedBlock allocateTemporary(size_t size, size_t alignment)
	{
		return malloc.allocate(size, alignment);
	}

	void deallocateTemporary(const vx::AllocatedBlock &block)
	{
		malloc.deallocate(block);
	}
};

bool test_load_bmp(const char* file, u8 testColor)
{
	printf("test_load_bmp\n");


	LoadFileHelperImpl alloc;

	BmpFile bmp;
	auto loaded = bmp.loadFromFile(file, &alloc);
	if (!loaded)
	{
		printf("\terror loading bmp\n");
		printf("test_load_bmp failure\n");
		return false;
	}

	Texture tex(std::move(bmp));

	auto pixels = tex.getPixels();
	auto dim = tex.getDimension();

	for (u32 y = 0; y < dim.y; ++y)
	{
		for (u32 x = 0; x < dim.x; ++x)
		{
			auto idx = x + y * dim.x;
			if (pixels[idx] != testColor)
			{
				printf("\tinvalid color\n");
				printf("test_load_bmp failure\n");
				return false;
			}
		}
	}

	printf("test_load_bmp success\n");

	return true;
}