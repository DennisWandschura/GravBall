#include "DrawTextProgram.h"

DrawTextProgram::DrawTextProgram()
{

}

DrawTextProgram::~DrawTextProgram()
{

}

bool DrawTextProgram::initialize(ShaderManager* shaderManager, ID3D12Device* device, RenderFrameData* frameData0, RenderFrameData* frameData1, TextureArrayManager* textureManager)
{
	return true;
}

void DrawTextProgram::shutdown()
{

}

void DrawTextProgram::buildCmdList(RenderFrameData* frameData, u32 idx, SwapChain* swapChain)
{

}

void DrawTextProgram::submit(ID3D12CommandList** list, u32* count)
{

}