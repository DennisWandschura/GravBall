#pragma once

#include <vxLib/Allocator/Allocator.h>

template<typename Large, typename Small, size_t SMALL_LIMIT>
class Segregator : public Large, public Small
{
public:
	Segregator() :Large(), Small() {}

	template<typename Arg0, typename Arg1>
	Segregator(Arg0 &&arg0, Arg1 &&arg1) : Large(std::forward<Args0>(arg0)), Small(std::forward<Arg1>(arg1)) {}

	template<typename ...Args>
	void initialize(Args&& ...args)
	{
		Large::initialize(std::forward<Args>(args)...);
		Small::initialize(std::forward<Args>(args)...);
	}

	vx::AllocatedBlock allocate(size_t size, size_t alignment)
	{
		if (size <= SMALL_LIMIT)
			return Small::allocate(size, alignment);

		return Large::allocate(size, alignment);
	}

	u32 deallocate(const vx::AllocatedBlock &block)
	{
		if (block.size <= SMALL_LIMIT)
		{
			return Small::deallocate(block);
		}
		
		return Large::deallocate(block);
	}

	bool contains(const vx::AllocatedBlock &block) const
	{
		return Large::contains(block) || Small::contains(block);
	}

	void release()
	{
		Large::release();
		Small::release();
	}

	void print() const
	{
		Large::print();
		Small::print();
	}
};