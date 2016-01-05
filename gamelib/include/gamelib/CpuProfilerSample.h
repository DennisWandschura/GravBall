#pragma once

#include <vxLib/types.h>

struct CpuProfilerSample
{
	u64 frameIndex;
	f32 time;
	u32 layer;
	char msg[64];
};

struct CpuProfilerFileHeader
{
	u64 frameTimeCount;
	u64 sampleCount;
};