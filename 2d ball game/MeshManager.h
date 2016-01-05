#pragma once

namespace Allocator
{
	class MainAllocator;
}
class UploadManager;
struct StaticEntityFile;
struct DynamicEntityFile;
class TextureArrayManager;
struct GravityAreaFile;
struct GravityWellFile;
struct DynamicEntity;

#include "D3DObject.h"
#include <vxLib/Container/array.h>

struct MeshManagerAddDynamicMeshesDesc
{
	const StaticEntityFile* entities;
	size_t entityCount; 
	Allocator::MainAllocator* allocator;
	UploadManager* uploadManager; 
	ID3D12Resource* frameData0TransformBlock;
	ID3D12Resource* frameData1TransformBlock;
};

struct MeshManagerUpdateMeshDesc
{
	u32 transformOffset;
	u32 vertexOffset; u32 indexOffset;
	Allocator::MainAllocator* allocator;
	UploadManager* uploadManager; 
	ID3D12Resource* frameData0TransformBlock;
	ID3D12Resource* frameData1TransformBlock;
	TextureArrayManager* textureManager;
};

class MeshManager
{
	D3D12_VERTEX_BUFFER_VIEW m_vbv;
	D3D12_INDEX_BUFFER_VIEW m_ibv;
	d3d::Resource m_vertexBuffer;
	d3d::Resource m_indexBuffer;
	u32 m_staticEntityCount;
	u32 m_staticEntityCapacity;
	u32 m_dynamicEntityCount;
	u32 m_dynamicEntityCapacity;
	u32 m_entityCapacity;
	u32 m_vertexCapacity;
	u32 m_vertexCount;
	u32 m_indexCapacity;
	u32 m_indexCount;
	d3d::Heap m_heap;

	void uploadMeshData(const u8* vertices, u32 vertexCount, const u8* indices, u32 indexCount, UploadManager* uploadManager);

	template<typename T>
	bool buildMeshes(u32 offset, const T* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);

	template<typename T>
	bool addMeshesImpl(u32 offset, const T* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);

	template<typename T>
	void updateMeshImpl(const T &entity, const MeshManagerUpdateMeshDesc &desc);
public:
	MeshManager();
	~MeshManager();

	bool initialize(ID3D12Device* device, u32 vertexCount, u32 indexCount, u32 dynamicEntityCapacity, u32 staticEntityCapacity, Allocator::MainAllocator* allocator);
	void shutdown();

	void clear();

	bool addMeshes(const StaticEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);
	bool addMeshes(const GravityAreaFile* areas, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);
	bool addMeshes(const DynamicEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);
	bool addMeshes(const DynamicEntity* entities, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);
	bool addMeshes(const GravityWellFile* areas, u32 count, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Resource* frameData0TransformBlock, ID3D12Resource* frameData1TransformBlock, TextureArrayManager* textureManager);

	void updateMesh(const StaticEntityFile &entity, const MeshManagerUpdateMeshDesc &desc);
	void updateMesh(const DynamicEntityFile &entity, const MeshManagerUpdateMeshDesc &desc);
	void updateMesh(const GravityAreaFile &entity, const MeshManagerUpdateMeshDesc &desc);
	void updateMesh(const GravityWellFile &entity, const MeshManagerUpdateMeshDesc &desc);

	const D3D12_VERTEX_BUFFER_VIEW& getVertexBufferView() const { return m_vbv; }
	const D3D12_INDEX_BUFFER_VIEW& getIndexBufferView() { return m_ibv; }

	u32 getIndexCount() const { return m_indexCount; }
	u32 getVertexCount() const { return m_vertexCount; }

	u32 getStaticEntityOffset() const { return m_staticEntityCount; }
	u32 getDynamicEntityOffset() const;

	void resetDynamicEntityCount();
};