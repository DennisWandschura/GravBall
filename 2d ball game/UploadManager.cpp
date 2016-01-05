#include "UploadManager.h"
#include <gamelib/MainAllocator.h>
#include "d3dx12.h"

UploadManager::UploadManager()
	:m_uploadTasks(),
	m_copyTasks(),
	m_cmdAllocator(),
	m_cmdList(),
	m_uploadBuffer(),
	m_uploadBufferSize(0),
	m_uploadBufferCapacity(0),
	m_uploadHeap()
{

}

UploadManager::~UploadManager()
{

}

bool UploadManager::initialize(ID3D12Device* device, Allocator::MainAllocator* allocator)
{
	const auto heapSize = vx::GetAlignedSize<16 MBYTE, 64 KBYTE>::size;

	D3D12_HEAP_DESC desc{};
	desc.SizeInBytes = heapSize;
	desc.Alignment = 64 KBYTE;
	desc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
	desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	if (!m_uploadHeap.create(desc, device))
		return false;

	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 64 KBYTE;
	bufferDesc.Width = heapSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	if (!m_uploadBuffer.create(m_uploadHeap, 0, bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, device))
		return false;
	m_uploadBuffer->SetName(L"UploadBuffer");

	m_uploadBufferCapacity = heapSize;

	if (!m_cmdAllocator[0].create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
		return false;

	if (!m_cmdAllocator[1].create(D3D12_COMMAND_LIST_TYPE_DIRECT, device))
		return false;

	if (!m_cmdList[0].create(D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator[0], device))
		return false;
	m_cmdList[0]->Close();

	if (!m_cmdList[1].create(D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator[1], device))
		return false;
	m_cmdList[1]->Close();

	m_uploadTasks = vx::DoubleBuffer<UploadTask, DebugAllocatorRenderer<Allocator::MainAllocator>>(allocator, 64);
	m_copyTasks = vx::array<CopyTask, DebugAllocatorRenderer<Allocator::MainAllocator>>(allocator, 64);

	return true;
}

void UploadManager::shutdown()
{
	m_copyTasks.release();
	m_uploadTasks.release();

	m_cmdList[0].release();
	m_cmdList[1].release();
	m_cmdAllocator[0].release();
	m_cmdAllocator[1].release();

	m_uploadBuffer.release();
	m_uploadHeap.release();
}

bool UploadManager::tryUpload(const u8* src, u64 size, u64 offset, ID3D12Resource* dst, u64 dstOffset, u32 state, TaskType type, const D3D12_SUBRESOURCE_FOOTPRINT &footprint)
{
	u64 alignment = 0;
	switch (type)
	{
	case UploadManager::Buffer:
		alignment = 256;
		break;
	case UploadManager::Texture:
		alignment = 64 KBYTE;
		break;
	default:
		break;
	}

	auto uploadBufferOffset = vx::getAlignedSize(m_uploadBufferSize, alignment);
	auto newSize = uploadBufferOffset + size;
	if (newSize >= m_uploadBufferCapacity)
	{
		return false;
	}

	u8* ptr = nullptr;
	m_uploadBuffer->Map(0, nullptr, (void**)&ptr);

	memcpy(ptr + uploadBufferOffset, src + offset, size);

	D3D12_RANGE writtenRange{ uploadBufferOffset, newSize };
	m_uploadBuffer->Unmap(0, &writtenRange);

	m_uploadBufferSize = newSize;

	CopyTask copyTask{};
	copyTask.type = type;
	copyTask.stateBefore = state;
	copyTask.dst = dst;
	copyTask.dstOffset = dstOffset;
	copyTask.srcOffset = uploadBufferOffset;
	copyTask.size = size;
	copyTask.footprint = footprint;

	m_copyTasks.push_back(copyTask);

	return true;
}

bool UploadManager::pushUploadBuffer(const u8* src, u64 size, u64 offset, ID3D12Resource* dst, u64 dstOffset, u32 state)
{
	D3D12_SUBRESOURCE_FOOTPRINT footprint{};
	if(!tryUpload(src, size, offset, dst, dstOffset, state, TaskType::Buffer, footprint))
	{
		UploadTask uploadTask{};
		uploadTask.type = TaskType::Buffer;
		uploadTask.state = state;
		uploadTask.src = src;
		uploadTask.size = size;
		uploadTask.srcOffset = offset;
		uploadTask.dst = dst;
		uploadTask.dstOffset = dstOffset;
		uploadTask.footprint = footprint;

		m_uploadTasks.push(uploadTask);

		return false;
	}

	return true;
}

bool UploadManager::pushUploadTexture(const u8* src, u64 srcSize, u64 srcOffset, ID3D12Resource* dst, u32 subresourceIndex, const D3D12_SUBRESOURCE_FOOTPRINT &footprint)
{
	if (!tryUpload(src, srcSize, srcOffset, dst, (u64)subresourceIndex, 0, TaskType::Texture, footprint))
	{
		UploadTask uploadTask{};
		uploadTask.subresourceIndex = subresourceIndex;
		uploadTask.type = TaskType::Texture;
		uploadTask.state = 0;
		uploadTask.src = src;
		uploadTask.size = srcSize;
		uploadTask.srcOffset = srcOffset;
		uploadTask.dst = dst;
		uploadTask.dstOffset = 0;
		uploadTask.footprint = footprint;

		m_uploadTasks.push(uploadTask);

		return false;
	}

	return true;
}

void UploadManager::update()
{
	if (m_copyTasks.empty())
	{
		m_uploadTasks.swapBuffers();

		auto sizeBack = m_uploadTasks.sizeBack();
		for (size_t i = 0; i < sizeBack; ++i)
		{
			UploadTask task;
			m_uploadTasks.pop_backBuffer(task);

			if (!tryUpload(task.src, task.size, task.srcOffset, task.dst, task.dstOffset, task.state, task.type, task.footprint))
			{
				m_uploadTasks.push(task);
			}
		}
	}
}

void UploadManager::buildCmdList(u32 frameIndex)
{
	auto currentCmdList = m_cmdList[frameIndex].get();
	m_cmdAllocator[frameIndex]->Reset();
	currentCmdList->Reset(m_cmdAllocator[frameIndex], nullptr);

	auto srcBuffer = m_uploadBuffer.get();
	for (auto &it : m_copyTasks)
	{
		switch (it.type)
		{
		case TaskType::Buffer:
		{
			currentCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, (D3D12_RESOURCE_STATES)it.stateBefore, D3D12_RESOURCE_STATE_COPY_DEST));
			currentCmdList->CopyBufferRegion(it.dst, it.dstOffset, srcBuffer, it.srcOffset, it.size);
			currentCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(it.dst, D3D12_RESOURCE_STATE_COPY_DEST, (D3D12_RESOURCE_STATES)it.stateBefore));
		}break;
		case TaskType::Texture:
		{
			D3D12_TEXTURE_COPY_LOCATION src;
			src.pResource = srcBuffer;
			src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			src.PlacedFootprint.Offset = it.srcOffset;
			src.PlacedFootprint.Footprint = it.footprint;

			D3D12_TEXTURE_COPY_LOCATION dst;
			dst.pResource = it.dst;
			dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dst.SubresourceIndex = it.subresourceIndex;

			currentCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}break;
		default:
			break;
		}
	}

	currentCmdList->Close();

	m_copyTasks.clear();
}

void UploadManager::submit(ID3D12CommandList** list, u32* count, u32 frameIndex)
{
	list[*count] = m_cmdList[frameIndex].get();
	++(*count);
}

void UploadManager::execute(ID3D12CommandQueue* queue, u32 frameIndex)
{
	ID3D12CommandList* list = m_cmdList[frameIndex].get();
	queue->ExecuteCommandLists(1, &list);
}

bool UploadManager::empty() const
{
	return m_copyTasks.empty() && m_uploadTasks.emptyBack() && m_uploadTasks.emptyFront();
}