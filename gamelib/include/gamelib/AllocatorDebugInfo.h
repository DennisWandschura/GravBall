#pragma once

#include <vxlib/types.h>

namespace Allocator
{
	enum class Channels : u16
	{
		General,
		Entity,
		Renderer,
		Physics,
		Events,
		Resource
	};

	enum class DebugInfoType : u16
	{
		Allocation,
		Deallocation
	};

	struct DebugInfo
	{
		enum { BufferSize = 64 };

		u64 index;
		u64 ptr;
		u64 allocatedSize;
		u64 requestedSize;
		u64 frame;
		f32 time;
		Channels channel;
		DebugInfoType type;
		char msg[BufferSize];
	};

	struct DebugInfoFileHeader
	{
		u64 entryCount;
		f32 endTime;
		u32 padding;
	};
}