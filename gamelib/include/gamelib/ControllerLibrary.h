#pragma once

#include <gamelib/InputController.h>
#include <vxLib/StringID.h>
#include <vxLib/Container/sorted_array.h>
#include <gamelib/MainAllocator.h>
#include <gamelib/DebugAllocator.h>

class ControllerLibrary
{
	vx::sorted_array<vx::StringID, CreateInputControllerFn, DebugAllocatorGeneral<Allocator::MainAllocator>> m_functions;

public:
	ControllerLibrary()
	{

	}

	~ControllerLibrary() {}

	void initialize(u32 capacity, Allocator::MainAllocator* allocator)
	{
		m_functions = vx::sorted_array<vx::StringID, CreateInputControllerFn, DebugAllocatorGeneral<Allocator::MainAllocator>>(allocator, capacity);
	}

	void release()
	{
		m_functions.release();
	}

	void registerFunction(const vx::StringID &sid, CreateInputControllerFn fn)
	{
		m_functions.insert(sid, fn);
	}

	InputController* createController(const vx::StringID &sid, void* p)
	{
		auto it = m_functions.find(sid);
		if (it != m_functions.end())
		{
			return (*it)(p);
		}

		return nullptr;
	}
};