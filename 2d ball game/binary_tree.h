#pragma once

#include <vxLib/algorithm.h>

template<typename KeyType, typename T, typename Allocator, typename Cmp>
class binary_tree
{
	template<typename Key, typename T, typename Cmp>
	struct TreeNode
	{
		std::pair<Key, T*> m_data;
		TreeNode* m_children[2];
		size_t m_blockSize;

		TreeNode() :m_key(), m_data(nullptr), m_children(), m_blockSize(0) {}
		TreeNode(const std::pair<Key, T*> &data, size_t blockSize) :m_data(data), m_children(), m_blockSize(blockSize) {}
		~TreeNode() {}

		std::pair<Key, T*>* find(const Key &key)
		{
			if (key == m_data.first)
				return &m_data;

			std::pair<Key, T*>* node = nullptr;
			if (m_children[0] != nullptr && Cmp()(key, m_data.first))
			{
				node = m_children[0]->find(key);
			}
			else if (m_children[1] != nullptr)
			{
				node = m_children[1]->find(key);
			}

			return node;
		}
	};

	typedef TreeNode<KeyType, T, Cmp> MyNode;

	MyNode* m_root;
	size_t m_size;
	Allocator* m_allocator;

	MyNode* createNode(const std::pair<KeyType, T*> &data)
	{
		auto nodeBlock = m_allocator->allocate(sizeof(MyNode), __alignof(MyNode));
		MyNode* ptr = (MyNode*)nodeBlock.ptr;

		new (ptr) MyNode{ data, nodeBlock.size };
		ptr->m_blockSize = nodeBlock.size;

		return ptr;
	}

	void deleteNode(MyNode* p)
	{
		if (p)
		{
			deleteNode(p->m_children[0]);
			deleteNode(p->m_children[1]);

			vx::AllocatedBlock block = { (u8*)p, p->m_blockSize };
			p->~MyNode();
			m_allocator->deallocate(block);
			--m_size;
		}
	}

	void createImpl(std::pair<KeyType, T*>* first, std::pair<KeyType, T*>* last, MyNode** parent)
	{
		auto n = last - first;
		if (n == 0)
			return;

		MyNode* newNode = nullptr;

		if (n == 1)
		{
			newNode = createNode(*first);
		}
		else
		{
			auto m = first + (n / 2);

			newNode = createNode(*m);

			createImpl(first, m, &newNode->m_children[0]);
			createImpl(m + 1, last, &newNode->m_children[1]);
		}

		++m_size;
		*parent = newNode;
	}

public:
	binary_tree() :m_root(nullptr), m_allocator(nullptr) {}
	explicit binary_tree(Allocator* alloc) :m_root(nullptr), m_allocator(alloc) {}

	~binary_tree()
	{
		clear();
	}

	void create(std::pair<KeyType, T*>* data, size_t count)
	{
		auto f = data;
		auto l = data + count;

		std::sort(f, l, Cmp());

		auto n = l - f;
		if (n == 0) return;

		auto m = f + (n / 2);

		m_root = createNode(*m);
		++m_size;

		createImpl(f, m, &m_root->m_children[0]);

		createImpl(m + 1, l, &m_root->m_children[1]);
	}

	void clear()
	{
		if (m_allocator && m_root)
		{
			deleteNode(m_root);
			m_root = nullptr;
		}
	}

	std::pair<KeyType, T*>* find(const KeyType &key)
	{
		if (m_root == nullptr)
		{
			return nullptr;
		}

		return m_root->find(key);
	}
};