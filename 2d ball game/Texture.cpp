#include "Texture.h"
#include <algorithm>
#include "BmpFile.h"

Texture::Texture()
	:m_block(),
	m_pixelSize(0),
	m_dimension(0, 0),
	m_format(TextureFormat::Unkown)
{

}

Texture::Texture(Texture &&rhs)
	:m_block(rhs.m_block),
	m_pixelSize(rhs.m_pixelSize),
	m_dimension(rhs.m_dimension),
	m_format(rhs.m_format)
{
	rhs.m_block = {nullptr, 0};
	rhs.m_pixelSize = 0;
}

Texture::Texture(BmpFile &&rhs)
	:m_block(rhs.m_block),
	m_pixelSize(rhs.m_pixelSize),
	m_dimension(rhs.m_dimension),
	m_format(rhs.m_format)
{
	rhs.m_block = { nullptr, 0 };
	rhs.m_pixelSize = 0;
}

Texture::~Texture()
{

}

Texture& Texture::operator=(Texture &&rhs)
{
	if (this != &rhs)
	{
		swap(rhs);
	}

	return *this;
}

void Texture::swap(Texture &other)
{
	std::swap(m_block, other.m_block);
	std::swap(m_pixelSize, other.m_pixelSize);
	std::swap(m_dimension, other.m_dimension);
	std::swap(m_format, other.m_format);
}

vx::AllocatedBlock Texture::release()
{
	auto blck = m_block;

	m_block = { nullptr, 0 };
	m_pixelSize = 0;

	return blck;
}

u32 Texture::getRowPitch() const
{
	auto pixelSize = 0u;
	switch (m_format)
	{
	case TextureFormat::RGB:
		pixelSize = 3;
		break;
	case TextureFormat::RGBA:
		pixelSize = 4;
		break;
	case TextureFormat::RGBX:
		pixelSize = 4;
		break;
	default:
		break;
	}

	return m_dimension.x * pixelSize;
}