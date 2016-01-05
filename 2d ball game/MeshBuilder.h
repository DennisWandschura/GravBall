#pragma once

struct StaticEntityFile;
struct DynamicEntityFile;
class TextureArrayManager;
struct GravityAreaFile;
struct GravityWellFile;
struct DynamicEntity;

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/Allocator/Allocator.h>

struct MeshBuilderOutput
{
	vx::AllocatedBlock vertexBlock;
	vx::AllocatedBlock indexBlock;
	vx::AllocatedBlock transformBlock;
	u32 vertexCount;
	u32 indexCount;
	u64 vertexSizeBytes;
	u64 indexSizeBytes;
	u64 transformSizeBytes;
};

class MeshBuilder
{
public:
	static bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const StaticEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output);
	static bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const DynamicEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output);
	static bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const DynamicEntity* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output);

	static bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const GravityAreaFile* areas, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output);

	static bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const GravityWellFile* wells, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output);
};