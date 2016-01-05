#include "GpuResourceManager.h"
#include <gamelib/MainAllocator.h>
#include "ResourceDesc.h"
#include <vxLib/print.h>

namespace Graphics
{
	namespace GpuResourceManagerCpp
	{

		
	}

	struct GpuResourceManager::ResourceEntry
	{
		d3d::Resource resource;
		vx::GpuAllocatedBlock block;

		ResourceEntry() :resource(), block() {}
		ResourceEntry(const ResourceEntry&) = delete;
		ResourceEntry(ResourceEntry &&rhs) :resource(std::move(rhs.resource)), block(rhs.block) { rhs.block.offset = rhs.block.size = 0; }

		ResourceEntry& operator=(const ResourceEntry&) = delete;

		ResourceEntry& operator=(ResourceEntry &&rhs)
		{
			if (this != &rhs)
			{
				resource = std::move(rhs.resource);
				std::swap(block.offset, rhs.block.offset);
				std::swap(block.size, rhs.block.size);
			}
			return *this;
		}
	};

	GpuResourceManager::GpuResourceManager()
		:m_bufferResources(),
		m_textureResources(),
		m_rtdsResources(),
		m_bufferAllocator(),
		m_textureAllocator(),
		m_bufferHeap(),
		m_textureHeap(),
		m_rtDsHeap(),
		m_device(nullptr)
	{

	}

	GpuResourceManager::~GpuResourceManager()
	{

	}

	bool GpuResourceManager::initialize(const GpuResourceManagerDesc &desc)
	{
		D3D12_HEAP_DESC heapDesc{};
		heapDesc.Alignment = 64 KBYTE;
		heapDesc.Properties = { D3D12_HEAP_TYPE_DEFAULT, D3D12_CPU_PAGE_PROPERTY_UNKNOWN, D3D12_MEMORY_POOL_UNKNOWN, 0, 0 };

		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		heapDesc.SizeInBytes = desc.bufferHeapSize;
		if (!m_bufferHeap.create(heapDesc, desc.device))
			return false;

		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		heapDesc.SizeInBytes = desc.textureHeapSize;
		if (!m_textureHeap.create(heapDesc, desc.device))
			return false;

		heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		heapDesc.SizeInBytes = desc.rtdsHeapSize;
		if (!m_rtDsHeap.create(heapDesc, desc.device))
			return false;

		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> bufferResources(desc.allocator, desc.bufferResourceCount);
		m_bufferResources.swap(bufferResources);

		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> textureResources(desc.allocator, desc.textureResourceCount);
		m_textureResources.swap(textureResources);

		vx::sorted_array<vx::StringID, ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>> rtdsResources(desc.allocator, desc.rtdsResourceCount);
		m_rtdsResources.swap(rtdsResources);

		m_device = desc.device;

		m_bufferAllocator.initialize(desc.bufferHeapSize, DebugAllocatorRenderer<Allocator::MainAllocator>(desc.allocator));
		m_textureAllocator.initialize(desc.textureHeapSize, DebugAllocatorRenderer<Allocator::MainAllocator>(desc.allocator));

		return true;
	}

	void GpuResourceManager::shutdown()
	{
		m_rtdsResources.release();
		destroyAndReleaseResourceEntries(&m_textureResources, &m_textureAllocator);
		destroyAndReleaseResourceEntries(&m_bufferResources, &m_bufferAllocator);

		m_textureAllocator.release();
		m_bufferAllocator.release();

		m_rtDsHeap.release();
		m_textureHeap.release();
		m_bufferHeap.release();
	}

	template<typename GpuAllocator>
	void GpuResourceManager::destroyAndReleaseResourceEntries(vx::sorted_array<vx::StringID, GpuResourceManager::ResourceEntry, DebugAllocatorRenderer<Allocator::MainAllocator>>* entries, GpuAllocator* allocator)
	{
		auto begin = entries->begin();
		auto end = entries->end();

		while (begin != end)
		{
			begin->resource.release();
			allocator->deallocate(begin->block);
			++begin;
		}

		entries->release();
	}

