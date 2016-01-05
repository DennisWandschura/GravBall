/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "TextRenderer.h"
#include <vxLib/File/FileHandle.h>
#include "ResourceDesc.h"
#include "d3d.h"
#include <vxlib/Graphics/Font.h>
#include <vxlib/Graphics/Texture.h>
#include "UploadManager.h"
#include <gamelib/MainAllocator.h>
#include "GpuResourceManager.h"
//#include "RenderPassText.h"
//#include "ResourceView.h"

namespace Graphics
{
	struct TextRenderer::Entry
	{
		char m_text[48];
		vx::float4 m_color;
		vx::float2 m_position;
		u32 m_size;
	};

	struct TextRenderer::TextVertex
	{
		vx::float3 position;
		vx::float2 uv;
		vx::float4 color;
	};

	TextRenderer::TextRenderer()
		:m_entries(),
		m_vertices(),
		m_drawTextProgram(nullptr),
		m_size(0),
		m_capacity(0),
		m_texureIndex(0),
		m_font(nullptr)
	{

	}

	TextRenderer::~TextRenderer()
	{

	}

	void TextRenderer::getRequiredMemory(u64* bufferSize, u32* bufferCount, u64* textureSize, u32*textureCount, ID3D12Device* device, u32 maxCharacters)
	{
		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * maxCharacters;

		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * maxCharacters;

		auto vertexSizeInBytes = totalVertexCount * sizeof(TextVertex);
		auto indexSizeInBytes = indexCount * sizeof(u32);

		auto fontTexResDesc = d3d::ResourceDesc::createDescTexture2D(vx::uint3(1024, 1024, 1), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_FLAG_NONE);
		auto fontTexAllocInfo = device->GetResourceAllocationInfo(1, 1,&fontTexResDesc);

		*textureSize += fontTexAllocInfo.SizeInBytes;
		*bufferSize += vx::getAlignedSize(vertexSizeInBytes, 64llu KBYTE) + vx::getAlignedSize(indexSizeInBytes, 64llu KBYTE);

		*bufferCount += 2;
		*textureCount += 1;
	}

