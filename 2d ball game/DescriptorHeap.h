#pragma once

#include "D3DObject.h"

namespace d3d
{
	struct CpuDescriptorHandle : public D3D12_CPU_DESCRIPTOR_HANDLE
	{
		u32 incSize;

		CpuDescriptorHandle() {}

		CpuDescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle, u32 incSize) { ptr = handle.ptr; this->incSize = incSize; }

		void inc()
		{
			ptr += incSize;
		}

		void inc(u32 v)
		{
			ptr += incSize * v;
		}
	};

	struct GpuDescriptorHandle : public D3D12_GPU_DESCRIPTOR_HANDLE
	{
		u32 incSize;

		GpuDescriptorHandle() {}

		GpuDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle, u32 incSize) { ptr = handle.ptr; this->incSize = incSize; }

		void inc()
		{
			ptr += incSize;
		}

		void inc(u32 v)
		{
			ptr += incSize * v;
		}
	};

	class DescriptorHeap : public Object<ID3D12DescriptorHeap>
	{
		typedef Object<ID3D12DescriptorHeap> Super;

		u32 m_incSize;

	public:
		DescriptorHeap():Super(), m_incSize(){}
		~DescriptorHeap() {}

		bool create(const D3D12_DESCRIPTOR_HEAP_DESC &desc, ID3D12Device* device)
		{
			auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_ptr));
			if (hr != 0)
				return false;

			m_incSize = device->GetDescriptorHandleIncrementSize(desc.Type);

			return true;
		}

		CpuDescriptorHandle getCpuHandle() const { return{m_ptr->GetCPUDescriptorHandleForHeapStart(), m_incSize }; }

		GpuDescriptorHandle getGpuHandle() const { return{ m_ptr->GetGPUDescriptorHandleForHeapStart(), m_incSize }; }
	};
}