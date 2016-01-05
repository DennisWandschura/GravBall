#if _VX_WINDOWS
#pragma once

#include <vxLib/math/matrix.h>

#define CBUFFER(NAME, SLOT) struct NAME

typedef vx::mat4 float4x4;
typedef vx::float4a float4;

#else

#define CBUFFER(NAME, SLOT) cbuffer NAME : register(b##SLOT)

#endif

CBUFFER(GpuCameraBuffer, 0)
{
	float4x4 pvMatrix;
	float4 cameraPosition;
};