	bool GpuResourceManager::createBufferResource(const char* id, u64 size, D3D12_RESOURCE_STATES state)
	{
		return createBufferResource(vx::make_sid(id), size, state);
	}

	bool GpuResourceManager::createBufferResource(const vx::StringID &sid, u64 size, D3D12_RESOURCE_STATES state)
	{
		if (m_bufferResources.size() >= m_bufferResources.capacity())
		{
			vx::printf("GpuResourceManager: No array entries left !");
			return false;
		}

		auto it = m_bufferResources.find(sid);
		if (it == m_bufferResources.end())
		{
			auto allocatedBlock = m_bufferAllocator.allocate(size);
			if (allocatedBlock.size == 0)
				return false;

			D3D12_RESOURCE_DESC bufferDesc{};
			bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			bufferDesc.Alignment = 64 KBYTE;
			bufferDesc.Width = allocatedBlock.size;
			bufferDesc.Height = 1;
			bufferDesc.DepthOrArraySize = 1;
			bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
			bufferDesc.SampleDesc.Count = 1;
			bufferDesc.MipLevels = 1;
			bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

			ResourceEntry entry;
			if (entry.resource.create(m_bufferHeap, allocatedBlock.offset, bufferDesc, state, m_device, nullptr))
			{
				entry.block = allocatedBlock;
				m_bufferResources.insert(sid, std::move(entry));
				return true;
			}
			else
			{
				m_bufferAllocator.deallocate(allocatedBlock);
			}
		}

		return false;
	}

	ID3D12Resource* GpuResourceManager::findBufferResource(const char* id)
	{
		return findBufferResource(vx::make_sid(id));
	}

	ID3D12Resource* GpuResourceManager::findBufferResource(const vx::StringID &sid)
	{
		auto it = m_bufferResources.find(sid);
		if (it == m_bufferResources.end())
			return nullptr;

		return it->resource.get();
	}

	bool GpuResourceManager::createTextureResource(const char* id, const vx::uint3 &dim, DXGI_FORMAT format, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue)
	{
		return createTextureResource(vx::make_sid(id), dim, format, state, clearValue);
	}

	bool GpuResourceManager::createTextureResource(const vx::StringID &sid, const vx::uint3 &dim, DXGI_FORMAT format, D3D12_RESOURCE_STATES state, const D3D12_CLEAR_VALUE* clearValue)
	{
		if (m_textureResources.size() >= m_textureResources.capacity())
		{
			vx::printf("GpuResourceManager: No array entries left !");
			return false;
		}

		auto it = m_textureResources.find(sid);
		if(it == m_textureResources.end())
		{
			d3d::ResourceDesc textureDesc = d3d::ResourceDesc::createDescTexture2D(dim, format);

			auto allocInfo = m_device->GetResourceAllocationInfo(0, 1, &textureDesc);

			auto allocatedBlock = m_textureAllocator.allocate(allocInfo.SizeInBytes);
			if (allocatedBlock.size == 0)
				return false;

			ResourceEntry entry;
			if (entry.resource.create(m_textureHeap, allocatedBlock.offset, textureDesc, state, m_device, clearValue))
			{
				entry.block = allocatedBlock;
				m_textureResources.insert(sid, std::move(entry));
				return true;
			}
			else
			{
				m_textureAllocator.deallocate(allocatedBlock);
			}
		}

		return false;
	}

	ID3D12Resource* GpuResourceManager::findTextureResource(const char* id)
	{
		return findTextureResource(vx::make_sid(id));
	}

	ID3D12Resource* GpuResourceManager::findTextureResource(const vx::StringID &sid)
	{
		auto it = m_textureResources.find(sid);
		if (it == m_textureResources.end())
			return nullptr;

		return it->resource.get();
	}
}