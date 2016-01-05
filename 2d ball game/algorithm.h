#pragma once

#include <vxLib/algorithm.h>

namespace vx
{
	template<typename T>
	inline void move(T* first, T* last, T* dstFirst)
	{
		auto count = (size_t)(last - first);

		memmove(dstFirst, first, sizeof(T) * count);
	}

	template<typename T, typename K>
	inline T find(T f, T l, const K &key)
	{
		while (f != l)
		{
			if (*f == key)
			{
				break;
			}

			++f;
		}

		return f;
	}

	template<typename T, typename K, typename Cmp>
	inline T find(T f, T l, const K &key, Cmp cmp)
	{
		while (f != l)
		{
			if (cmp(*f, key))
			{
				break;
			}

			++f;
		}

		return f;
	}

	template<typename T, size_t N>
	inline void swap(T(&l)[N], T(&r)[N])
	{
		for (size_t i = 0; i < N; ++i)
		{
			vx::swap(l[i], r[i]);
		}
	}

	struct Loop
	{
		template<typename T, typename Fn>
		void operator()(T count, Fn fn)
		{
			for (T i = 0; i < count; ++i)
			{
				fn(i);
			}
		}
	};
}