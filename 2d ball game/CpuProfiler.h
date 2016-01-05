#pragma once

namespace Allocator
{
	class MainAllocator;
}

struct CpuProfilerSample;

#include <vxLib/CpuTimer.h>
#include <vxLib/Container/array.h>
#include <vxLib/Allocator/LinearAllocator.h>
#include <vxLib/Allocator/DelegateAllocator.h>
#include <vxLib/File/File.h>
#include <atomic>

struct ThreadWriteToFileTask
{
	std::atomic_int hasData;
	vx::File* file;
	u64 dataSize;
	vx::AllocatedBlock data;
};

class CpuProfilerEntry
{
	vx::CpuTimer m_timer;
	u32 m_layer;

public:
	explicit CpuProfilerEntry(u32 l):m_timer(), m_layer(l){}

	f32 getTimeMiliseconds() const { return m_timer.getTimeMiliseconds(); }
	u32 getLayer() const { return m_layer; }
};

#if _DEV
class CpuProfiler
{
	typedef vx::DelegateAllocator<vx::LinearAllocator> ListAllocator;

	u64 m_frame;
	vx::CpuTimer m_timer;
	u32 m_currentLayer;
	u32 m_padding;
	vx::array<f32, vx::DelegateAllocator<vx::LinearAllocator>> m_frameTimes;
	vx::array<CpuProfilerSample, vx::DelegateAllocator<vx::LinearAllocator>> m_samples;
	vx::File m_outputFile;
	vx::LinearAllocator m_allocator;

	void writeToFileNoSampleNoThread();
	void writeToFileNoSample(ThreadWriteToFileTask* task);
	void writeToFile(ThreadWriteToFileTask* task);

public:
	CpuProfiler();
	~CpuProfiler();

	bool initialize(Allocator::MainAllocator* allocator);
	void shutdown(Allocator::MainAllocator* allocator);

	void frameStart(ThreadWriteToFileTask* task);
	void frameEnd();

	void update(ThreadWriteToFileTask* task);

	CpuProfilerEntry beginSample();
	void endSample(const CpuProfilerEntry &sample, const char* msg);
};
#else
class CpuProfiler
{
public:
	CpuProfiler() {}
	~CpuProfiler(){}

	bool initialize(Allocator::MainAllocator* allocator) {return true;}
	void shutdown(Allocator::MainAllocator* allocator) {}

	void frameStart(ThreadWriteToFileTask* task) {}
	void frameEnd() {}

	void update(ThreadWriteToFileTask* task) {}

	CpuProfilerEntry beginSample() { return CpuProfilerEntry(0); }
	void endSample(const CpuProfilerEntry &sample, const char* msg) {}
};
#endif

#if _DEV
struct CpuSample
{
	template<typename T>
	void operator()(CpuProfiler &profiler, const char* text, T fn)
	{
		auto sample = profiler.beginSample();
		fn();
		profiler.endSample(sample, text);
	}

	template<typename T>
	void operator()(CpuProfiler* profiler, const char* text, T fn)
	{
		auto sample = profiler->beginSample();
		fn();
		profiler->endSample(sample, text);
	}
};
#else
struct CpuSample
{
	template<typename T>
	void operator()(CpuProfiler &profiler, const char* text, T fn) { fn(); }

	template<typename T>
	void operator()(CpuProfiler* profiler, const char* text, T fn) { fn(); }
};
#endif