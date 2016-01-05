#pragma once

template<typename T>
class ReferenceCounted : public T
{
	typedef T Super;

	int m_refCount;

public:
	ReferenceCounted() :Super(), m_refCount(0) {}
	~ReferenceCounted() {}

	void increment()
	{
		++m_refCount;
	}

	void decrement()
	{
		--m_refCount;
	}
};

template<typename T>
class Resource
{
	ReferenceCounted<T>* m_data;

public:
	explicit Resource(ReferenceCounted<T>* data)
		:m_data(data)
	{
		data->increment();
	}

	~Resource()
	{
		if (m_data)
		{
			m_data->decrement();
			m_data = nullptr;
		}
	}

	T* operator->()
	{
		return m_data;
	}
};