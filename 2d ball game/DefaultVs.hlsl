#include "gpucamerabuffer.h"
struct Vertex
{
	float3 position : POSITION0;
	float3 texCoords : TEXCOORDS0;
	uint index : INDEX0;
};

struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 wsPosition : POSITION1;
	float2 cameraPosition : POSITION2;
	float3 texCoords : TEXCOORDS0;
};

StructuredBuffer<float4> g_transforms : register(t0);

VS_OUTPUT main(Vertex vertex)
{
	float4 wsPosition = float4(vertex.position, 1) + g_transforms[vertex.index];

	VS_OUTPUT output;
	output.pos = mul(pvMatrix, wsPosition);
	output.wsPosition = wsPosition.xy;
	output.cameraPosition = cameraPosition.xy;
	output.texCoords = vertex.texCoords;

	return output;
}