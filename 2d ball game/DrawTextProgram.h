#pragma once

struct RenderFrameData;
class SwapChain;
class ShaderManager;
class TextureArrayManager;

#include "D3DObject.h"
#include "DescriptorHeap.h"

class DrawTextProgram
{
	u32 m_indexCount;

public:
	DrawTextProgram();
	~DrawTextProgram();

	bool initialize(ShaderManager* shaderManager, ID3D12Device* device, RenderFrameData* frameData0, RenderFrameData* frameData1, TextureArrayManager* textureManager);
	void shutdown();

	void buildCmdList(RenderFrameData* frameData, u32 idx, SwapChain* swapChain);
	void submit(ID3D12CommandList** list, u32* count);

	void setIndexCount(u32 count);
};