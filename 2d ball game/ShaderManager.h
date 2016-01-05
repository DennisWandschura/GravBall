#pragma once

namespace Allocator
{
	class MainAllocator;
}

#include "D3DObject.h"
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <gamelib/DebugAllocator.h>

class ShaderManager
{
	vx::sorted_array<vx::StringID, d3d::Blob, DebugAllocatorRenderer<Allocator::MainAllocator>> m_programs;

public:
	ShaderManager();
	~ShaderManager();

	bool initialize(u32 capacity, Allocator::MainAllocator* allocator);
	void shutdown();

	bool loadShader(const wchar_t* filename, const char* id);

	d3d::Blob* findShader(const char* id);
};