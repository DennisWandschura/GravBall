#pragma once

class UploadManager;

namespace vx
{
	class LinearAllocator;
}

#include <gamelib/Entity.h>
#include "D3DObject.h"
#include <vxLib/Graphics/Camera.h>
#include <vxLib/Container/array.h>
#include "DescriptorHeap.h"
#include <vxLib/Allocator/DelegateAllocator.h>

struct RenderFrameDynamicData
{
	vx::array<RenderComponent, vx::DelegateAllocator<vx::LinearAllocator>> components;
	vx::float2a cameraPosition;

	RenderFrameDynamicData();
	~RenderFrameDynamicData();

	void swap(RenderFrameDynamicData &other)
	{
		components.swap(other.components);
	}
};

struct RtvDesc
{
	vx::uint2 dim;
	ID3D12Heap* rtvHeap;
	u64* heapOffset;
	u64 size;
};

struct GpuCameraBufferData
{
	vx::mat4 pvMatrix;
};

struct GpuStaticCameraBuffer
{
	vx::float4 orthoMatrix;
};

struct RenderFrameData
{
	enum :size_t { StaticCameraBufferOffset = 0, CameraBufferOffset = StaticCameraBufferOffset + vx::GetAlignedSize<sizeof(GpuStaticCameraBuffer), 256>::size};

	d3d::CommandAllocator cmdAllocator;
	d3d::Resource staticBuffer;
	d3d::Resource transformBuffer;
	vx::uint2 dim;
	D3D12_VERTEX_BUFFER_VIEW vbv;
	D3D12_INDEX_BUFFER_VIEW ibv;
	ID3D12Resource* srgbaTexture;
	d3d::CpuDescriptorHandle depthCpuHandle;
	u32 indexCount;
	u32 padding;
	d3d::Fence fence;
	u64 fenceValue;
	u64 lastFenceValue;
	void* fenceEvent;
	RenderFrameDynamicData dynamicData;
	u32 dynamicTransformOffset;
	u32 padding1;

	RenderFrameData() :staticBuffer(), transformBuffer(), fence(), dim(), vbv(), ibv(), srgbaTexture(nullptr), depthCpuHandle(), fenceValue(0),
		lastFenceValue(0), fenceEvent(nullptr), indexCount(0), cmdAllocator(), dynamicData(), dynamicTransformOffset(0) {}

	bool create(const vx::uint2 &fbDim, ID3D12Heap* bufferHeap, u64* bufferHeapOffset, u32 maxTransformCount, ID3D12Device* device, d3d::CpuDescriptorHandle depthHandle);
	void shutdown();

	void signal(ID3D12CommandQueue* queue);

	bool isReady();

	void wait();

	void releaseCpuMemory();

	void updateDynamicTransforms(UploadManager* uploadManager);

	static u64 getAllocSize(u32 maxTransformCount, u32 frameCount);
};