	bool TextRenderer::createData(ID3D12Device* device, GpuResourceManager* resourceManager, UploadManager* uploadManager, u32 maxCharacters)
	{
		//auto fontTexResDesc = d3d::ResourceDesc::createDescTexture2D(vx::uint3(1024, 1024, 3), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		//auto fontTexAllocInfo = device->GetResourceAllocationInfo(1, 1, &fontTexResDesc);

		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * maxCharacters;

		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * maxCharacters;

		auto vertexSizeInBytes = vx::getAlignedSize(vx::getAlignedSize(totalVertexCount * sizeof(TextVertex), 256) * 2, 64llu KBYTE);

		auto indexSizeInBytes = vx::getAlignedSize(vx::getAlignedSize(indexCount * sizeof(u32), 256), 64llu KBYTE);

		if(!resourceManager->createTextureResource("fontTexture", vx::uint3(1024, 1024, 1), DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, nullptr))
		{
			return false;
		}

		if (!resourceManager->createBufferResource("textVertexBuffer", vertexSizeInBytes, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
		{
			return false;
		}

		if (!resourceManager->createBufferResource("textIndexBuffer", indexSizeInBytes, D3D12_RESOURCE_STATE_INDEX_BUFFER))
		{
			return false;
		}

		return true;
	}

	bool TextRenderer::initialize(u32 maxCharacters, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Device* device, GpuResourceManager* resourceManager, DrawTextProgram* drawTextProgram)
	{
		m_capacity = maxCharacters;

		if (!createData(device, resourceManager, uploadManager, maxCharacters))
			return false;

		vx::array<Entry, DebugAllocatorRenderer<Allocator::MainAllocator>> entries(allocator, s_maxEntryCount);
		m_entries.swap(entries);

		const auto verticesPerCharacter = 4u;
		auto totalVertexCount = verticesPerCharacter * m_capacity;
		const auto indicesPerCharacter = 6u;
		auto indexCount = indicesPerCharacter * m_capacity;

		//auto vertexSizeInBytes = vx::getAlignedSize(totalVertexCount * sizeof(TextVertex), 64llu KBYTE);
		//auto indexSizeInBytes = (indexCount * sizeof(u32), 64llu KBYTE);

		/*
		m_renderPassText = desc->m_renderPassText;

		
		

		auto textVertexBuffer = desc->resourceManager->getBuffer(L"textVertexBuffer");
		d3d::ResourceView vbv;
		vbv.vbv.BufferLocation = textVertexBuffer->GetGPUVirtualAddress();
		vbv.vbv.SizeInBytes = vertexSizeInBytes;
		vbv.vbv.StrideInBytes = sizeof(TextVertex);
		vbv.type = d3d::ResourceView::Type::VertexBufferView;
		desc->resourceManager->insertResourceView("textVbv", vbv);

		auto textIndexBuffer = desc->resourceManager->getBuffer(L"textIndexBuffer");
		d3d::ResourceView textIbv;
		textIbv.type = d3d::ResourceView::Type::IndexBufferView;
		textIbv.ibv.BufferLocation = textIndexBuffer->GetGPUVirtualAddress();
		textIbv.ibv.Format = DXGI_FORMAT_R32_UINT;
		textIbv.ibv.SizeInBytes = indexSizeInBytes;
		desc->resourceManager->insertResourceView("textIbv", textIbv);*/


		vx::array<TextVertex, DebugAllocatorRenderer<Allocator::MainAllocator>> vertices(allocator, totalVertexCount);
		m_vertices.swap(vertices);

		/*auto indexDataSize = sizeof(u32) * indexCount;
		auto indexBlock = allocator->allocate(indexDataSize, __alignof(u32), Allocator::Channels::Renderer, "text renderer indices");
		auto incides = (u32*)indexBlock.ptr;
		for (u32 i = 0, j = 0; i < indexCount; i += 6, j += 4)
		{
			incides[i] = j;
			incides[i + 1] = j + 1;
			incides[i + 2] = j + 2;

			incides[i + 3] = j + 2;
			incides[i + 4] = j + 3;
			incides[i + 5] = j;
		}

		auto uploaded = uploadManager->pushUploadBuffer((u8*)incides, indexDataSize, 0, resourceManager->findBufferResource("textIndexBuffer"), 0, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		VX_ASSERT(uploaded);

		auto indexSubBufferOffset = vx::getAlignedSize(indexDataSize, 256);
		uploaded = uploadManager->pushUploadBuffer((u8*)incides, indexDataSize, indexSubBufferOffset, resourceManager->findBufferResource("textIndexBuffer"), D3D12_RESOURCE_STATE_INDEX_BUFFER);
		VX_ASSERT(uploaded);

		allocator->deallocate(indexBlock, Allocator::Channels::Renderer, "text renderer indices");*/

		m_drawTextProgram = drawTextProgram;

		return true;
	}

	void TextRenderer::shutdown()
	{
		m_vertices.release();
		m_entries.release();
	}

	void TextRenderer::pushEntry(const char(&text)[48], u32 strSize, const vx::float2 &topLeftPosition, const vx::float3 &color)
	{
		VX_ASSERT(m_entries.size() < s_maxEntryCount);

		Entry entry;
		strncpy(entry.m_text, text, 48);
		entry.m_position = topLeftPosition;
		entry.m_color = vx::float4(color, 1.0f);
		entry.m_size = strSize;

		m_entries.push_back(entry);

		m_size += strSize;
	}

	void TextRenderer::update(UploadManager* uploadManager, GpuResourceManager* resourceManager, bool upload)
	{
		if (m_size == 0 || m_font == nullptr)
			return;

		if (upload)
		{
			updateVertexBuffer(uploadManager, resourceManager);
			m_entries.clear();
		}

		m_size = 0;
	}

	void TextRenderer::updateVertexBuffer(UploadManager* uploadManager, GpuResourceManager* resourceManager)
	{
		u32 offset = 0;

		auto fontTexture = m_font->getTexture();
		auto textureSize = fontTexture->getFace(0).getDimension().x;

		vx::float4a invTextureSize;
		invTextureSize.x = 1.0f / textureSize;

		auto vInvTexSize = _mm_shuffle_ps(invTextureSize.v, invTextureSize.v, _MM_SHUFFLE(0, 0, 0, 0));

		auto entryCount = m_entries.size();
		auto entries = m_entries.data();
		for (u32 i = 0; i < entryCount; ++i)
		{
			writeEntryToVertexBuffer(vInvTexSize, entries[i], &offset, textureSize);
		}

		if (offset != 0)
		{
			/*auto indexCount = offset * 6;
			auto vertexCount = offset * 4;

			auto textVertexBuffer = resourceManager->getBuffer(L"textVertexBuffer");
			uploadManager->pushUploadBuffer((u8*)m_vertices.get(), textVertexBuffer->get(), 0, sizeof(TextVertex) * vertexCount, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			m_renderPassText->setIndexCount(indexCount);*/
		}
	}

	void TextRenderer::writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize)
	{
		auto entryText = entry.m_text;
		auto entryTextSize = entry.m_size;
		auto entryOrigin = entry.m_position;
		auto entryColor = vx::loadFloat4(&entry.m_color);

		auto currentSize = m_size;

		if (currentSize + entryTextSize >= m_capacity)
			return;

		u32 vertexOffset = (*offset) * 4;
		*offset += entryTextSize;

		auto vertices = &m_vertices[vertexOffset];
		u32 vertexIndex = 0;

		const f32 scale = 0.25f;
		const __m128 vScale = { scale, scale, 0, 0 };
		const __m128 tmp = { -1.0f, 0, 0, 0 };

		auto cursorPosition = entryOrigin;
		for (u32 i = 0; i < entryTextSize; ++i)
		{
			char ascii_code = entryText[i];

			if (ascii_code == '\n')
			{
				cursorPosition.y -= 53.0f * scale;
				cursorPosition.x = entryOrigin.x;
			}
			else
			{
				__m128 vCursorPos = { cursorPosition.x, cursorPosition.y, 0.0f, 0.0f };

				auto pAtlasEntry = m_font->getAtlasEntry(ascii_code);
				vx::float4a texRect(pAtlasEntry->x, textureSize - pAtlasEntry->y - pAtlasEntry->height, pAtlasEntry->width, pAtlasEntry->height);

				__m128 vAtlasPos = { pAtlasEntry->offsetX, pAtlasEntry->offsetY, 0.0f, 0.0f };

				__m128 vEntrySize = { texRect.z, -texRect.w, 0.0f, 0.0f };

				__m128 vCurrentPosition = _mm_div_ps(vEntrySize, vx::g_VXTwo);
				vCurrentPosition = _mm_add_ps(vCurrentPosition, vAtlasPos);
				vCurrentPosition = vx::fma(vCurrentPosition, vScale, vCursorPos);
				vCurrentPosition = _mm_movelh_ps(vCurrentPosition, tmp);

				__m128 vSource = _mm_mul_ps(texRect, invTextureSize);
				__m128 vSourceSize = _mm_shuffle_ps(vSource, vSource, _MM_SHUFFLE(3, 2, 3, 2));

				static const __m128 texOffsets[4] =
				{
					{ 0, 0, 0, 0 },
					{ 1, 0, 0, 0 },
					{ 1, 1, 0, 0 },
					{ 0, 1, 0, 0 }
				};

				static const __m128 posOffsets[4] =
				{
					{ -0.5f, -0.5f, 0, 0 },
					{ 0.5f, -0.5f, 0, 0 },
					{ 0.5f, 0.5f, 0, 0 },
					{ -0.5f, 0.5f, 0, 0 }
				};

				vEntrySize = _mm_shuffle_ps(texRect, texRect, _MM_SHUFFLE(3, 2, 3, 2));

				for (u32 k = 0; k < 4; ++k)
				{
					u32 index = vertexIndex + k;

					auto pos = _mm_mul_ps(posOffsets[k], vEntrySize);
					pos = vx::fma(pos, vScale, vCurrentPosition);
					_mm_storeu_ps(&vertices[index].position.x, pos);

					__m128 uv = vx::fma(texOffsets[k], vSourceSize, vSource);
					_mm_storeu_ps(&vertices[index].uv.x, uv);

					_mm_storeu_ps(&vertices[index].color.x, entryColor);
				}
				vertexIndex += 4;

				cursorPosition.x = fmaf(pAtlasEntry->advanceX, scale, cursorPosition.x);
			}
		}
	}
}