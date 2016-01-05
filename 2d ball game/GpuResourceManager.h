#pragma once

namespace Allocator
{
	class MainAllocator;
}

#include "D3DObject.h"
#include <vxLib/StringID.h>
#include <vxLib/Container/sorted_array.h>
#include "GpuMultiBlockAllocator.h"
#include <vxLib/math/Vector.h>
#include <gamelib/DebugAllocator.h>

namespace Graphics
{
	struct GpuResourceManagerDesc
	{
		u64 bufferHeapSize; 
		u64 textureHeapSize; 
		u64 rtdsHeapSize; 
		u32 bufferResourceCount;
		u32 textureResourceCount;
		u32 rtdsResourceCount;
		ID3D12Device* device;
		Allocator::MainAllocator* allocator;
	};

	class GpuResourceManager
	{
		struct ResourceEntry;

		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> m_bufferResources;
		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> m_textureResources;
		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> m_rtdsResources;
		vx::GpuMultiBlockAllocator<64 KBYTE, 64 KBYTE, 32, DebugAllocatorRenderer<Allocator::MainAllocator>> m_bufferAllocator;
		vx::GpuMultiBlockAllocator<64 KBYTE, 64 KBYTE, 1024, DebugAllocatorRenderer<Allocator::MainAllocator>> m_textureAllocator;
		d3d::Heap m_bufferHeap;
		d3d::Heap m_textureHeap;
		d3d::Heap m_rtDsHeap;
		ID3D12Device* m_device;

		template<typename GpuAllocator>
		void destroyAndReleaseResourceEntries(vx::sorted_array<vx::StringID, GpuResourceManager::ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>>* entries, GpuAllocator* allocator);

	public:
		GpuResourceManager();
		~GpuResourceManager();

		bool initialize(const GpuResourceManagerDesc &desc);
		void shutdown();

		bool createBufferResource(const char* id, u64 size, D3D12_RESOURCE_STATES state);
		bool createBufferResource(const vx::StringID &sid, u64 size, D3D12_RESOURCE_STATES state);

		ID3D12Resource* findBufferResource(const char* id);
		ID3D12Resource* findBufferResource(const vx::StringID &sid);

		bool createTextureResource(const char* id, const vx::uint3 &dim, DXGI_FORMAT format, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue);
		bool createTextureResource(const vx::StringID &sid, const vx::uint3 &dim, DXGI_FORMAT format, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue);

		ID3D12Resource* findTextureResource(const char* id);
		ID3D12Resource* findTextureResource(const vx::StringID &sid);
	};
}