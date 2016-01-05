#include "DefaultProgram.h"
#include "RenderFrameData.h"
#include "algorithm.h"
#include "ShaderManager.h"
#include "d3dx12.h"
#include "SwapChain.h"
#include "MeshManager.h"
#include "UploadManager.h"
#include "TextureArrayManager.h"

DefaultProgram::DefaultProgram()
	:m_currentCmdList(nullptr),
	m_cmdList()
{

}

DefaultProgram::~DefaultProgram()
{

}

bool DefaultProgram::createRootSignature(ID3D12Device* device)
{
	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;
	params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 0;
	params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_DESCRIPTOR_RANGE range[1];
	range[0].BaseShaderRegister = 1;
	range[0].NumDescriptors = 1;
	range[0].OffsetInDescriptorsFromTableStart = 0;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].RegisterSpace = 0;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = range;
	params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.NumParameters = _countof(params);
	desc.pParameters = params;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 1;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	desc.NumStaticSamplers = 1;
	desc.pStaticSamplers = &samplerDesc;

	d3d::Blob blob, errorBlob;
	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, blob.getAddressOf(), errorBlob.getAddressOf());
	if (hr != 0)
		return false;

	hr = device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.getAddressOf()));
	if (hr != 0)
		return false;

	return true;
}

bool DefaultProgram::createPipelineState(ShaderManager* shaderManager, ID3D12Device* device)
{
	auto vsShader = shaderManager->findShader("defaultvs.cso");
	auto psShader = shaderManager->findShader("defaultps.cso");

	D3D12_INPUT_ELEMENT_DESC inputElements[]=
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = m_rootSignature;
	desc.VS.pShaderBytecode = (*vsShader)->GetBufferPointer();
	desc.VS.BytecodeLength = (*vsShader)->GetBufferSize();
	desc.PS.pShaderBytecode = (*psShader)->GetBufferPointer();
	desc.PS.BytecodeLength = (*psShader)->GetBufferSize();
	desc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
	desc.BlendState.RenderTarget[0].BlendEnable = 1;
	desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	desc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
	desc.RasterizerState.FrontCounterClockwise = 1;
	desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
	desc.InputLayout = { inputElements, _countof(inputElements) };
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc.Count = 1;
	desc.SampleMask = UINT_MAX;
	desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	auto hr = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pipelineState.getAddressOf()));
	if (hr != 0)
		return false;

	return true;
}

bool DefaultProgram::initialize(ShaderManager* shaderManager, ID3D12Device* device, RenderFrameData* frameData0, RenderFrameData* frameData1, TextureArrayManager* textureManager)
{
	if (!createRootSignature(device))
		return false;

	if (!createPipelineState(shaderManager, device))
		return false;

	if (!m_cmdList[0].create(D3D12_COMMAND_LIST_TYPE_DIRECT, frameData0->cmdAllocator, device, m_pipelineState))
		return false;
	m_cmdList[0]->Close();

	if (!m_cmdList[1].create(D3D12_COMMAND_LIST_TYPE_DIRECT, frameData1->cmdAllocator, device, m_pipelineState))
		return false;
	m_cmdList[1]->Close();

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	if (!m_descriptorHeap.create(descHeapDesc, device))
		return false;

	auto srvDesc = textureManager->getSrvDesc();

	auto cpuHandle = m_descriptorHeap.getCpuHandle();
	device->CreateShaderResourceView(frameData0->srgbaTexture,&srvDesc, cpuHandle);

	cpuHandle.inc();
	device->CreateShaderResourceView(frameData1->srgbaTexture, &srvDesc, cpuHandle);

	return true;
}

void DefaultProgram::shutdown()
{
	m_cmdList[1].release();
	m_cmdList[0].release();
	m_pipelineState.release();
	m_rootSignature.release();
}

void DefaultProgram::buildCmdList(RenderFrameData* frameData, u32 idx, SwapChain* swapChain)
{
	auto resolution = frameData->dim;

	D3D12_VIEWPORT viewport;
	viewport.Height = (f32)resolution.y;
	viewport.Width = (f32)resolution.x;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	D3D12_RECT rectScissor;
	rectScissor.left = 0;
	rectScissor.top = 0;
	rectScissor.right = resolution.x;
	rectScissor.bottom = resolution.y;

#if _EDITOR
	f32 clearColor[4] = { 0.1f, 0.1f, 0.1f, 0.0f };
#else
	f32 clearColor[4] = { 0, 0, 0, 0 };
#endif

	m_currentCmdList = m_cmdList[idx];

	m_currentCmdList->Reset(frameData->cmdAllocator, m_pipelineState);

	m_currentCmdList->RSSetScissorRects(1, &rectScissor);
	m_currentCmdList->RSSetViewports(1, &viewport);

	auto currentBackBufferIndex = swapChain->getCurrentBackBufferIndex();
	auto rtvCpuHandle = swapChain->getCpuDescriptorHandle();
	rtvCpuHandle.inc(currentBackBufferIndex);

	D3D12_CPU_DESCRIPTOR_HANDLE depthHandle = frameData->depthCpuHandle;
	m_currentCmdList->OMSetRenderTargets(1, &rtvCpuHandle, 1, &depthHandle);
	m_currentCmdList->ClearRenderTargetView(rtvCpuHandle, clearColor, 0, nullptr);
	m_currentCmdList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* descHeaps[] = { m_descriptorHeap };
	auto gpuHandle = m_descriptorHeap.getGpuHandle();
	gpuHandle.inc(idx);

	m_currentCmdList->SetDescriptorHeaps(1, descHeaps);

	m_currentCmdList->SetGraphicsRootSignature(m_rootSignature);
	m_currentCmdList->SetGraphicsRootConstantBufferView(0, frameData->staticBuffer->GetGPUVirtualAddress() + RenderFrameData::CameraBufferOffset);
	m_currentCmdList->SetGraphicsRootShaderResourceView(1, frameData->transformBuffer->GetGPUVirtualAddress());
	m_currentCmdList->SetGraphicsRootDescriptorTable(2, gpuHandle);

	m_currentCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_currentCmdList->IASetVertexBuffers(0, 1, &frameData->vbv);
	m_currentCmdList->IASetIndexBuffer(&frameData->ibv);
	m_currentCmdList->DrawIndexedInstanced(frameData->indexCount, 1, 0, 0, 0);

	m_currentCmdList->Close();
}

void DefaultProgram::submit(ID3D12CommandList** list, u32* count)
{
	list[*count] = m_currentCmdList;
	++(*count);
}