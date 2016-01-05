#include "TextureArrayManager.h"
#include <gamelib/MainAllocator.h>
#include "GpuResourceManager.h"

TextureArrayManager::TextureArrayManager()
	: m_freeTextures(0),
	m_bitset(),
	m_sortedTextures(),
	m_textureSrgba(nullptr),
	m_textureDim(0),
	m_textureArrayDim(0)
{

}

TextureArrayManager::~TextureArrayManager()
{

}

bool TextureArrayManager::initialize(ID3D12Device* device, u32 dim, u32 count, Allocator::MainAllocator* allocator, Graphics::GpuResourceManager* resourceManager)
{
	if (!resourceManager->createTextureResource("srgbaTexture", vx::uint3(dim, dim, count), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr))
		return false;

	auto srgbaTexture = resourceManager->findTextureResource("srgbaTexture");
	if (srgbaTexture == nullptr)
		return false;

	if (count <= 64)
	{
		dynamic_bitset set{ count };
		m_bitset.swap(set);
	}
	else
	{
		auto tmpAlloc = DebugAllocatorRenderer<Allocator::MainAllocator>(allocator);
		dynamic_bitset set{ count, &tmpAlloc };
		m_bitset.swap(set);
	}

	m_textureSrgba = srgbaTexture;
	m_freeTextures = count;

	m_textureDim = dim;
	m_textureArrayDim = count;

	m_sortedTextures = vx::sorted_array<vx::StringID, size_t, DebugAllocatorRenderer<Allocator::MainAllocator>>(allocator, count);

	return true;
}

void TextureArrayManager::shutdown(Allocator::MainAllocator* allocator)
{
	m_sortedTextures.release();

	if (m_bitset.allocatedMemory())
	{
		auto tmpAlloc = DebugAllocatorRenderer<Allocator::MainAllocator>(allocator);
		tmpAlloc.deallocate(m_bitset.release());
	}
}

bool TextureArrayManager::allocateTexture(const vx::StringID &sid, size_t* idx)
{
	if (m_freeTextures == 0)
		return false;

	size_t bitIdx = 0;
	bool result = m_bitset.findEmptyBit(&bitIdx);
	if (result)
	{
		m_bitset.setBit(bitIdx);
		*idx = bitIdx;
		--m_freeTextures;

		m_sortedTextures.insert(sid, bitIdx);
	}

	return result;
}

/*void TextureManager::update(ID3D12CommandList** lists, u32* count)
{
	if (m_uploadTasks.empty())
		return;

	m_cmdAllocator->Reset();
	m_cmdList->Reset(m_cmdAllocator, nullptr);

	size_t taskCount = m_uploadTasks.size();
	taskCount = (taskCount <= MAX_TEXTURE_COUNT_UPLOAD) ? taskCount : MAX_TEXTURE_COUNT_UPLOAD;
	u64 offset = 0;

	u8* ptr = nullptr;
	m_uploadBuffer->Map(0, nullptr, (void**)&ptr);
	for (size_t i = 0, k = taskCount - 1; i < taskCount; ++i, --k)
	{
		auto task = m_uploadTasks[k];
		m_uploadTasks.pop_back();

		memcpy(ptr, task.data, TextureSize_512_SRGBA);

		D3D12_TEXTURE_COPY_LOCATION src;
		src.pResource = m_uploadBuffer;
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint.Offset = offset;
		src.PlacedFootprint.Footprint = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 512, 512, 1, 4 * 512};

		D3D12_TEXTURE_COPY_LOCATION dst;
		dst.pResource = m_textureSrgba;
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = (u32)task.idx;

		m_cmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

		offset += TextureSize_512_SRGBA;
		ptr += TextureSize_512_SRGBA;
	}
	D3D12_RANGE writtenRange;
	writtenRange.Begin = 0;
	writtenRange.End = offset;
	m_uploadBuffer->Unmap(0, &writtenRange);

	m_cmdList->Close();

	lists[(*count)++] = m_cmdList;
}*/

void TextureArrayManager::clear()
{
	m_bitset.clear();
}

size_t TextureArrayManager::findTexture(const vx::StringID &sid) const
{
	auto result = m_sortedTextures.find(sid);

	return (result == m_sortedTextures.end()) ? (size_t)-1 : *result;
}

ID3D12Resource* TextureArrayManager::getSrgbaTextureResource()
{
	return m_textureSrgba;
}

D3D12_SHADER_RESOURCE_VIEW_DESC TextureArrayManager::getSrvDesc() const
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = m_textureArrayDim;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.PlaneSlice = 0;
	srvDesc.Texture2DArray.ResourceMinLODClamp = 0;

	return srvDesc;
}