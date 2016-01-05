#pragma once

namespace Allocator
{
	class MainAllocator;
}

namespace Graphics
{
	class GpuResourceManager;
}

#include "D3DObject.h"
#include "dynamic_bitset.h"
#include <vxLib/Container/array.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <gamelib/DebugAllocator.h>

class TextureArrayManager
{
	size_t m_freeTextures;
	dynamic_bitset m_bitset;
	vx::sorted_array<vx::StringID, size_t, DebugAllocatorRenderer<Allocator::MainAllocator>> m_sortedTextures;
	ID3D12Resource* m_textureSrgba;
	u32 m_textureDim;
	u32 m_textureArrayDim;

public:
	TextureArrayManager();
	~TextureArrayManager();

	bool initialize(ID3D12Device* device, u32 dim, u32 count, Allocator::MainAllocator* allocator, Graphics::GpuResourceManager* resourceManager);
	void shutdown(Allocator::MainAllocator* allocator);

	bool allocateTexture(const vx::StringID &sid, size_t* idx);

	void clear();

	size_t findTexture(const vx::StringID &sid) const;

	ID3D12Resource* getSrgbaTextureResource();

	D3D12_SHADER_RESOURCE_VIEW_DESC getSrvDesc() const;
};