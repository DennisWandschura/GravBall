#include "RenderFrameData.h"
#include <vxLib/Allocator/Allocator.h>
#include <gamelib/MainAllocator.h>
#include "UploadManager.h"

bool createBuffer(d3d::Resource* resource, ID3D12Heap* bufferHeap, u64 offset, u64 size, D3D12_RESOURCE_STATES initialState, ID3D12Device* device)
{
	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 64 KBYTE;
	desc.Width = size;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	return resource->create(bufferHeap, offset, desc, initialState, device);
}

bool createRtv(d3d::Resource* resource, const RtvDesc &rtvDesc, D3D12_RESOURCE_STATES initialState, ID3D12Device* device)
{
	auto offset = *rtvDesc.heapOffset;

	D3D12_RESOURCE_DESC desc{};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 64 KBYTE;
	desc.Width = rtvDesc.dim.x;
	desc.Height = rtvDesc.dim.y;
	desc.DepthOrArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.MipLevels = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

	auto b = resource->create(rtvDesc.rtvHeap, offset, desc, initialState, device, &clearValue);
	if (b)
	{
		*rtvDesc.heapOffset += rtvDesc.size;
	}
	
	return b;
}

RenderFrameDynamicData::RenderFrameDynamicData()
	:components(),
	cameraPosition()
{

}

RenderFrameDynamicData::~RenderFrameDynamicData()
{

}

bool RenderFrameData::create(const vx::uint2 &fbDim, ID3D12Heap* bufferHeap, u64* bufferHeapOffset, u32 maxTransformCount, ID3D12Device* device, d3d::CpuDescriptorHandle depthHandle)
{
	auto offset = *bufferHeapOffset;

	const auto constantBufferSize = vx::getAlignedSize(sizeof(GpuCameraBufferData) + vx::getAlignedSize(sizeof(D3D12_SHADER_RESOURCE_VIEW_DESC), 256), 64 KBYTE);
	if (!createBuffer(&staticBuffer, bufferHeap, offset, constantBufferSize, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, device))
		return false;
	offset += constantBufferSize;

	const auto transformBufferSize = vx::getAlignedSize(sizeof(vx::float4) * maxTransformCount, 64 KBYTE);
	if (!createBuffer(&transformBuffer, bufferHeap, offset, transformBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, device))
		return false;
	offset += transformBufferSize;

	*bufferHeapOffset = offset;

	auto hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.getAddressOf()));
	if (hr != 0)
		return false;

	if (!cmdAllocator.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
		return false;

	this->dim = fbDim;
	this->depthCpuHandle = depthHandle;

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return true;
}

void RenderFrameData::shutdown()
{
	if (fenceEvent)
	{
		CloseHandle(fenceEvent);
		fenceEvent = nullptr;
	}

	cmdAllocator.release();
	fence.release();
	transformBuffer.release();
	staticBuffer.release();
}

void RenderFrameData::signal(ID3D12CommandQueue* queue)
{
	lastFenceValue = fenceValue++;
	queue->Signal(fence, fenceValue);
}


bool RenderFrameData::isReady()
{
	auto lastCompletedFence = fence->GetCompletedValue();
	if (lastCompletedFence < fenceValue)
		return false;

	return true;
}

void RenderFrameData::wait()
{
	if (!isReady())
	{
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void RenderFrameData::releaseCpuMemory()
{
	dynamicData.components.release();
}

void RenderFrameData::updateDynamicTransforms(UploadManager* uploadManager)
{
	for (auto &it : dynamicData.components)
	{
		auto idx = it.rendererIndex;
		auto dstOffset = sizeof(vx::float4) * idx;

		uploadManager->pushUploadBuffer((u8*)&it.position, sizeof(vx::float2), 0, transformBuffer, dstOffset, D3D12_RESOURCE_STATE_GENERIC_READ);
	}
}

u64 RenderFrameData::getAllocSize(u32 maxTransformCount, u32 frameCount)
{
	const auto cameraBufferSize = 64 KBYTE;
	const auto transformBufferSize = vx::getAlignedSize(sizeof(vx::float4) * maxTransformCount, 64 KBYTE);

	return (cameraBufferSize + transformBufferSize) * frameCount;
}