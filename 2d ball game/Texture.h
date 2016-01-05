#pragma once

class BmpFile;

#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/Allocator.h>

enum class TextureFormat : u32
{
	Unkown,
	RGB,
	RGBA,
	RGBX
};

class Texture
{
	vx::AllocatedBlock m_block;
	u32 m_pixelSize;
	vx::uint2 m_dimension;
	TextureFormat m_format;

public:
	Texture();
	Texture(const Texture&) = delete;
	Texture(Texture &&rhs);
	Texture(BmpFile &&rhs);
	~Texture();

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture &&);

	vx::AllocatedBlock release();

	void swap(Texture &other);

	const u8* getPixels() const { return m_block.ptr; }

	u32 getPixelSize() const { return m_pixelSize; }

	const vx::uint2 getDimension() const { return m_dimension; }

	u32 getRowPitch() const;
};