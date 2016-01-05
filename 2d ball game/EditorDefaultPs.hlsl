struct VS_OUTPUT
{
	float4 pos: SV_POSITION;
	float2 wsPosition : POSITION1;
	float2 cameraPosition : POSITION2;
	float3 texCoords : TEXCOORDS0;
};

#define PI 3.14159265359

Texture2DArray g_diffuseTextures : register(t1);
SamplerState g_sampler : register(s0);

float4 main(VS_OUTPUT input) : SV_TARGET
{
	float4 color = g_diffuseTextures.Sample(g_sampler, input.texCoords);// / PI;

	return float4(color);
}