#pragma once

#include <vxLib/StringID.h>
#include "dynamic_array.h"
#include <vxLib/Container/array.h>

template<typename K, typename T, size_t BUCKET_COUNT, size_t BUCKET_SIZE, typename Allocator>
class hash_table
{
	typedef K key_type;
	typedef T value_type;
	typedef value_type* pointer;

	struct Entry
	{
		key_type key;
		pointer data;
	};

	struct SubBucket
	{
		Entry m_entries[BUCKET_SIZE];
		size_t m_size;

		bool push(const key_type &key, pointer data)
		{
			if (m_size >= BUCKET_SIZE)
				return false;

			m_entries[m_size].key = key;
			m_entries[m_size].data = data;

			++m_size;

			return true;
		}

		Entry* find(const key_type &key)
		{
			return vx::find(m_entries, m_entries + m_size, key, [](const Entry &lhs, const key_type &key)
			{
				return (lhs.key == key);
			});
		}

		const Entry* find(const key_type &key) const
		{
			return vx::find(m_entries, m_entries + m_size, key, [](const Entry &lhs, const key_type &key)
			{
				return (lhs.key == key);
			});
		}

		Entry* last()
		{
			return m_entries + m_size;
		}

		const Entry* last() const
		{
			return m_entries + m_size;
		}
	};

	class Bucket
	{
		vx::dynamic_array<SubBucket, Allocator> m_subBuckets;
		size_t m_entryCount;

	public:
		explicit Bucket(Allocator* alloc)
			:m_subBuckets(alloc),
			m_entryCount(0)
		{
			m_subBuckets.reserve(1);
		}

		Bucket(const Bucket&) = delete;

		Bucket(Bucket &&rhs)
			:m_subBuckets(std::move(rhs.m_subBuckets)),
			m_entryCount(rhs.m_entryCount)
		{
			rhs.m_entryCount = 0;
		}

		~Bucket()
		{

		}

		void release()
		{
			m_subBuckets.release();
		}

		void push(const key_type &key, value_type* data)
		{
			if (!m_subBuckets.back().push(key, data))
			{
				m_subBuckets.push_back(SubBucket{});
				m_subBuckets.back().push(key, data);
			}

			++m_entryCount;
		}

		pointer find(const key_type &key)
		{
			Entry* e = nullptr;

			for (auto &it : m_subBuckets)
			{
				e = it.find(key);
				if (e != it.last())
				{
					break;
				}
			}

			return (e != nullptr) ? (e->data) : nullptr;
		}

		size_t size() const { return m_entryCount; }
	};

	vx::array<Bucket, Allocator> m_buckets;

	u64 hash0(const key_type &key) const
	{
		return CityHash64((char*)&key, sizeof(key_type)) % BUCKET_COUNT;
	}

	u64 hash1(const key_type &key, u64 value) const
	{
		return CityHash64WithSeed((char*)&key, sizeof(key_type), value) % BUCKET_COUNT;
	}

public:
	explicit hash_table(Allocator* alloc)
		:m_buckets(alloc, BUCKET_COUNT)
	{
		for (size_t i = 0; i < BUCKET_COUNT; ++i)
		{
			m_buckets.push_back(alloc);
		}
	}

	hash_table(const hash_table&) = delete;

	~hash_table()
	{
	}

	void release()
	{
		m_buckets.release();
	}

	void insert(const key_type &key, pointer data)
	{
		auto idx0 = hash0(key);
		auto idx1 = hash1(key, idx0);

		auto &b0 = m_buckets[idx0];
		auto &b1 = m_buckets[idx1];

		if (b0.size() < b1.size())
		{
			b0.push(key, data);
		}
		else
		{
			b1.push(key, data);
		}
	}

	pointer find(const key_type &key)
	{
		auto idx0 = hash0(key);
		auto &b0 = m_buckets[idx0];
		auto p = b0.find(key);

		if (p == nullptr)
		{
			auto idx1 = hash1(key, idx0);
			auto &b1 = m_buckets[idx1];

			p = b1.find(key);
		}

		return p;
	}
};