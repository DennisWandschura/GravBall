#pragma once

struct ID3D12CommandQueue;

#include "DescriptorHeap.h"
#include <dxgi1_4.h>

class SwapChain
{
	d3d::CpuDescriptorHandle m_handle;
	d3d::DescriptorHeap m_descriptorHeap;
	IDXGISwapChain3* m_swapChain;

public:
	SwapChain();
	~SwapChain();

	bool create(ID3D12CommandQueue* cmdQueue, void* hwnd, ID3D12Device* device);
	void shutdown();

	void present();

	d3d::CpuDescriptorHandle getCpuDescriptorHandle() const;
	u32 getCurrentBackBufferIndex() const { return m_swapChain->GetCurrentBackBufferIndex(); };
};