#pragma once

#include <vxLib/File/File.h>
#include <vxLib/Allocator/Allocator.h>

class Logfile
{
	vx::File m_file;
	vx::AllocatedBlock m_block;
	u32 m_size;

	void flush();
	void copyToBuffer(const char* str, u32 siz);

	template<typename T>
	void appendImpl(T value);

public:
	Logfile();
	~Logfile();

	bool create(const char* filename, const vx::AllocatedBlock &block);
	vx::AllocatedBlock close();

	void append(char c);
	void append(const char* text);
	void append(const char* text, u32 size);

	void append(u32 value);
	void append(s32 value);
	void append(u64 value);
	void append(s64 value);
};