#pragma once
#if _EDITOR

#include <gamelib/EngineBase.h>

typedef EngineBase* (*CreateEngineProc)(const EngineBaseDesc&);
typedef void(*DestroyEngineProc)(EngineBase*,Allocator::MainAllocator*);

extern "C" __declspec(dllexport) EngineBase* createEngine(const EngineBaseDesc &desc);

extern "C" __declspec(dllexport) void destroyEngine(EngineBase* engine, Allocator::MainAllocator* mainAllocator);
#endif