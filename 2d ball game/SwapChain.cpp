#include "SwapChain.h"
#include <d3d12.h>

SwapChain::SwapChain()
	:m_swapChain(nullptr)
{

}

SwapChain::~SwapChain()
{
	shutdown();
}

bool SwapChain::create(ID3D12CommandQueue* cmdQueue, void* hwnd, ID3D12Device* device)
{
	IDXGIFactory4* factory = nullptr;
	auto hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	if (hr != 0)
		return false;

	const u32 bufferCount = 3;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(swapChainDesc));
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.BufferCount = bufferCount;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain1* swapChain = nullptr;
	hr = factory->CreateSwapChainForHwnd(cmdQueue, (HWND)hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
	if (hr != 0)
		return false;

	hr = swapChain->QueryInterface(&m_swapChain);
	if (hr != 0)
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NumDescriptors = bufferCount;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	hr = device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_descriptorHeap.getAddressOf()));
	if (hr != 0)
		return false;

	auto cpuHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (u32 i = 0; i < bufferCount; ++i)
	{
		ID3D12Resource* buffer = nullptr;
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer));

		D3D12_RENDER_TARGET_VIEW_DESC renterTargetViewDesc{};
		renterTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		renterTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		device->CreateRenderTargetView(buffer, &renterTargetViewDesc, cpuHandle);

		cpuHandle.ptr += incSize;
	}

	m_handle = { m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),incSize };

	return true;
}

void SwapChain::shutdown()
{
	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}
}

void SwapChain::present()
{
	DXGI_PRESENT_PARAMETERS params{};
	m_swapChain->Present1(1, 0, &params);
}

d3d::CpuDescriptorHandle SwapChain::getCpuDescriptorHandle() const
{
	return m_handle;
}