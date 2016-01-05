#include "CpuProfiler.h"
#include <gamelib/MainAllocator.h>
#include <vxLib/File/File.h>
#include <gamelib/CpuProfilerSample.h>

namespace CpuProfilerCpp
{
	static const u32 CAPACITY = 1024;
	static const u32 WRITE_SIZE = 512;

	template<typename List>
	void writeListToFile(vx::File &f, List &list)
	{
		u64 frameCount = list.size();
		f.write(frameCount);

		while (!list.empty())
		{
			f.write(list.front());
			list.pop_front();
		}
	}
}

#if _DEV
CpuProfiler::CpuProfiler()
	:m_frame(),
	m_timer(),
	m_currentLayer(0),
	m_padding(0),
	m_frameTimes(),
	m_samples(),
	m_allocator(),
	m_outputFile()
{

}

CpuProfiler::~CpuProfiler()
{
}

bool CpuProfiler::initialize(Allocator::MainAllocator* allocator)
{
	const auto requiredMemory = sizeof(f32) * CpuProfilerCpp::CAPACITY + sizeof(CpuProfilerSample) * CpuProfilerCpp::CAPACITY;
	auto block = allocator->allocate(requiredMemory, 16, Allocator::Channels::General, "cpu profiler");
	if (block.size == 0)
		return false;

	m_allocator.initialize(block);

	m_frameTimes = vx::array<f32, vx::DelegateAllocator<vx::LinearAllocator>>(&m_allocator, CpuProfilerCpp::CAPACITY);
	m_samples = vx::array<CpuProfilerSample, vx::DelegateAllocator<vx::LinearAllocator>>(&m_allocator, CpuProfilerCpp::CAPACITY);

	m_outputFile.create("profiler.info", vx::FileAccess::Write);

	return true;
}

void CpuProfiler::shutdown(Allocator::MainAllocator* allocator)
{
	writeToFileNoSampleNoThread();
	m_outputFile.close();

	m_samples.clear();
	m_frameTimes.clear();
	allocator->deallocate(m_allocator.release(), Allocator::Channels::General, "cpu profiler");
}

void CpuProfiler::writeToFileNoSampleNoThread()
{
	auto frameTimeCount = m_frameTimes.size();
	auto sampleCount = m_samples.size();

	CpuProfilerFileHeader header;
	header.frameTimeCount = frameTimeCount;
	header.sampleCount = sampleCount;
	m_outputFile.write(header);

	m_outputFile.write((u8*)m_frameTimes.data(), frameTimeCount * sizeof(f32));

	m_outputFile.write((u8*)m_samples.data(), sampleCount * sizeof(CpuProfilerSample));

	m_outputFile.flush();

	m_frameTimes.clear();
	m_samples.clear();
}

void CpuProfiler::writeToFileNoSample(ThreadWriteToFileTask* task)
{
	auto frameTimeCount = m_frameTimes.size();
	auto sampleCount = m_samples.size();

	CpuProfilerFileHeader header;
	header.frameTimeCount = frameTimeCount;
	header.sampleCount = sampleCount;

	const auto totalSize = sizeof(header) + frameTimeCount * sizeof(f32) + sampleCount * sizeof(CpuProfilerSample);
	VX_ASSERT(totalSize <= task->data.size);

	auto p = task->data.ptr;

	memcpy(p, &header, sizeof(header));
	p += sizeof(header);

	memcpy(p, m_frameTimes.data(), frameTimeCount * sizeof(f32));
	p += frameTimeCount * sizeof(f32);

	memcpy(p, m_samples.data(), sampleCount * sizeof(CpuProfilerSample));
	p += sampleCount * sizeof(CpuProfilerSample);

	task->dataSize = totalSize;
	task->file = &m_outputFile;
	task->hasData.store(1, std::memory_order_release);

	m_frameTimes.clear();
	m_samples.clear();
}

void CpuProfiler::writeToFile(ThreadWriteToFileTask* task)
{
	auto bg = beginSample();

	writeToFileNoSample(task);

	endSample(bg, "write cpu profiler");
}

void CpuProfiler::frameStart(ThreadWriteToFileTask* task)
{
	m_currentLayer = 0;
	m_timer.reset();

	update(task);
}

void CpuProfiler::frameEnd()
{
	++m_frame;

	auto time = m_timer.getTimeSeconds();

	auto b = m_frameTimes.push_back(time);
	VX_ASSERT(b);
}

void CpuProfiler::update(ThreadWriteToFileTask* task)
{
	if (m_frameTimes.size() >= CpuProfilerCpp::WRITE_SIZE || m_samples.size() >= CpuProfilerCpp::WRITE_SIZE)
	{
		writeToFile(task);
	}
}

CpuProfilerEntry CpuProfiler::beginSample()
{
	return CpuProfilerEntry(m_currentLayer++);
}

void CpuProfiler::endSample(const CpuProfilerEntry &sample, const char* msg)
{
	--m_currentLayer;

	auto len = strlen(msg);
	len = (len < 64) ? len : 63;

	CpuProfilerSample entry;
	entry.frameIndex = m_frame;
	entry.time = sample.getTimeMiliseconds();
	entry.layer = sample.getLayer();
	memcpy(entry.msg, msg, len);
	entry.msg[len] = '\0';

	auto sz = m_samples.size();
	auto b = m_samples.push_back(entry);
	VX_ASSERT(b);
}
#endif