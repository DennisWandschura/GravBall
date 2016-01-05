#pragma once

class Engine;
namespace Allocator
{
	class MainAllocator;
}

namespace dev
{
	extern bool createLevel(Engine* engine, Allocator::MainAllocator* allocator);
	extern bool loadTextures(Engine* engine, Allocator::MainAllocator* allocator);
}