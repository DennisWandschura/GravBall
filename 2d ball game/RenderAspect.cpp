#include "RenderAspect.h"
#include <gamelib/Event.h>
#include <d3d12sdklayers.h>
#include <gamelib/LevelFile.h>
#include "Texture.h"
#include "ResourceAspect.h"
#include <gamelib/MainAllocator.h>
#include "Locator.h"
#include "CpuProfiler.h"
#include <vxLib/print.h>
#include "EventManager.h"
#include "GpuCameraBuffer.h"
#include <gamelib/Entity.h>
#include <gamelib/GravityArea.h>
#include <gamelib/GravityWell.h>

RenderAspect::RenderAspect()
	:m_cmdQueue(),
	m_updateStatus(UpdateState::None),
	m_swapChain(),
	m_textRenderer(),
	m_currentFrameData(nullptr),
	m_nextFrameData(nullptr),
	m_device(),
	m_debug()
{

}

RenderAspect::~RenderAspect()
{

}

bool RenderAspect::initialize(void* hwnd, const vx::uint2 &resolution, bool debug, Allocator::MainAllocator* allocator, const char(&rootDir)[8], u32 dynamicEntityCapacity, u32 staticEntityCapacity)
{
	vx::printf("\nRenderAspect:\n");

	if (debug)
	{
		if (!m_debug.create())
		{
			vx::printf("	Error creating Debug\n");
			//return false;
		}
		else
		{
			m_debug->EnableDebugLayer();
		}
	}

	if (!m_device.create())
	{
		vx::printf("	Error creating Device\n");
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc;
	memset(&cmdQueueDesc, 0, sizeof(cmdQueueDesc));
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (!m_cmdQueue.create(cmdQueueDesc, m_device))
	{
		vx::printf("	Error creating Command Queue\n");
		return false;
	}

	if (!m_swapChain.create(m_cmdQueue, hwnd, m_device))
	{
		vx::printf("	Error creating Swap Chain\n");
		return false;
	}

	Graphics::GpuResourceManagerDesc resourceManagerDesc{};
	resourceManagerDesc.allocator = allocator;
	resourceManagerDesc.bufferHeapSize = 1 MBYTE;
	resourceManagerDesc.bufferResourceCount = 16;
	resourceManagerDesc.device = m_device;
	resourceManagerDesc.rtdsHeapSize = 64 KBYTE;
	resourceManagerDesc.rtdsResourceCount = 1;
	resourceManagerDesc.textureHeapSize = 64 MBYTE;
	resourceManagerDesc.textureResourceCount = 32;
	if (!m_resourceManager.initialize(resourceManagerDesc))
	{
		vx::printf("	Error creating Resource Manager\n");
		return false;
	}

	if (!m_uploadManager.initialize(m_device, allocator))
	{
		vx::printf("	Error creating Upload Manager\n");
		return false;
	}

	if (!m_meshManager.initialize(m_device, 1024, 1024, dynamicEntityCapacity, staticEntityCapacity, allocator))
	{
		vx::printf("	Error creating Mesh Manager\n");
		return false;
	}

	if (!m_textureArrayManager.initialize(m_device, 512, 32, allocator, &m_resourceManager))
	{
		vx::printf("	Error creating Texture Array Manager\n");
		return false;
	}

	if (!m_shaderManager.initialize(8, allocator))
	{
		vx::printf("	Error creating Shader Manager\n");
		return false;
	}

	//if (!m_textRenderer.initialize(2048, allocator, &m_uploadManager, m_device, &m_resourceManager, &m_drawTextProgram))
	{
		//	return false;
	}

	m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::degToRad(90.0f), (f32)resolution.x / (f32)resolution.y, 0.1f, 100.0f);

	auto heapSize = RenderFrameData::getAllocSize(dynamicEntityCapacity + staticEntityCapacity, 2);

	D3D12_HEAP_DESC heapDesc{};
	heapDesc.SizeInBytes = heapSize;
	heapDesc.Alignment = 64 KBYTE;
	heapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	if (!m_bufferHeap.create(heapDesc, m_device))
	{
		vx::printf("	Error creating Heap\n");
		return false;
	}

	D3D12_RESOURCE_DESC rtvResDesc[2];
	memset(rtvResDesc, 0, sizeof(rtvResDesc));
	rtvResDesc[0].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvResDesc[0].Alignment = 64 KBYTE;
	rtvResDesc[0].Width = resolution.x;
	rtvResDesc[0].Height = resolution.y;
	rtvResDesc[0].DepthOrArraySize = 2;
	rtvResDesc[0].Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	rtvResDesc[0].SampleDesc.Count = 1;
	rtvResDesc[0].MipLevels = 1;
	rtvResDesc[0].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvResDesc[0].Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	rtvResDesc[1].Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvResDesc[1].Alignment = 64 KBYTE;
	rtvResDesc[1].Width = resolution.x;
	rtvResDesc[1].Height = resolution.y;
	rtvResDesc[1].DepthOrArraySize = 2;
	rtvResDesc[1].Format = DXGI_FORMAT_D32_FLOAT;
	rtvResDesc[1].SampleDesc.Count = 1;
	rtvResDesc[1].MipLevels = 1;
	rtvResDesc[1].Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvResDesc[1].Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	auto allocInfoRT = m_device->GetResourceAllocationInfo(0, _countof(rtvResDesc), rtvResDesc);

	D3D12_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.SizeInBytes = allocInfoRT.SizeInBytes * 2;
	rtvHeapDesc.Alignment = allocInfoRT.Alignment;
	rtvHeapDesc.Properties.Type = D3D12_HEAP_TYPE_DEFAULT;
	rtvHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
	if (!m_rtvHeap.create(rtvHeapDesc, m_device))
	{
		vx::printf("	Error creating Heap\n");
		return false;
	}

	D3D12_CLEAR_VALUE depthClearValue;
	depthClearValue.DepthStencil.Depth = 1;
	depthClearValue.DepthStencil.Stencil = 0;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	D3D12_RESOURCE_DESC depthBufferDesc{};
	depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthBufferDesc.Alignment = 64 KBYTE;
	depthBufferDesc.Width = resolution.x;
	depthBufferDesc.Height = resolution.y;
	depthBufferDesc.DepthOrArraySize = 2;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	if (!m_depthBuffer.create(m_rtvHeap, 0, depthBufferDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, m_device, &depthClearValue))
	{
		vx::printf("	Error creating Resource\n");
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc{};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	if (!m_descriptorHeapDepthBuffer.create(descHeapDesc, m_device))
	{
		vx::printf("	Error creating Descriptor Heap\n");
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.ArraySize = 1;
	dsvDesc.Texture2DArray.FirstArraySlice = 0;
	dsvDesc.Texture2DArray.MipSlice = 0;

	auto cpuHandle = m_descriptorHeapDepthBuffer.getCpuHandle();
	m_device->CreateDepthStencilView(m_depthBuffer, &dsvDesc, cpuHandle);
	auto depthHandle0 = cpuHandle;

	dsvDesc.Texture2DArray.FirstArraySlice = 1;
	cpuHandle.inc();
	m_device->CreateDepthStencilView(m_depthBuffer, &dsvDesc, cpuHandle);
	auto depthHandle1 = cpuHandle;

	m_frameDataBlock = allocator->allocate(sizeof(RenderFrameData) * 2, __alignof(RenderFrameData), Allocator::Channels::Renderer, "frame data");
	m_currentFrameData = (RenderFrameData*)m_frameDataBlock.ptr;
	new (m_currentFrameData) RenderFrameData{};
	m_nextFrameData = m_currentFrameData + 1;
	new (m_nextFrameData) RenderFrameData{};

	u64 heapOffset = 0;
	if (!m_currentFrameData->create(resolution, m_bufferHeap, &heapOffset, dynamicEntityCapacity + staticEntityCapacity, m_device, depthHandle0))
	{
		vx::printf("	Error creating Frame Data\n");
		return false;
	}
	if (!m_nextFrameData->create(resolution, m_bufferHeap, &heapOffset, dynamicEntityCapacity + staticEntityCapacity, m_device, depthHandle1))
	{
		vx::printf("	Error creating Frame Data\n");
		return false;
	}

	m_currentFrameData->srgbaTexture = m_textureArrayManager.getSrgbaTextureResource();
	m_nextFrameData->srgbaTexture = m_textureArrayManager.getSrgbaTextureResource();

	char buffer[64];
#if _EDITOR
	snprintf(buffer, sizeof(buffer), "%sdata/shaders/EditorDefaultPs_d.cso", rootDir);
#else
	snprintf(buffer, sizeof(buffer), "%sdata/shaders/DefaultPs_d.cso", rootDir);
#endif
	wchar_t wbuffer[64];
	mbstowcs(wbuffer, buffer, sizeof(buffer) - 1);
	if (!m_shaderManager.loadShader(wbuffer, "defaultps.cso"))
	{
		vx::printf("	Error creating loading Shader\n");
		return false;
	}

	snprintf(buffer, sizeof(buffer), "%sdata/shaders/DefaultVs_d.cso", rootDir);
	mbstowcs(wbuffer, buffer, sizeof(buffer) - 1);
	if (!m_shaderManager.loadShader(wbuffer, "defaultvs.cso"))
	{
		vx::printf("	Error creating loading Shader\n");
		return false;
	}

	if (!m_defaultProgram.initialize(&m_shaderManager, m_device, m_currentFrameData, m_nextFrameData, &m_textureArrayManager))
	{
		vx::printf("	Error creating Default Program\n");
		return false;
	}

	//if (!m_drawTextProgram.initialize(&m_shaderManager, m_device, m_currentFrameData, m_nextFrameData, &m_textureArrayManager))
	//	return false;

	m_updateStatus = UpdateState::None;
	m_frameDataIndex = 0;

	m_mainAllocator = allocator;

	vx::Graphics::Camera camera;
	camera.setPosition(0, 0, 5);

	vx::mat4 viewMatrix;
	camera.getViewMatrixRH(&viewMatrix);
	auto pvMatrix = m_projectionMatrix *viewMatrix;

	m_uploadManager.pushUploadBuffer((u8*)&pvMatrix, sizeof(pvMatrix), 0, m_currentFrameData->staticBuffer, RenderFrameData::CameraBufferOffset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_uploadManager.pushUploadBuffer((u8*)&pvMatrix, sizeof(pvMatrix), 0, m_nextFrameData->staticBuffer, RenderFrameData::CameraBufferOffset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	printf("initialized\n");
	return true;
}

void RenderAspect::shutdown(Allocator::MainAllocator* allocator)
{
	if (m_currentFrameData)
	{
		m_currentFrameData->wait();
	}

	//m_drawTextProgram.shutdown();
	m_defaultProgram.shutdown();

	if (m_nextFrameData)
	{
		m_nextFrameData->shutdown();
		m_nextFrameData = nullptr;
	}
	if (m_currentFrameData)
	{
		m_currentFrameData->shutdown();
		m_currentFrameData = nullptr;
	}

	allocator->deallocate(m_frameDataBlock, Allocator::Channels::Renderer, "frame data");
	m_frameDataBlock = { nullptr, 0 };

	//m_textRenderer.shutdown();
	m_shaderManager.shutdown();
	m_textureArrayManager.shutdown(allocator);
	m_meshManager.shutdown();
	m_uploadManager.shutdown();
	m_resourceManager.shutdown();

	m_swapChain.shutdown();

	m_cmdQueue.release();


	m_device.release();
	m_debug.release();
}

#if _EDITOR
#define SAMPLE_BEGIN(PROFILER) 
#define SAMPLE_END(PROFILER, TEXT)
#else
#define SAMPLE_BEGIN(PROFILER) {auto sample = cpuProfiler->beginSample()
#define SAMPLE_END(PROFILER, TEXT) cpuProfiler->endSample(sample, TEXT);}
#endif

void RenderAspect::updateFrameData(CpuProfiler* cpuProfiler)
{
	auto cameraPosition = m_nextFrameData->dynamicData.cameraPosition;

	vx::Graphics::Camera camera;
	camera.setPosition(cameraPosition.x, cameraPosition.y, 5);

	camera.getViewMatrixRH(&m_viewMatrix);

	SAMPLE_BEGIN(cpuProfiler);
	m_nextFrameData->wait();
	SAMPLE_END(cpuProfiler, "wait");


#if _EDITOR
	m_cameraPosition = camera.getPosition();
#endif
	// update gpu data
	GpuCameraBuffer cameraBuffer;
	cameraBuffer.pvMatrix = m_projectionMatrix * m_viewMatrix;
	cameraBuffer.cameraPosition = camera.getPosition();
	m_nextFrameData->updateDynamicTransforms(&m_uploadManager);
	m_uploadManager.pushUploadBuffer((u8*)&cameraBuffer, sizeof(cameraBuffer), 0, m_nextFrameData->staticBuffer, RenderFrameData::CameraBufferOffset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	//m_nextFrameData->signal(m_cmdQueue);

	m_frameDataIndex = (m_frameDataIndex + 1) % 2;
	std::swap(m_currentFrameData, m_nextFrameData);
	m_updateStatus = UpdateState::None;
}

void RenderAspect::update(CpuProfiler* cpuProfiler)
{
	switch (m_updateStatus)
	{
	case RenderAspect::UpdateState::None:
		break;
	case RenderAspect::UpdateState::UpdateData:
	{
		SAMPLE_BEGIN(cpuProfiler);
		updateFrameData(cpuProfiler);
		SAMPLE_END(cpuProfiler, "update frame data");
	}break;
	/*case RenderAspect::UpdateState::Swap:
	{
		printf("try swap\n");
		if (m_nextFrameData->isReady())
		{
			printf("swap\n");
			m_frameDataIndex = (m_frameDataIndex + 1) % 2;
			std::swap(m_currentFrameData, m_nextFrameData);
			m_updateStatus = UpdateState::None;
		}
	}break;*/
	default:
		break;
	}

	m_uploadManager.update();

	SAMPLE_BEGIN(cpuProfiler);
	m_currentFrameData->wait();
	SAMPLE_END(cpuProfiler, "gpu wait");
	m_currentFrameData->releaseCpuMemory();

	m_currentFrameData->cmdAllocator->Reset();

	SAMPLE_BEGIN(cpuProfiler);

	m_uploadManager.buildCmdList(m_frameDataIndex);

	// build cmdLists
	m_defaultProgram.buildCmdList(m_currentFrameData, m_frameDataIndex, &m_swapChain);

	SAMPLE_END(cpuProfiler, "build cmd");

	u32 listCount = 0;
	ID3D12CommandList* cmdLists[4];

	//m_textureManager.update((ID3D12CommandList**)&cmdLists, &listCount);

	m_uploadManager.submit(cmdLists, &listCount, m_frameDataIndex);
	m_defaultProgram.submit(cmdLists, &listCount);

	if (listCount != 0)
	{
		m_cmdQueue->ExecuteCommandLists(listCount, cmdLists);
		m_currentFrameData->signal(m_cmdQueue);
	}

	SAMPLE_BEGIN(cpuProfiler);
	m_swapChain.present();
	SAMPLE_END(cpuProfiler, "present");
}

void RenderAspect::pushFrameData(RenderFrameDynamicData &&data)
{
	m_nextFrameData->dynamicData.swap(data);
	m_nextFrameData->dynamicData.cameraPosition = data.cameraPosition;
	m_updateStatus = UpdateState::UpdateData;
}

#if _EDITOR
template<typename T>
bool RenderAspect::createEntityMesh(T* entity)
{
	auto indexOffset = m_meshManager.getIndexCount();
	auto vertexOffset = m_meshManager.getVertexCount();

	auto entityOffset = m_meshManager.getStaticEntityOffset();
	if (!m_meshManager.addMeshes(entity, 1, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager))
		return false;

	m_currentFrameData->indexCount = m_meshManager.getIndexCount();
	m_currentFrameData->vbv = m_meshManager.getVertexBufferView();
	m_currentFrameData->ibv = m_meshManager.getIndexBufferView();

	m_nextFrameData->indexCount = m_meshManager.getIndexCount();
	m_nextFrameData->vbv = m_meshManager.getVertexBufferView();
	m_nextFrameData->ibv = m_meshManager.getIndexBufferView();

	entity->transformOffset = entityOffset;
	entity->vertexOffset = vertexOffset;
	entity->indexOffset = indexOffset;

	return true;
}

bool RenderAspect::createEntityMesh(EditorDynamicEntityFile* entity)
{
	auto indexOffset = m_meshManager.getIndexCount();
	auto vertexOffset = m_meshManager.getVertexCount();

	auto entityOffset = m_meshManager.getDynamicEntityOffset();
	if (!m_meshManager.addMeshes(entity, 1, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager))
		return false;

	m_currentFrameData->indexCount = m_meshManager.getIndexCount();
	m_currentFrameData->vbv = m_meshManager.getVertexBufferView();
	m_currentFrameData->ibv = m_meshManager.getIndexBufferView();

	m_nextFrameData->indexCount = m_meshManager.getIndexCount();
	m_nextFrameData->vbv = m_meshManager.getVertexBufferView();
	m_nextFrameData->ibv = m_meshManager.getIndexBufferView();

	entity->transformOffset = entityOffset;
	entity->vertexOffset = vertexOffset;
	entity->indexOffset = indexOffset;

	return true;
}

template<typename T>
void RenderAspect::updateEntityMesh(const T& entity)
{
	MeshManagerUpdateMeshDesc desc{};
	desc.transformOffset = entity.transformOffset;
	desc.vertexOffset = entity.vertexOffset;
	desc.indexOffset = entity.indexOffset;
	desc.allocator = m_mainAllocator;
	desc.uploadManager = &m_uploadManager;
	desc.frameData0TransformBlock = m_currentFrameData->transformBuffer;
	desc.frameData1TransformBlock = m_nextFrameData->transformBuffer;
	desc.textureManager = &m_textureArrayManager;

	m_meshManager.updateMesh(entity, desc);
}
#endif

void RenderAspect::handleEditorEvent(const Event::Event &evt)
{
#if _EDITOR
	auto type = evt.asEditorEvent();
	switch (type)
	{
	case Event::Type::EditorEventTypes::CreatePlayer:
	{
		auto& evtManager = Locator::getEventManager();
		auto data = (EditorDynamicEntityFile*)evt.getData().ptr;

		auto dynamicOffset = m_meshManager.getDynamicEntityOffset();
		if (m_meshManager.addMeshes(data, 1, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager))
		{
			data->createdRenderer = 1;
			data->transformOffset = dynamicOffset;

			vx::Variant evtData;
			evtData.ptr = data;
			evtManager.pushEvent(Event::Event(Event::Type::EditorEventTypes::RendererCreatedPlayer, evtData, evt.getCallback()));
		}
		else
		{
			evtManager.pushEvent(evt);
		}
	}break;
	case Event::Type::EditorEventTypes::CreateSpawn:
	{
		auto entity = (Editor::SpawnEntityFile*)evt.getData().ptr;
		if (entity->createdRenderer == 0)
		{
			EditorStaticEntityFile staticEntity;
			staticEntity.position = { entity->position.x, entity->position.y };
			staticEntity.texture = entity->textureSid;
			staticEntity.shape.type = EntityShapeType::Circle;
			staticEntity.shape.radius = { 1.0f, 1.0f };

			if (!createEntityMesh(&staticEntity))
			{
				auto& evtManager = Locator::getEventManager();
				evtManager.pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateSpawn, evt.getData(), evt.getCallback()));
			}
			else
			{
				entity->transformOffset = staticEntity.transformOffset;
				entity->vertexOffset = staticEntity.vertexOffset;
				entity->indexOffset = staticEntity.indexOffset;
				entity->createdRenderer = 1;
			}
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateSpawnPosition:
	{
		auto entity = (Editor::SpawnEntityFile*)evt.getData().ptr;
		EditorStaticEntityFile staticEntity;
		staticEntity.position = { entity->position.x, entity->position.y };
		staticEntity.texture = entity->textureSid;
		staticEntity.shape.type = EntityShapeType::Circle;
		staticEntity.shape.radius = { 1.0f, 1.0f };

		updateEntityMesh(staticEntity);
	}break;
	case Event::Type::EditorEventTypes::CreateGoal:
	{
		auto entity = (Editor::GoalEntityFile*)evt.getData().ptr;
		if (entity->createdRenderer == 0)
		{
			EditorStaticEntityFile staticEntity;
			staticEntity.position = { entity->position.x, entity->position.y };
			staticEntity.texture = entity->textureSid;
			staticEntity.shape.type = EntityShapeType::Rectangle;
			staticEntity.shape.halfDim = { entity->halfDim.x,entity->halfDim.y };

			if (!createEntityMesh(&staticEntity))
			{
				auto& evtManager = Locator::getEventManager();
				evtManager.pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateGoal, evt.getData(), evt.getCallback()));
			}
			else
			{
				entity->transformOffset = staticEntity.transformOffset;
				entity->vertexOffset = staticEntity.vertexOffset;
				entity->indexOffset = staticEntity.indexOffset;

				entity->createdRenderer = 1;
			}
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGoalShape:
	{
		auto entity = (Editor::GoalEntityFile*)evt.getData().ptr;
		updateEntityMesh(entity->asStaticEntity());

	}break;
	case Event::Type::EditorEventTypes::CreateStaticEntity:
	{
		EditorStaticEntityFile* entity = (EditorStaticEntityFile*)evt.getData().ptr;

		createEntityMesh(entity);

		entity->createdRenderer = 1;
	}break;
	case Event::Type::EditorEventTypes::UpdateStaticEntityShape:
	{
		EditorStaticEntityFile* entity = (EditorStaticEntityFile*)evt.getData().ptr;
		updateEntityMesh(*entity);
	}break;
	case Event::Type::EditorEventTypes::CreateDynamicEntity:
	{
		EditorDynamicEntityFile* entity = (EditorDynamicEntityFile*)evt.getData().ptr;

		createEntityMesh(entity);
		entity->createdRenderer = 1;
	}break;
	case Event::Type::EditorEventTypes::UpdateDynamicEntityShape:
	{
		EditorDynamicEntityFile* entity = (EditorDynamicEntityFile*)evt.getData().ptr;
		updateEntityMesh(*entity);
	}break;
	case Event::Type::EditorEventTypes::CreateGravityArea:
	{
		EditorGravityAreaFile* entity = (EditorGravityAreaFile*)evt.getData().ptr;
		if (entity->createdRenderer == 0)
		{
			createEntityMesh(entity);
			entity->createdRenderer = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGravityAreaShape:
	{
		EditorGravityAreaFile* entity = (EditorGravityAreaFile*)evt.getData().ptr;
		updateEntityMesh(*entity);
	}break;
	case Event::Type::EditorEventTypes::CreateGravityWell:
	{
		EditorGravityWellFile* entity = (EditorGravityWellFile*)evt.getData().ptr;
		if (entity->createdRenderer == 0)
		{
			createEntityMesh(entity);
			entity->createdRenderer = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGravityWellShape:
	{
		EditorGravityWellFile* entity = (EditorGravityWellFile*)evt.getData().ptr;
		updateEntityMesh(*entity);
	}break;
	default:
		break;
	}
#endif
}

void RenderAspect::reset()
{
	m_meshManager.clear();
}

void RenderAspect::loadLevel(const LevelFile &level)
{
	size_t gravityAreaCount = 0;
	auto gravityAreas = level.getGravityAreas(&gravityAreaCount);
	m_meshManager.addMeshes(gravityAreas, (u32)gravityAreaCount, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	size_t staticMeshCount = 0;
	auto staticEntities = level.getStaticEntities(&staticMeshCount);
	m_meshManager.addMeshes(staticEntities, (u32)staticMeshCount, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	auto goalArea = level.getGoalArea();
	m_meshManager.addMeshes(&goalArea.asStaticEntity(), 1, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	size_t gravityWellCount = 0;
	auto gravityWells = level.getGravityWells(&gravityWellCount);
	m_meshManager.addMeshes(gravityWells, (u32)gravityWellCount, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	auto dynamicOffset = m_meshManager.getDynamicEntityOffset();
	VX_ASSERT(dynamicOffset == level.getDynamicOffset());

	auto playerEntity = level.getPlayerEntity();
	m_meshManager.addMeshes(&playerEntity, 1, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	size_t dynamicEntityCount = 0;
	auto dynamicEntities = level.getDynamicEntities(&dynamicEntityCount);
	m_meshManager.addMeshes(dynamicEntities, dynamicEntityCount, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);

	m_currentFrameData->dynamicTransformOffset = dynamicOffset;
	m_nextFrameData->dynamicTransformOffset = dynamicOffset;

	while (!m_uploadManager.empty())
	{
		m_currentFrameData->wait();

		m_uploadManager.update();
		m_uploadManager.buildCmdList(m_frameDataIndex);

		m_uploadManager.execute(m_cmdQueue, m_frameDataIndex);
		m_currentFrameData->signal(m_cmdQueue);
	}

	m_currentFrameData->wait();

	m_currentFrameData->indexCount = m_meshManager.getIndexCount();
	m_currentFrameData->vbv = m_meshManager.getVertexBufferView();
	m_currentFrameData->ibv = m_meshManager.getIndexBufferView();

	m_nextFrameData->indexCount = m_meshManager.getIndexCount();
	m_nextFrameData->vbv = m_meshManager.getVertexBufferView();
	m_nextFrameData->ibv = m_meshManager.getIndexBufferView();
}

void RenderAspect::handleFileEvent(const Event::Event &evt)
{
	auto fileEvent = evt.asFileEvent();
	if (fileEvent == Event::Type::FileEventTypes::LoadedLevel)
	{
		auto &resourceAspect = Locator::getResourceAspect();

		vx::StringID sid(evt.getData().u64);
		auto level = resourceAspect.findLevel(sid);

		m_textureArrayManager.clear();
		reset();

		size_t textureCount = 0;
		auto textures = level->getTextures(&textureCount);
		for (size_t i = 0; i < textureCount; ++i)
		{
			auto &textureSid = textures[i];
			auto texture = resourceAspect.findTexture(textureSid);
			VX_ASSERT(texture != nullptr);

			size_t idx = 0;
			auto r = m_textureArrayManager.allocateTexture(textureSid, &idx);
			VX_ASSERT(r == true);

			auto textureDim = texture->getDimension();
			D3D12_SUBRESOURCE_FOOTPRINT footprint{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, textureDim.x, textureDim.y, 1, texture->getRowPitch() };

			auto uploaded = m_uploadManager.pushUploadTexture(texture->getPixels(), texture->getPixelSize(), 0, m_textureArrayManager.getSrgbaTextureResource(), (u32)idx, footprint);
			VX_ASSERT(uploaded);
			//m_textureManager.uploadTextureData(idx, texture->getPixels(), texture->getPixelSize());
		}

		loadLevel(*level);
	}
	else if (fileEvent == Event::Type::FileEventTypes::LoadedTexture)
	{
		auto &resourceAspect = Locator::getResourceAspect();
		auto textureSid = vx::StringID(evt.getData().u64);
		auto texture = resourceAspect.findTexture(textureSid);

		size_t idx = 0;
		auto r = m_textureArrayManager.allocateTexture(textureSid, &idx);
		VX_ASSERT(r == true);

		auto textureDim = texture->getDimension();
		D3D12_SUBRESOURCE_FOOTPRINT footprint{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, textureDim.x, textureDim.y, 1, texture->getRowPitch() };

		auto uploaded = m_uploadManager.pushUploadTexture(texture->getPixels(), texture->getPixelSize(), 0, m_textureArrayManager.getSrgbaTextureResource(), (u32)idx, footprint);
		VX_ASSERT(uploaded);
	}
}

void RenderAspect::handleEvent(const Event::Event &evt)
{
	auto evttype = evt.getType();
	if (evttype == Event::Type::EventTypeFile)
	{
		handleFileEvent(evt);
	}
#if !_EDITOR
	else if (evttype == Event::Type::EventTypeIngame)
	{
		auto type = evt.asIngameEvent();
		if (type == Event::Type::IngameEventTypes::RendererRebuildDynamicEntities)
		{
			Event::RendererRebuildDynamicEntitiesData* p = (Event::RendererRebuildDynamicEntitiesData*)evt.getData().ptr;
			rebuildDynamicMeshes((DynamicEntity*)p->data, p->count);

			delete(p);
		}
		else if (type == Event::Type::IngameEventTypes::RestartLevelBegin)
		{
			Event::RestartLevelData &p = *((Event::RestartLevelData*)&evt.getData().ptr);
			if (p.restartRender == 0)
			{
				reset();
				p.restartRender = 1;
			}

			if (p.finished())
			{
				auto &evtManager = Locator::getEventManager();
				evtManager.pushEvent(Event::Event(Event::Type::IngameEventTypes::RestartLevel, vx::Variant()));
			}
		}
		else if (type == Event::Type::IngameEventTypes::RestartLevelFinished)
		{
			vx::StringID sid(evt.getData().u64);
			auto level = Locator::getResourceAspect().findLevel(sid);
			loadLevel(*level);
		}
	}
#endif
#if _EDITOR
	else if (evt.getType() == Event::Type::EventTypeEditor)
	{
		handleEditorEvent(evt);
	}
#endif
}

void RenderAspect::rebuildDynamicMeshes(DynamicEntity* entities, size_t count)
{
	m_meshManager.resetDynamicEntityCount();
	auto dynamicOffset = m_meshManager.getDynamicEntityOffset();

	m_meshManager.addMeshes(entities, count, m_mainAllocator, &m_uploadManager, m_currentFrameData->transformBuffer, m_nextFrameData->transformBuffer, &m_textureArrayManager);
	for (size_t i = 0; i < count; ++i)
	{
		entities[i].rendererIndex = dynamicOffset + i;
	}

	m_currentFrameData->dynamicTransformOffset = dynamicOffset;
	m_nextFrameData->dynamicTransformOffset = dynamicOffset;

	m_currentFrameData->indexCount = m_meshManager.getIndexCount();
	m_currentFrameData->vbv = m_meshManager.getVertexBufferView();
	m_currentFrameData->ibv = m_meshManager.getIndexBufferView();

	m_nextFrameData->indexCount = m_meshManager.getIndexCount();
	m_nextFrameData->vbv = m_meshManager.getVertexBufferView();
	m_nextFrameData->ibv = m_meshManager.getIndexBufferView();
}

void RenderAspect::getViewMatrix(vx::mat4* m) const
{
	*m = m_viewMatrix;
}

void RenderAspect::getProjectionMatrix(vx::mat4* m) const
{
	*m = m_projectionMatrix;
}

__m128 RenderAspect::getCameraPosition() const
{
#if _EDITOR
	return m_cameraPosition;
#else
	return{};
#endif
}