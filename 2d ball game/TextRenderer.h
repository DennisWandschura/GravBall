#pragma once

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

struct ID3D12Device;
class UploadManager;

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/math/Vector.h>
#include <vxLib/Container/array.h>
#include <gamelib/DebugAllocator.h>

class DrawTextProgram;

namespace vx
{
	namespace Graphics
	{
		class Font;
	}
}

namespace Graphics
{
	class GpuResourceManager;

	struct TextVertex
	{
		vx::float3 position;
		vx::float2 texCoords;
		vx::float4 color;
	};

	class TextRenderer
	{
		static const auto s_maxEntryCount = 256u;
		struct Entry;
		struct TextVertex;

		vx::array<Entry, DebugAllocatorRenderer<Allocator::MainAllocator>> m_entries;
		vx::array<TextVertex, DebugAllocatorRenderer<Allocator::MainAllocator>> m_vertices;
		DrawTextProgram* m_drawTextProgram;
		u32 m_size;
		u32 m_capacity;
		u32 m_texureIndex;
		const vx::Graphics::Font* m_font;

		bool createData(ID3D12Device* device, GpuResourceManager* resourceManager, UploadManager* uploadManager, u32 maxCharacters);

		void updateVertexBuffer(UploadManager* uploadManager, GpuResourceManager* resourceManager);
		void writeEntryToVertexBuffer(const __m128 invTextureSize, const Entry &entry, u32* offset, u32 textureSize);

	public:
		TextRenderer();
		~TextRenderer();

		void getRequiredMemory(u64* bufferSize, u32* bufferCount, u64* textureSize, u32*textureCount, ID3D12Device* device, u32 maxCharacters);

		bool initialize(u32 maxCharacters, Allocator::MainAllocator* allocator, UploadManager* uploadManager, ID3D12Device* device, GpuResourceManager* resourceManager, DrawTextProgram* drawTextProgram);
		void shutdown();

		void pushEntry(const char(&text)[48], u32 strSize, const vx::float2 &topLeftPosition, const vx::float3 &color);

		void setFont(const vx::Graphics::Font* font) { m_font = font; }

		void update(UploadManager* uploadManager, GpuResourceManager* resourceManager, bool upload);
	};
}