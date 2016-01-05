/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <gamelib/Logfile.h>
#include <cstring>
#include <vxLib/print.h>

Logfile::Logfile()
	:m_file(),
	m_block(),
	m_size(0)
{

}

Logfile::~Logfile()
{
	close();
}

bool Logfile::create(const char* filename, const vx::AllocatedBlock &block)
{
	bool result = true;

	if (!m_file.isOpen())
	{
		result = m_file.create(filename, vx::FileAccess::Write);

		m_block = block;
		m_size = 0;
	}

	return result;
}

vx::AllocatedBlock Logfile::close()
{
	if (m_file.isOpen())
	{
		if (m_size != 0)
		{
			flush();
		}
		m_file.close();
	}
	vx::AllocatedBlock block = m_block;

	m_block.ptr = nullptr;
	m_block.size = 0;

	return block;
}

void Logfile::flush()
{
	m_file.write(m_block.ptr, m_size);
	memset(m_block.ptr, '\0', m_size);
	m_size = 0;
}

void Logfile::copyToBuffer(const char* str, u32 size)
{
	memcpy(m_block.ptr + m_size, str, size);
	m_size += size;
	m_block.ptr[m_size] = '\0';
}

void Logfile::append(char c)
{
	if ((m_size + 1) >= m_block.size)
	{
		flush();
	}

	copyToBuffer(&c, 1);
}

void Logfile::append(const char* text)
{
	auto sz = strlen(text);
	append(text, sz);
}

void Logfile::append(const char* text, u32 size)
{
	if (size == 0)
		return;

	if((m_size + size) >= m_block.size)
	{
		flush();
	}

	copyToBuffer(text, size);
}

template<typename T>
void Logfile::appendImpl(T value)
{
	int remainingSize = static_cast<int>(m_block.size - m_size);
	int size = 0;
	char* ptr = (char*)(m_block.ptr + m_size);

	vx::detail::print::ConvertImpl<T>()(&ptr, &remainingSize, &size, value);

	m_size += size;
	m_block.ptr[m_size] = '\0';
}

void Logfile::append(u32 value)
{
	appendImpl(value);
}

void Logfile::append(s32 value)
{
	appendImpl(value);
}

void Logfile::append(u64 value)
{
	appendImpl(value);
}

void Logfile::append(s64 value)
{
	appendImpl(value);
}