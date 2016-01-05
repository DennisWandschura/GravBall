#include "MeshManager.h"
#include <vxLib/Allocator/Allocator.h>
#include "Vertex.h"
#include <gamelib/MainAllocator.h>
#include "MeshBuilder.h"
#include "UploadManager.h"

#include <gamelib/EntityFile.h>

MeshManager::MeshManager()
	:m_vbv(),
	m_vertexBuffer(),
	m_indexBuffer(),
	m_staticEntityCount(0),
	m_staticEntityCapacity(0),
	m_dynamicEntityCount(0),
	m_dynamicEntityCapacity(0),
	m_entityCapacity(0),
	m_vertexCapacity(0),
	m_indexCapacity(0)
{

}

MeshManager::~MeshManager()
{

}

bool MeshManager::initialize(ID3D12Device* device, u32 vertexCount, u32 indexCount, u32 dynamicEntityCapacity, u32 staticEntityCapacity, Allocator::MainAllocator* allocator)
{
	auto vboSize = vx::getAlignedSize(vertexCount * sizeof(Vertex), 64 KBYTE);
	auto iboSize = vx::getAlignedSize(indexCount * sizeof(u32), 64 KBYTE);

	D3D12_HEAP_DESC heapDesc{};
	heapDesc.SizeInBytes = vboSize + iboSize;
	heapDesc.Alignment = 64 KBYTE;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	if (!m_heap.create(heapDesc, device))
		return false;

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 64 KBYTE;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	desc.Width = vboSize;
	if (!m_vertexBuffer.create(m_heap, 0, desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, device))
		return false;
	m_vertexBuffer->SetName(L"vbo");

	desc.Width = iboSize;
	if (!m_indexBuffer.create(m_heap, vboSize, desc, D3D12_RESOURCE_STATE_INDEX_BUFFER, device))
		return false;
	m_indexBuffer->SetName(L"ibo");

	m_vertexCapacity = vertexCount;
	m_vertexCount = 0;
	m_indexCapacity = indexCount;
	m_indexCount = 0;

	m_vbv.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vbv.StrideInBytes = sizeof(Vertex);

	m_ibv.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_ibv.Format = DXGI_FORMAT_R32_UINT;

	m_entityCapacity = dynamicEntityCapacity + staticEntityCapacity;
	m_staticEntityCapacity = staticEntityCapacity;
	m_dynamicEntityCapacity = dynamicEntityCapacity;

	return true;
}

void MeshManager::shutdown()
{
	m_indexBuffer.release();
	m_vertexBuffer.release();
}

void MeshManager::clear()
{
	m_vertexCount = 0;
	m_indexCount = 0;
	m_staticEntityCount = 0;
	m_dynamicEntityCount = 0;
}

template<typename T>
bool MeshManager::buildMeshes(u32 offset, const T* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager,
	ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	MeshBuilderOutput meshOutput{};
	if (!MeshBuilder::build(offset, m_vertexCount, m_indexCount, entities, count, allocator, textureManager, &meshOutput))
		return false;

	uploadMeshData(meshOutput.vertexBlock.ptr, meshOutput.vertexCount, meshOutput.indexBlock.ptr, meshOutput.indexCount, uploadManager);

	auto transformOffset = offset * sizeof(vx::float4);

	auto b = uploadManager->pushUploadBuffer(meshOutput.transformBlock.ptr, meshOutput.transformSizeBytes, 0, frameData0TransformBlock, transformOffset, D3D12_RESOURCE_STATE_GENERIC_READ);
	VX_ASSERT(b);
	b = uploadManager->pushUploadBuffer(meshOutput.transformBlock.ptr, meshOutput.transformSizeBytes, 0, frameData1TransformBlock, transformOffset, D3D12_RESOURCE_STATE_GENERIC_READ);
	VX_ASSERT(b);

	allocator->deallocate(meshOutput.transformBlock, Allocator::Channels::Renderer, "mesh transform data");
	allocator->deallocate(meshOutput.vertexBlock, Allocator::Channels::Renderer, "mesh vertex data");
	allocator->deallocate(meshOutput.indexBlock, Allocator::Channels::Renderer, "mesh index data");

	return true;
}

template<typename T>
bool MeshManager::addMeshesImpl(u32 offset, const T* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, 
	ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	auto newCapacity = offset + count;
	if (m_entityCapacity >= newCapacity)
	{
		return buildMeshes(offset, entities, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager);
	}

	return false;
}

bool MeshManager::addMeshes(const StaticEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	if (addMeshesImpl(m_staticEntityCount, entities, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager))
	{
		m_staticEntityCount += (u32)count;
		return true;
	}

	return false;
}

bool MeshManager::addMeshes(const GravityAreaFile* areas, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	if(addMeshesImpl(m_staticEntityCount, areas, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager))
	{
		m_staticEntityCount += (u32)count;
		return true;
	}

	return false;
}

bool MeshManager::addMeshes(const DynamicEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	auto offset = getDynamicEntityOffset();
	if (addMeshesImpl(offset, entities, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager))
	{
		m_dynamicEntityCount += count;
		return true;
	}

	return false;
}

