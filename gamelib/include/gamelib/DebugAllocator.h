#pragma once

#include <vxLib/Allocator/Allocator.h>
#include <gamelib/AllocatorDebugInfo.h>

template<typename Super, typename System>
class DebugAllocator
{
	Super* m_ptr;

public:
	DebugAllocator() :m_ptr(nullptr) {}
	explicit DebugAllocator(Super* ptr) :m_ptr(ptr) { }
	DebugAllocator(const DebugAllocator&) = delete;
	DebugAllocator(DebugAllocator &&rhs)
		:m_ptr(rhs.m_ptr)
	{
		rhs.m_ptr = nullptr;
	}

	DebugAllocator& operator=(const DebugAllocator &) = delete;

	DebugAllocator& operator=(DebugAllocator &&rhs)
	{
		if (this != &rhs)
		{
			swap(rhs);
		}
		return *this;
	}

	vx::AllocatedBlock allocate(size_t size, size_t alignment)
	{
		return m_ptr->allocate(size, alignment, System::Channel, System::msg);
	}

	u32 deallocate(const vx::AllocatedBlock &block)
	{
		return m_ptr->deallocate(block, System::Channel, System::msg);
	}

	void swap(DebugAllocator &other)
	{
		auto tmp = m_ptr;
		m_ptr = other.m_ptr;
		other.m_ptr = tmp;
	}
};

struct DebugAllocatorSystemRenderer
{
	static const Allocator::Channels Channel{Allocator::Channels::Renderer};
	static const char* msg;
};

struct DebugAllocatorSystemEvent
{
	static const Allocator::Channels Channel{ Allocator::Channels::Events };
	static const char* msg;
};

struct DebugAllocatorSystemPhysics
{
	static const Allocator::Channels Channel{ Allocator::Channels::Physics };
	static const char* msg;
};

struct DebugAllocatorSystemEntity
{
	static const Allocator::Channels Channel{ Allocator::Channels::Entity };
	static const char* msg;
};

struct DebugAllocatorSystemResource
{
	static const Allocator::Channels Channel{ Allocator::Channels::Resource };
	static const char* msg;
};

struct DebugAllocatorSystemGeneral
{
	static const Allocator::Channels Channel{ Allocator::Channels::General };
	static const char* msg;
};

template<typename Super>
using DebugAllocatorRenderer = DebugAllocator<Super, DebugAllocatorSystemRenderer>;

template<typename Super>
using DebugAllocatorEvent = DebugAllocator<Super, DebugAllocatorSystemEvent>;

template<typename Super>
using DebugAllocatorPhysics= DebugAllocator<Super, DebugAllocatorSystemPhysics>;

template<typename Super>
using DebugAllocatorEntity = DebugAllocator<Super, DebugAllocatorSystemEntity>;

template<typename Super>
using DebugAllocatorResource = DebugAllocator<Super, DebugAllocatorSystemResource>;

template<typename Super>
using DebugAllocatorGeneral= DebugAllocator<Super, DebugAllocatorSystemGeneral>;