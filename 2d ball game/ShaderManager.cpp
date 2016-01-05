#include "ShaderManager.h"
#include <d3dcompiler.h>
#include <gamelib/MainAllocator.h>

ShaderManager::ShaderManager()
	:m_programs()
{

}

ShaderManager::~ShaderManager()
{

}

bool ShaderManager::initialize(u32 capacity, Allocator::MainAllocator* allocator)
{
	m_programs = vx::sorted_array<vx::StringID, d3d::Blob, DebugAllocatorRenderer<Allocator::MainAllocator>>(allocator, capacity);

	return true;
}

void ShaderManager::shutdown()
{
	m_programs.release();
}

bool ShaderManager::loadShader(const wchar_t* filename, const char* id)
{
	d3d::Blob blob;
	auto hr = D3DReadFileToBlob(filename, (ID3D10Blob**)&blob);
	if (hr != 0)
		return false;

	m_programs.insert(vx::make_sid(id), std::move(blob));

	return true;
}

d3d::Blob* ShaderManager::findShader(const char* id)
{
	return m_programs.find(vx::make_sid(id));
}