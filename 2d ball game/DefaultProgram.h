#pragma once

struct RenderFrameData;
class SwapChain;
class ShaderManager;
class TextureArrayManager;

#include "D3DObject.h"
#include "DescriptorHeap.h"

class DefaultProgram
{
	ID3D12GraphicsCommandList* m_currentCmdList;
	d3d::GraphicsCommandList m_cmdList[2];
	d3d::PipelineState m_pipelineState;
	d3d::RootSignature m_rootSignature;
	d3d::DescriptorHeap m_descriptorHeap;

	bool createRootSignature(ID3D12Device* device);
	bool createPipelineState(ShaderManager* shaderManager, ID3D12Device* device);

public:
	DefaultProgram();
	~DefaultProgram();

	bool initialize(ShaderManager* shaderManager, ID3D12Device* device, RenderFrameData* frameData0, RenderFrameData* frameData1, TextureArrayManager* textureManager);
	void shutdown();

	void buildCmdList(RenderFrameData* frameData, u32 idx, SwapChain* swapChain);
	void submit(ID3D12CommandList** list, u32* count);
};