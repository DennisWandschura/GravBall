#include "MeshBuilder.h"
#include <gamelib/EntityFile.h>
#include "Vertex.h"
#include <gamelib/MainAllocator.h>
#include "TextureArrayManager.h"
#include <gamelib/GravityArea.h>
#include <gamelib/GravityWell.h>
#include <gamelib/Entity.h>

namespace MeshBuilderCpp
{
	template<typename LAYER, typename T>
	bool build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const T* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
	{
		u32 totalVertexCount = (u32)count * 4;
		u32 totalIndexCount = (u32)count * 6;

		const Vertex defaultVertices[4] =
		{
			{ { -1, -1, LAYER::Z_LAYER },{ 0, 0, 0 }, 0 },
			{ { 1, -1, LAYER::Z_LAYER },{ 1, 0, 0 }, 0 },
			{ { 1, 1, LAYER::Z_LAYER },{ 1, 1, 0 }, 0 },
			{ { -1, 1, LAYER::Z_LAYER },{ 0, 1, 0 }, 0 }
		};

		const u32 defaultIndices[6] = { 0, 1, 2, 2, 3, 0 };

		auto vertexBlock = allocator->allocate(sizeof(Vertex) * totalVertexCount, __alignof(Vertex), Allocator::Channels::Renderer, "mesh vertex data");
		auto indexBlock = allocator->allocate(sizeof(u32) * totalIndexCount, __alignof(u32), Allocator::Channels::Renderer, "mesh index data");
		auto transformBlock = allocator->allocate(sizeof(vx::float4) * totalIndexCount, __alignof(vx::float4), Allocator::Channels::Renderer, "mesh transform data");

		auto vertexPtr = (Vertex*)vertexBlock.ptr;
		auto indexPtr = (u32*)indexBlock.ptr;
		auto transformPtr = (vx::float4*)transformBlock.ptr;
		for (u32 i = 0; i < count; ++i)
		{
			auto &entity = entities[i];

			auto textureIdx = textureManager->findTexture(entity.getTexture());
			if (textureIdx == (size_t)-1)
			{
				allocator->deallocate(transformBlock, Allocator::Channels::Renderer, "mesh transform data");
				allocator->deallocate(indexBlock, Allocator::Channels::Renderer, "mesh index data");
				allocator->deallocate(vertexBlock, Allocator::Channels::Renderer, "mesh vertex data");
				return false;
			}

			auto position = entity.getPosition();
			*transformPtr = vx::float4(position.x, position.y, 0, 0);

			vertexPtr[0] = defaultVertices[0];
			vertexPtr[1] = defaultVertices[1];
			vertexPtr[2] = defaultVertices[2];
			vertexPtr[3] = defaultVertices[3];

			vertexPtr[0].index = meshOffset + i;
			vertexPtr[1].index = meshOffset + i;
			vertexPtr[2].index = meshOffset + i;
			vertexPtr[3].index = meshOffset + i;

			vertexPtr[0].texCoords.z = (f32)textureIdx;
			vertexPtr[1].texCoords.z = (f32)textureIdx;
			vertexPtr[2].texCoords.z = (f32)textureIdx;
			vertexPtr[3].texCoords.z = (f32)textureIdx;

			auto shapeType = entity.getShapeType();
			if (shapeType == EntityShapeType::Circle)
			{
				auto radius = entity.getRadius();

				vertexPtr[0].position *= radius;
				vertexPtr[1].position *= radius;
				vertexPtr[2].position *= radius;
				vertexPtr[3].position *= radius;
			}
			else if (shapeType == EntityShapeType::Rectangle)
			{
				auto halfDim = entity.getHalfDim();

				vertexPtr[0].position.x *= halfDim.x;
				vertexPtr[0].position.y *= halfDim.y;

				vertexPtr[1].position.x *= halfDim.x;
				vertexPtr[1].position.y *= halfDim.y;

				vertexPtr[2].position.x *= halfDim.x;
				vertexPtr[2].position.y *= halfDim.y;

				vertexPtr[3].position.x *= halfDim.x;
				vertexPtr[3].position.y *= halfDim.y;
			}

			indexPtr[0] = defaultIndices[0] + vertexOffset;
			indexPtr[1] = defaultIndices[1] + vertexOffset;
			indexPtr[2] = defaultIndices[2] + vertexOffset;

			indexPtr[3] = defaultIndices[3] + vertexOffset;
			indexPtr[4] = defaultIndices[4] + vertexOffset;
			indexPtr[5] = defaultIndices[5] + vertexOffset;

			vertexPtr += 4;
			vertexOffset += 4;

			indexOffset += 6;
			indexPtr += 6;

			++transformPtr;
		}

		output->vertexSizeBytes = (u8*)vertexPtr - vertexBlock.ptr;
		output->indexSizeBytes = (u8*)indexPtr - indexBlock.ptr;
		output->transformSizeBytes = (u8*)transformPtr - transformBlock.ptr;

		output->transformBlock = transformBlock;
		output->vertexBlock = vertexBlock;
		output->indexBlock = indexBlock;
		output->vertexCount = totalVertexCount;
		output->indexCount = totalIndexCount;

		return true;
	}

	struct EntityLayer
	{
		static const f32 Z_LAYER;
	};

	struct GravityAreaLayer
	{
		static const f32 Z_LAYER;
	};

	struct GravityWellLayer
	{
		static const f32 Z_LAYER;
	};
}

const f32 MeshBuilderCpp::EntityLayer::Z_LAYER = 0.0f;
const f32 MeshBuilderCpp::GravityAreaLayer::Z_LAYER = -0.1f;
const f32 MeshBuilderCpp::GravityWellLayer::Z_LAYER = -0.1f;

bool MeshBuilder::build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const StaticEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
{
	return MeshBuilderCpp::build<MeshBuilderCpp::EntityLayer>(meshOffset, vertexOffset, indexOffset, entities, count, allocator, textureManager, output);
}

bool MeshBuilder::build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const DynamicEntityFile* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
{
	return MeshBuilderCpp::build<MeshBuilderCpp::EntityLayer>(meshOffset, vertexOffset, indexOffset, entities, count, allocator, textureManager, output);
}

bool MeshBuilder::build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const DynamicEntity* entities, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
{
	return MeshBuilderCpp::build<MeshBuilderCpp::EntityLayer>(meshOffset, vertexOffset, indexOffset, entities, count, allocator, textureManager, output);
}

bool MeshBuilder::build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const GravityAreaFile* areas, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
{
	return MeshBuilderCpp::build<MeshBuilderCpp::GravityAreaLayer>(meshOffset, vertexOffset, indexOffset, areas, count, allocator, textureManager, output);
}

bool MeshBuilder::build(u32 meshOffset, u32 vertexOffset, u32 indexOffset, const GravityWellFile* wells, u32 count, Allocator::MainAllocator* allocator, TextureArrayManager* textureManager, MeshBuilderOutput* output)
{
	return MeshBuilderCpp::build<MeshBuilderCpp::GravityWellLayer>(meshOffset, vertexOffset, indexOffset, wells, count, allocator, textureManager, output);
}