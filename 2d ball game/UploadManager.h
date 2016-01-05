#pragma once

namespace Allocator
{
	class MainAllocator;
}

#include <vxLib/Container/array.h>
#include "D3DObject.h"
#include <vxLib/Container/DoubleBuffer.h>
#include <gamelib/DebugAllocator.h>

class UploadManager
{
	enum TaskType : u32 { Buffer, Texture };

	struct UploadTask
	{
		TaskType type;
		u32 state;
		D3D12_SUBRESOURCE_FOOTPRINT footprint;
		const u8* src;
		u64 srcOffset;
		u64 size;
		ID3D12Resource* dst;
		union
		{
			u64 dstOffset;
			u32 subresourceIndex;
		};
	};

	struct CopyTask
	{
		TaskType type;
		u32 stateBefore;
		D3D12_SUBRESOURCE_FOOTPRINT footprint;
		ID3D12Resource* dst;
		union
		{
			u64 dstOffset;
			u32 subresourceIndex;
		};
		u64 size;
		u64 srcOffset;
	};

	vx::DoubleBuffer<UploadTask, DebugAllocatorRenderer<Allocator::MainAllocator>> m_uploadTasks;
	vx::array<CopyTask, DebugAllocatorRenderer<Allocator::MainAllocator>> m_copyTasks;
	d3d::CommandAllocator m_cmdAllocator[2];
	d3d::GraphicsCommandList m_cmdList[2];
	d3d::Resource m_uploadBuffer;
	u64 m_uploadBufferSize;
	u64 m_uploadBufferCapacity;
	d3d::Heap m_uploadHeap;

	bool tryUpload(const u8* src, u64 size, u64 offset, ID3D12Resource* dst, u64 dstOffset, u32 state, TaskType type, const D3D12_SUBRESOURCE_FOOTPRINT &footprint);

public:
	UploadManager();
	~UploadManager();

	bool initialize(ID3D12Device* device, Allocator::MainAllocator* allocator);
	void shutdown();

	bool pushUploadBuffer(const u8* src, u64 size, u64 offset, ID3D12Resource* dst, u64 dstOffset, u32 state);
	bool pushUploadTexture(const u8* src, u64 srcSize, u64 srcOffset, ID3D12Resource* dst, u32 subresourceIndex, const D3D12_SUBRESOURCE_FOOTPRINT &footprint);

	void update();
	void buildCmdList(u32 frameIndex);
	void submit(ID3D12CommandList** list, u32* count, u32 frameIndex);

	void execute(ID3D12CommandQueue* queue, u32 frameIndex);

	bool empty() const;
};