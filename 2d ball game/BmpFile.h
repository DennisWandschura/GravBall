#pragma once

#include "Texture.h"

struct LoadFileHelper
{
	virtual ~LoadFileHelper() {}

	virtual vx::AllocatedBlock allocate(size_t size, size_t alignment) = 0;
	virtual vx::AllocatedBlock allocateTemporary(size_t size, size_t alignment) = 0;
	virtual void deallocateTemporary(const vx::AllocatedBlock &block) = 0;
};

class BmpFile
{
	friend Texture;

	vx::AllocatedBlock m_block;
	u32 m_pixelSize;
	vx::uint2 m_dimension;
	TextureFormat m_format;

public:
	BmpFile();
	BmpFile(const BmpFile&) = delete;
	~BmpFile();

	BmpFile& operator=(const BmpFile&) = delete;

	vx::AllocatedBlock release();

	bool loadFromFile(const char* file, LoadFileHelper* alloc);
};