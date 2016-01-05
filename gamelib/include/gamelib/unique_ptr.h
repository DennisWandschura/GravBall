#pragma once

#include <vxLib/Allocator/Allocator.h>
#include <algorithm>

namespace vx
{
	template<typename T>
	class unique_ptr
	{
		T* m_ptr;
		size_t m_allocatedSize;

	public:
		typedef T* pointer;
		typedef unique_ptr<T> MyType;

		unique_ptr() :m_ptr(nullptr), m_allocatedSize(0) {}
		unique_ptr(pointer ptr, size_t allocatedSize) :m_ptr(ptr), m_allocatedSize(allocatedSize) {}

		template<typename U>
		unique_ptr(U* ptr, size_t allocatedSize) : m_ptr(ptr), m_allocatedSize(allocatedSize) {}

		unique_ptr(const unique_ptr&) = delete;

		unique_ptr(unique_ptr &&rhs)
			:m_ptr(rhs.m_ptr), m_allocatedSize(rhs.m_allocatedSize)
		{
			rhs.m_ptr = nullptr;
			rhs.m_allocatedSize = 0;
		}

		~unique_ptr() {}

		unique_ptr& operator=(const unique_ptr&) = delete;

		unique_ptr& operator=(unique_ptr &&rhs)
		{
			if (this != &rhs)
			{
				std::swap(m_ptr, rhs.m_ptr);
				std::swap(m_allocatedSize, rhs.m_allocatedSize);
			}
			return *this;
		}

		T& operator*()
		{
			return *m_ptr;
		}

		const T& operator*() const
		{
			return *m_ptr;
		}

		T* operator->()
		{
			return m_ptr;
		}

		const T* operator->() const
		{
			return m_ptr;
		}

		T& operator[](size_t i) = delete;
		const T& operator[](size_t i) const = delete;

		pointer release(size_t* allocatedSize)
		{
			*allocatedSize = m_allocatedSize;
			auto p = m_ptr;

			m_ptr = nullptr;
			m_allocatedSize = 0;

			return p;
		}
	};

	template<typename T>
	class unique_ptr<T[]>
	{
	};

	template<typename T, typename Allocator, typename ...Args>
	unique_ptr<T> make_unique(Allocator* allocator, Args&& ...args)
	{
		auto block = allocator->allocate(sizeof(T), __alignof(T));
		if (block.ptr != nullptr)
		{
			new (block.ptr) T{ std::forward<Args>(args)... };
			return unique_ptr<T>{ (T*)block.ptr, block.size };
		}

		return unique_ptr<T>{};
	}
}