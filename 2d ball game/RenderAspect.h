#pragma once

class CpuProfiler;
class LevelFile;

#include <gamelib/EventListener.h>
#include "D3DObject.h"
#include "SwapChain.h"
#include "RenderFrameData.h"
#include "MeshManager.h"
#include "TextureArrayManager.h"
#include "UploadManager.h"
#include "ShaderManager.h"
#include "DefaultProgram.h"
#include "GpuResourceManager.h"
#include "TextRenderer.h"
#include "DrawTextProgram.h"

class RenderAspect : public Event::ListenerInterface
{
	enum class UpdateState : u32
	{
		None,
		UpdateData
	};

	d3d::CommandQueue m_cmdQueue;
	UpdateState m_updateStatus;
	u32 m_frameDataIndex;
	SwapChain m_swapChain;
	RenderFrameData* m_currentFrameData;
	RenderFrameData* m_nextFrameData;
	vx::mat4 m_projectionMatrix;
#if _EDITOR
	__m128 m_cameraPosition;
#endif
	vx::mat4 m_viewMatrix;
	DefaultProgram m_defaultProgram;
	DrawTextProgram m_drawTextProgram;
	Graphics::TextRenderer m_textRenderer;
	UploadManager m_uploadManager;
	MeshManager m_meshManager;
	TextureArrayManager m_textureArrayManager;
	Allocator::MainAllocator* m_mainAllocator;
	ShaderManager m_shaderManager;
	d3d::Device m_device;
	d3d::Debug m_debug;
	d3d::Heap m_bufferHeap;
	d3d::Resource m_depthBuffer;
	d3d::DescriptorHeap m_descriptorHeapDepthBuffer;
	d3d::Heap m_rtvHeap;
	Graphics::GpuResourceManager m_resourceManager;
	vx::AllocatedBlock m_frameDataBlock;

	void updateFrameData(CpuProfiler* cpuProfiler);

	void reset();
	void loadLevel(const LevelFile &l);

	void handleEditorEvent(const Event::Event &evt);
	void handleFileEvent(const Event::Event &evt);

	void rebuildDynamicMeshes(DynamicEntity* entities, size_t count);

#if _EDITOR
	template<typename T>
	bool createEntityMesh(T* entity);

	bool createEntityMesh(struct EditorDynamicEntityFile* entity);

	template<typename T>
	void updateEntityMesh(const T& entity);
#endif

public:
	RenderAspect();
	~RenderAspect();

	bool initialize(void* hwnd, const vx::uint2 &resolution, bool debug, Allocator::MainAllocator* allocator, const char(&rootDir)[8], u32 dynamicEntityCapacity, u32 staticEntityCapacity);
	void shutdown(Allocator::MainAllocator* allocator);

	void update(CpuProfiler* cpuProfiler);
	
	void pushFrameData(RenderFrameDynamicData &&data);

	void handleEvent(const Event::Event &evt) override;

	void getViewMatrix(vx::mat4* m) const;
	void getProjectionMatrix(vx::mat4* m) const;
	__m128 getCameraPosition() const;
};