#pragma once

#include <vxLib/types.h>
#include "algorithm.h"

namespace vx
{
	template<typename T, typename Allocator>
	class dynamic_array
	{
		typedef T value_type;
		typedef value_type* pointer;

		pointer m_begin;
		pointer m_end;
		pointer m_last;
		Allocator* m_allocator;
		size_t m_allocatedSize;

		void reserveImpl(size_t capacity, size_t size)
		{
			auto block = m_allocator->allocate(sizeof(value_type) * capacity, __alignof(value_type));

			auto newData = (pointer)block.ptr;

			vx::move(m_begin, m_end, newData);

			m_allocator->deallocate({ (u8*)m_begin, m_allocatedSize });

			m_begin = newData;
			m_end = newData + size;
			m_last = newData + capacity;
			m_allocatedSize = block.size;
		}

	public:
		dynamic_array() :m_begin(nullptr), m_end(nullptr), m_last(nullptr), m_allocator(nullptr), m_allocatedSize(0) {}

		explicit dynamic_array(Allocator* alloc) :m_begin(nullptr), m_end(nullptr), m_last(nullptr), m_allocator(alloc), m_allocatedSize(0) {}

		dynamic_array(const dynamic_array&) = delete;

		dynamic_array(dynamic_array &&rhs)
			:m_begin(rhs.m_begin),
			m_end(rhs.m_end),
			m_last(rhs.m_last),
			m_allocator(rhs.m_allocator),
			m_allocatedSize(rhs.m_allocatedSize)
		{
			rhs.m_begin = rhs.m_end = rhs.m_last = rhs.m_allocator = nullptr;
			rhs.m_allocatedSize = 0;
		}

		~dynamic_array()
		{
			release();
			m_allocator = nullptr;
		}

		dynamic_array& operator=(const dynamic_array&) = delete;

		dynamic_array& operator=(dynamic_array &&rhs)
		{
			if (this != &rhs)
			{
				swap(rhs);
			}
			return *this;
		}

		void swap(dynamic_array &other)
		{
			vx::swap(m_begin, other.m_begin);
			vx::swap(m_end, other.m_end);
			vx::swap(m_last, other.m_last);
			vx::swap(m_allocator, other.m_allocator);
			vx::swap(m_allocatedSize, other.m_allocatedSize);
		}

		template<typename ...Args>
		void push_back(Args&& ...args)
		{
			auto sz = size();
			auto cp = capacity();
			if (sz >= cp)
			{
				reserveImpl(size_t(cp * 1.5f), sz);
			}

			new (&m_begin[sz]) value_type{ std::forward<Args>(args)... };
			++m_end;
		}

		void pop_back()
		{
			auto newEnd = m_end - 1;
			newEnd->~value_type();
			m_end = newEnd;
		}

		void reserve(size_t count)
		{
			if (count <= capacity())
			{
				return;
			}

			reserveImpl(count, size());
		}

		void clear()
		{
			auto ptr = begin();
			auto e = end();

			while (ptr != e)
			{
				ptr->~value_type();
			}

			m_end = m_begin;
		}

		void release()
		{
			if (m_allocator)
			{
				clear();

				m_allocator->deallocate({ (u8*)m_begin, m_allocatedSize });

				m_begin = m_end = m_last = nullptr;
				m_allocatedSize = 0;
			}
		}

		value_type& front()
		{
			return *m_begin
		}

		const value_type& front() const
		{
			return *m_begin
		}

		value_type& back()
		{
			return *(m_end - 1);
		}

		const value_type& back() const
		{
			return *(m_end - 1);
		}

		pointer begin()
		{
			return m_begin;
		}

		const pointer begin() const
		{
			return m_begin;
		}

		pointer end()
		{
			return m_end;
		}

		const pointer end() const
		{
			return m_end;
		}

		size_t size() const
		{
			return m_end - m_begin;
		}

		size_t capacity() const
		{
			return m_last - m_begin;
		}

		bool empty() const
		{
			return (m_end == m_begin);
		}
	};

	template<typename T, typename Allocator>
	void swap(dynamic_array<T, Allocator> &l, dynamic_array<T, Allocator> &r)
	{
		l.swap(r);
	}
}