#pragma once

#include <vxLib/types.h>
#include <d3d12.h>
#include <type_traits>

namespace d3d
{
	template<typename T>
	class Object
	{
	protected:
		T* m_ptr;

		template<typename U>
		struct CreateImpl;

		template<>
		struct CreateImpl<ID3D12Device>
		{
			bool operator()(ID3D12Device** ptr)
			{
				auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(ptr));
				return (hr == 0);
			}
		};

		template<>
		struct CreateImpl<ID3D12Debug>
		{
			bool operator()(ID3D12Debug** ptr)
			{
				auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(ptr));
				return (hr == 0);
			}
		};

	public:
		Object() :m_ptr(nullptr) {}
		Object(const Object&) = delete;
		Object(Object &&rhs) :m_ptr(rhs.m_ptr) { rhs.m_ptr = nullptr; }
		~Object() { release(); }

		Object& operator=(const Object&) = delete;

		Object& operator=(Object &&rhs)
		{
			if (this != &rhs)
			{
				std::swap(m_ptr, rhs.m_ptr);
			}
			return *this;
		}

		T* get() { return m_ptr; }

		void release()
		{
			if (m_ptr)
			{
				m_ptr->Release();
				m_ptr = nullptr;
			}
		}

		operator T*()
		{
			return m_ptr;
		}

		T* operator->()
		{
			return m_ptr;
		}

		T** getAddressOf()
		{
			return &m_ptr;
		}

		inline bool create()
		{
			return CreateImpl<T>()(&m_ptr);
		}

		template<typename = typename std::enable_if_t<std::is_same<T, ID3D12CommandQueue>::value>*>
		bool create(const D3D12_COMMAND_QUEUE_DESC &desc, ID3D12Device * device)
		{
			auto hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_ptr));
			return (hr == 0);
		}

		template<typename = typename std::enable_if_t<std::is_same<T, ID3D12Heap>::value>*>
		bool create(const D3D12_HEAP_DESC &desc, ID3D12Device * device)
		{
			auto hr = device->CreateHeap(&desc, IID_PPV_ARGS(&m_ptr));
			return (hr == 0);
		}

		template<typename = typename std::enable_if_t<std::is_same<T, ID3D12CommandAllocator>::value>*>
		bool create(D3D12_COMMAND_LIST_TYPE type, ID3D12Device * device)
		{
			auto hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_ptr));
			return (hr == 0);
		}

		template<typename = typename std::enable_if_t<std::is_same<T, ID3D12GraphicsCommandList>::value>*>
		bool create(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* allocator, ID3D12Device * device, ID3D12PipelineState* pipelineState = nullptr, u32 nodeMask = 0)
		{
			auto hr = device->CreateCommandList(nodeMask, type, allocator, pipelineState, IID_PPV_ARGS(&m_ptr));
			return (hr == 0);
		}

		template<typename = typename std::enable_if_t<std::is_same<T, ID3D12Resource>::value>*>
		bool create(ID3D12Heap* heap, u64 offset, const D3D12_RESOURCE_DESC &desc, D3D12_RESOURCE_STATES initialState, ID3D12Device * device, const D3D12_CLEAR_VALUE* clearValue = nullptr)
		{
			auto hr = device->CreatePlacedResource(heap, offset, &desc, initialState, clearValue, IID_PPV_ARGS(&m_ptr));
			return (hr == 0);
		}
	};

	typedef Object<ID3D12CommandQueue> CommandQueue;
	typedef Object<ID3D12Device> Device;
	typedef Object<ID3D12PipelineState> PipelineState;
	typedef Object<ID3D12RootSignature> RootSignature;
	typedef Object<ID3D12CommandAllocator> CommandAllocator;
	typedef Object<ID3D12GraphicsCommandList> GraphicsCommandList;
	typedef Object<ID3D10Blob> Blob;
	typedef Object<ID3D12Heap> Heap;
	typedef Object<ID3D12Debug> Debug;
	typedef Object<ID3D12Resource> Resource;
	typedef Object<ID3D12Fence> Fence;
}