bool MeshManager::addMeshes(const DynamicEntity* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	auto offset = getDynamicEntityOffset();
	if (addMeshesImpl(offset, entities, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager))
	{
		m_dynamicEntityCount += count;
		return true;
	}

	return false;
}

bool MeshManager::addMeshes(const GravityWellFile* wells, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager)
{
	if(addMeshesImpl(m_staticEntityCount, wells, count, allocator, uploadManager, frameData0TransformBlock, frameData1TransformBlock, textureManager))
	{
		m_staticEntityCount += (u32)count;
		return true;
	}

	return false;
}

template<typename T>
void MeshManager::updateMeshImpl(const T &entity, const MeshManagerUpdateMeshDesc &desc)
{
	MeshBuilderOutput meshOutput{};
	MeshBuilder::build(desc.transformOffset, desc.vertexOffset, desc.indexOffset, &entity, 1, desc.allocator, desc.textureManager, &meshOutput);

	auto vboOffset = sizeof(Vertex) * desc.vertexOffset;
	auto iboOffset = sizeof(u32) * desc.indexOffset;

	auto b = desc.uploadManager->pushUploadBuffer(meshOutput.vertexBlock.ptr, meshOutput.vertexSizeBytes, 0, m_vertexBuffer, vboOffset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	VX_ASSERT(b);
	b = desc.uploadManager->pushUploadBuffer(meshOutput.indexBlock.ptr, meshOutput.indexSizeBytes, 0, m_indexBuffer, iboOffset, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	VX_ASSERT(b);

	auto transformOffsetBytes = desc.transformOffset * sizeof(vx::float4);
	b = desc.uploadManager->pushUploadBuffer(meshOutput.transformBlock.ptr, meshOutput.transformSizeBytes, 0, desc.frameData0TransformBlock, transformOffsetBytes, D3D12_RESOURCE_STATE_GENERIC_READ);
	VX_ASSERT(b);
	b = desc.uploadManager->pushUploadBuffer(meshOutput.transformBlock.ptr, meshOutput.transformSizeBytes, 0, desc.frameData1TransformBlock, transformOffsetBytes, D3D12_RESOURCE_STATE_GENERIC_READ);
	VX_ASSERT(b);

	desc.allocator->deallocate(meshOutput.transformBlock, Allocator::Channels::Renderer, "mesh transform data");
	desc.allocator->deallocate(meshOutput.vertexBlock, Allocator::Channels::Renderer, "mesh vertex data");
	desc.allocator->deallocate(meshOutput.indexBlock, Allocator::Channels::Renderer, "mesh index data");
}

void MeshManager::updateMesh(const StaticEntityFile &entity, const MeshManagerUpdateMeshDesc &desc)
{
	updateMeshImpl(entity, desc);
}

void MeshManager::updateMesh(const DynamicEntityFile &entity, const MeshManagerUpdateMeshDesc &desc)
{
	updateMeshImpl(entity, desc);
}

void MeshManager::updateMesh(const GravityAreaFile &entity, const MeshManagerUpdateMeshDesc &desc)
{
	updateMeshImpl(entity, desc);
}

void MeshManager::updateMesh(const GravityWellFile &entity, const MeshManagerUpdateMeshDesc &desc)
{
	updateMeshImpl(entity, desc);
}

void MeshManager::uploadMeshData(const u8* vertices, u32 vertexCount, const u8* indices, u32 indexCount, UploadManager* uploadManager)
{
	VX_ASSERT(vertexCount + m_vertexCount <= m_vertexCapacity);
	VX_ASSERT(indexCount + m_indexCount <= m_indexCapacity);

	auto vboOffset = sizeof(Vertex) * m_vertexCount;
	auto iboOffset = sizeof(u32) * m_indexCount;

	auto vertexSize = sizeof(Vertex) * vertexCount;
	auto indexSize = sizeof(u32) * indexCount;

	auto b = uploadManager->pushUploadBuffer(vertices, vertexSize, 0, m_vertexBuffer, vboOffset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	VX_ASSERT(b);
	b = uploadManager->pushUploadBuffer(indices, indexSize, 0, m_indexBuffer, iboOffset, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	VX_ASSERT(b);

	m_vertexCount += (u32)vertexCount;
	m_indexCount += (u32)indexCount;

	m_vbv.SizeInBytes = sizeof(Vertex) * m_vertexCount;
	m_ibv.SizeInBytes = sizeof(u32) * m_indexCount;
}

u32 MeshManager::getDynamicEntityOffset() const
{
#if _EDITOR
	return m_staticEntityCapacity + m_dynamicEntityCount;
#else
	return m_staticEntityCount + m_dynamicEntityCount;
#endif
}

void MeshManager::resetDynamicEntityCount()
{
	auto indexCount = m_dynamicEntityCount * 6;
	auto vertexCount = m_dynamicEntityCount * 4;

	m_vertexCount -= vertexCount;
	m_indexCount -= indexCount;

	m_dynamicEntityCount = 0;
}