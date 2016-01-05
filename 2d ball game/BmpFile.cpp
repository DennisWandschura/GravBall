#include "BmpFile.h"
#include <vxLib/File/File.h>
#include <Windows.h>
#include <vxLib/ScopeGuard.h>
#include <algorithm>

struct alignas(1) BmpHeader
{
	u8 header[2];
	u8 fileSize[4];
	u8 reserved[4];
	u8 offsetPixelArray[4];

	u32 getSize() const
	{
		return *(u32*)fileSize;
	}

	u32 getOffset() const
	{
		return *(u32*)offsetPixelArray;
	}

	bool isHeaderValid() const
	{
		if (header[0] != 'B' || header[1] != 'M')
			return false;

		return true;
	}
};

bool loadV4(vx::File &imgFile, u32 offsetToPixelData, u8* header, vx::AllocatedBlock* dataBlock, u32* pixelSizeOut, vx::uint2* dim, TextureFormat* color,
	LoadFileHelper* alloc)
{
	auto v4Header = (BITMAPV4HEADER*)header;

	auto isUncompressed = (v4Header->bV4V4Compression == BI_RGB || v4Header->bV4V4Compression == BI_BITFIELDS);
	if (!isUncompressed)
	{
		return false;
	}

	imgFile.seek(offsetToPixelData, vx::FileSeekPosition::Begin);

	auto bytesPerPixel = v4Header->bV4BitCount / 8;

	auto pixelSize = bytesPerPixel * v4Header->bV4Width * v4Header->bV4Height;

	if (bytesPerPixel == 3)
	{
		auto tmpBlock = alloc->allocateTemporary(pixelSize, 4);
		SCOPE_EXIT
		{
			alloc->deallocateTemporary(tmpBlock);
		};

		if (tmpBlock.ptr == nullptr)
			return false;

		imgFile.read(tmpBlock.ptr, pixelSize);

		auto block = alloc->allocate(pixelSize, 4);
		if (block.ptr == nullptr)
			return false;

		u32 pixelCount = v4Header->bV4Width * v4Header->bV4Height;
		u8* src = tmpBlock.ptr;
		u8* dst = block.ptr;
		for (u32 i = 0; i < pixelCount; i += 3)
		{
			u8 red = src[i + 2];
			u8 green = src[i + 1];
			u8 blue = src[i];

			dst[i] = red;
			dst[i + 1] = green;
			dst[i + 2] = blue;
		}

		*dataBlock = block;
		*pixelSizeOut = pixelSize;
		dim->x = v4Header->bV4Width;
		dim->y = v4Header->bV4Height;
		*color = TextureFormat::RGB;
		return true;
	}
	else
		return false;
}

bool loadV5(vx::File &imgFile, u32 offsetToPixelData, u8* header, vx::AllocatedBlock* dataBlock, u32* pixelSizeOut, vx::uint2* dim, TextureFormat* color,
	LoadFileHelper* alloc)
{
	BITMAPV5HEADER* v5Header = (BITMAPV5HEADER*)header;
	auto isUncompressed = (v5Header->bV5Compression == BI_RGB || v5Header->bV5Compression == BI_BITFIELDS);
	if (!isUncompressed)
	{
		return false;
	}

	imgFile.seek(offsetToPixelData, vx::FileSeekPosition::Begin);

	auto bytesPerPixel = v5Header->bV5BitCount / 8;
	auto pixelSize = bytesPerPixel * v5Header->bV5Width * v5Header->bV5Height;

	u32 redMask = v5Header->bV5RedMask;
	u32 greenMask = v5Header->bV5GreenMask;
	u32 blueMask = v5Header->bV5BlueMask;
	u32 alphaMask = v5Header->bV5AlphaMask;

	if (bytesPerPixel == 4)
	{
		// abgr

		auto tmpBlock = alloc->allocateTemporary(pixelSize, 4);
		SCOPE_EXIT
		{
			alloc->deallocateTemporary(tmpBlock);
		};
		if (tmpBlock.ptr == nullptr)
			return false;

		imgFile.read(tmpBlock.ptr, pixelSize);

		auto block = alloc->allocate(pixelSize, 4);

		u32 pixelCount = v5Header->bV5Width * v5Header->bV5Height;
		u32* src = (u32*)tmpBlock.ptr;
		u32* dst = (u32*)block.ptr;
		for (u32 i = 0; i < pixelCount; ++i)
		{
			u32 alpha = src[i] & alphaMask;
			u32 red = (src[i] & redMask) >> 24;
			u32 green = (src[i] & greenMask) >> 16;
			u32 blue = (src[i] & blueMask) >> 8;
			dst[i] = red | (green << 8) | (blue << 16) | (alpha << 24);
		}

		*dataBlock = block;
		*pixelSizeOut = pixelSize;
		dim->x = v5Header->bV5Width;
		dim->y = v5Header->bV5Height;

		if(alphaMask == 0)
			*color = TextureFormat::RGBX;
		else
			*color = TextureFormat::RGBA;

		return true;
	}
	else
		return false;
}

BmpFile::BmpFile()
	:m_block(),
	m_pixelSize(0),
	m_dimension(0, 0),
	m_format()
{

}

vx::AllocatedBlock BmpFile::release()
{
	auto blck = m_block;
	m_block.ptr = nullptr;
	m_block.size = 0;
	m_pixelSize = 0;

	return blck;
}

BmpFile::~BmpFile()
{
}

bool BmpFile::loadFromFile(const char* file, LoadFileHelper* alloc)
{
	vx::File imgFile;

	if (!imgFile.open(file, vx::FileAccess::Read))
		return false;

	BmpHeader header{};
	imgFile.read(header);

	if (!header.isHeaderValid())
		return false;

	u8 buffer[128];
	imgFile.read(buffer, sizeof(BITMAPCOREHEADER));

	BITMAPCOREHEADER* coreHeader = (BITMAPCOREHEADER*)buffer;

	auto offsetToPixelData = header.getOffset();
	if (coreHeader->bcSize == sizeof(BITMAPV5HEADER))
	{
		auto remainingSize = sizeof(BITMAPV5HEADER) - sizeof(BITMAPCOREHEADER);
		imgFile.read(buffer + sizeof(BITMAPCOREHEADER), remainingSize);

		return loadV5(imgFile, offsetToPixelData, buffer, &m_block, &m_pixelSize, &m_dimension, &m_format, alloc);
	}
	else if (coreHeader->bcSize == sizeof(BITMAPV4HEADER))
	{
		auto remainingSize = sizeof(BITMAPV4HEADER) - sizeof(BITMAPCOREHEADER);
		imgFile.read(buffer + sizeof(BITMAPCOREHEADER), remainingSize);

		return loadV4(imgFile, offsetToPixelData, buffer, &m_block, &m_pixelSize, &m_dimension, &m_format, alloc);
	}
	else
	{
		return false;
	}
}