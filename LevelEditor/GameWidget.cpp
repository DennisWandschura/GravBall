#include "GameWidget.h"
#include <qpainter.h>
#include <qwindow.h>
#include <vxLib/Allocator/Mallocator.h>
#include <game/dllExport.h>
#include <gamelib/Event.h>
#include "EditorPlayerInputController.h"
#include <qevent.h>
#include <vxLib/file/FileHandle.h>
#include <gamelib/EntityFile.h>
#include <qtreewidget.h>
#include "EditorEntityDynamic.h"
#include "EditorEntityStatic.h"
#include <gamelib/LevelFile.h>
#include <vxLib/File/File.h>
#include "EditorEntityGravityArea.h"
#include <gamelib/GravityWell.h>
#include "EditorEntityGravityWell.h"
#include "EditorEntityGoal.h"
#include "EditorEntityPlayerSpawn.h"
#include <gamelib/Entity.h>
#include <gamelib/PhysicsComponent.h>
#include "leveleditor.h"

EditorPlayerInputController* g_playerController{ nullptr };
GameWidget* g_GameWidget{ nullptr };

InputController* createPlayerInputController(void*)
{
	return (g_playerController = new EditorPlayerInputController());
}

void destroyInputController(InputController* p)
{
	delete(p);
}

void createPlayerCallback(vx::Variant data)
{
	EditorDynamicEntityFile* p = (EditorDynamicEntityFile*)data.ptr;
	if (p->createdPhysics != 0 && p->createdRenderer != 0)
	{
		delete(p);
	}
}

void createStaticEntityCallback(vx::Variant data)
{
	auto entity = (EditorStaticEntityFile*)data.ptr;
	if (entity->createdPhysics != 0 && entity->createdRenderer != 0)
	{
		g_GameWidget->addStaticEntity(entity);
	}
}

void createDynamicEntityCallback(vx::Variant data)
{
	auto entity = (EditorDynamicEntityFile*)data.ptr;
	if (entity->createdPhysics != 0 && entity->createdRenderer != 0)
	{
		g_GameWidget->addDynamicEntity(entity);
	}
}

void createGravityAreaCallback(vx::Variant data)
{
	auto entity = (EditorGravityAreaFile*)data.ptr;
	g_GameWidget->addGravityArea(entity);
}

void createGravityWellCallback(vx::Variant data)
{
	auto entity = (EditorGravityWellFile*)data.ptr;
	g_GameWidget->addGravityWell(entity);
}

void createSpawnCallback(vx::Variant data)
{
	auto entity = (Editor::SpawnEntityFile*)data.ptr;
	if (entity->createdPhysics == 1 && entity->createdRenderer == 1)
	{
		g_GameWidget->setSpawnData(entity);
	}
}

void createGoalCallback(vx::Variant data)
{
	auto entity = (Editor::GoalEntityFile*)data.ptr;
	if (entity->createdPhysics == 1 && entity->createdRenderer == 1)
	{
		g_GameWidget->setGoalData(entity);
	}
}

void loadTextureCallback(vx::Variant data)
{
	delete((vx::FileHandle*)data.ptr);
}

void updateStaticEntityPositionCallback(vx::Variant data)
{
	delete((Event::Editor::UpdateStaticEntityPosition*)data.ptr);
}

void updateStaticEntityShapeCallback(vx::Variant data)
{
	delete((EditorStaticEntityFile*)data.ptr);
}

GameWidget::GameWidget(QWidget* parent)
	:QWidget(parent),
	m_engine(nullptr),
	m_rootStaticEntities(nullptr),
	m_rootDynamicEntities(nullptr),
	m_rootGravityAreas(nullptr),
	m_rootGravityWells(nullptr),
	m_systemAspect(),
	m_physicsAspect(),
	m_logFile(),
	m_mouseMiddleButtonPressed(false),
	m_mainAllocator(),
	m_allocatorDebugLayer(),
	m_controllerLibrary(),
	m_dllFile(nullptr),
	m_isShutdown(true)
{
	g_GameWidget = this;
}

GameWidget::~GameWidget()
{
	shutdown();
}

bool GameWidget::initialize(QTreeWidgetItem* staticEntities, QTreeWidgetItem* dynamicEntities, QTreeWidgetItem* gravityAreas, QTreeWidgetItem* rootGravityWells,
	LevelEditor* editor, Editor::EntityPlayerSpawn** spawn, Editor::EntityGoal** goal)
{
	m_dllFile = LoadLibraryA("../../lib/game_debug.dll");
	if (!m_dllFile)
		return false;

	m_systemAspect.setHwnd((void*)winId());

	vx::Mallocator malloc;
	m_logFile.create("logfile.txt", malloc.allocate(2 KBYTE, 16));

	m_allocatorDebugLayer.initialize(&malloc);
	m_mainAllocator.initialize(&malloc, &m_allocatorDebugLayer);

	m_controllerLibrary.initialize(8, &m_mainAllocator);
	m_controllerLibrary.registerFunction(vx::make_sid("player"), createPlayerInputController);

	auto createEngineProcAdr = (CreateEngineProc)GetProcAddress(m_dllFile, "createEngine");

	EngineBaseDesc desc;
	desc.physicsAspectDesc.allocator = &m_mainAllocator;
	desc.physicsAspectDesc.capacityDynamic = 128;
	desc.physicsAspectDesc.capacityGravityAreas = 16;
	desc.physicsAspectDesc.capacityGravityWell = 16;
	desc.physicsAspectDesc.capacityStatic = 128;
	desc.physicsAspectDesc.onCollisionCallback = nullptr;
	desc.mainAllocator = &m_mainAllocator;
	desc.onKeyPressed = nullptr;
	desc.onKeyReleased = nullptr;
	desc.physicsAspect = &m_physicsAspect;
	desc.resolution = vx::uint2(width(), height());
	sprintf(desc.rootDir, "../../");
	desc.systemAspect = &m_systemAspect;
	desc.logFile = &m_logFile;
	desc.controllerLibrary = &m_controllerLibrary;
	desc.destroyInputControllerFunction = destroyInputController;

	auto engine = createEngineProcAdr(desc);
	if (!engine)
		return false;

	m_engine = engine;
	m_levelEditor = editor;

	auto playerEntity = new EditorDynamicEntityFile();
	playerEntity->controller = vx::make_sid("player");
	playerEntity->texture = vx::make_sid("editor.bmp");
	playerEntity->position = {0, 0};
	playerEntity->shape.type = EntityShapeType::Circle;
	playerEntity->shape.radius = { 1,1 };

	vx::Variant createPlayerVariant;
	createPlayerVariant.ptr = playerEntity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreatePlayer, createPlayerVariant, createPlayerCallback));

	vx::Variant loadTextureVariant;
	loadTextureVariant.ptr = new vx::FileHandle("green.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("enemy.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("player.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("goal.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("gravityAreaStatic.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("gravityWell.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	loadTextureVariant.ptr = new vx::FileHandle("editor.bmp");
	m_engine->pushEvent(Event::Event(Event::Type::FileEventTypes::LoadTexture, loadTextureVariant, loadTextureCallback));

	auto spawnEntity = new Editor::SpawnEntityFile();
	spawnEntity->position = {0, 0};
	spawnEntity->textureSid = vx::make_sid("player.bmp");
	*spawn = m_spawn = new Editor::EntityPlayerSpawn("Player Spawn", this, spawnEntity);

	vx::Variant spawnData;
	spawnData.ptr = spawnEntity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateSpawn, spawnData, createSpawnCallback));

	auto goalEntity = new Editor::GoalEntityFile();
	goalEntity->position = {5, 0};
	goalEntity->halfDim = {1, 1};
	goalEntity->textureSid = vx::make_sid("goal.bmp");
	*goal = m_goal = new Editor::EntityGoal("Player Goal", this, goalEntity);

	vx::Variant goalData;
	goalData.ptr = goalEntity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateGoal, goalData, createGoalCallback));

	m_rootStaticEntities = staticEntities;
	m_rootDynamicEntities = dynamicEntities;
	m_rootGravityAreas = gravityAreas;
	m_rootGravityWells = rootGravityWells;
	m_isShutdown = false;

	return true;
}

void GameWidget::shutdown()
{
	if (!m_isShutdown)
	{
		g_playerController = nullptr;

		if (m_engine)
		{
			auto destroyEngineProcAdr = (DestroyEngineProc)GetProcAddress(m_dllFile, "destroyEngine");
			destroyEngineProcAdr(m_engine, &m_mainAllocator);
			m_engine = nullptr;
		}

		m_controllerLibrary.release();

		vx::Mallocator malloc;
		m_mainAllocator.shutdown(&malloc);
		m_allocatorDebugLayer.shutdown(&malloc);

		malloc.deallocate(m_logFile.close());

		if (m_dllFile)
		{
			FreeModule(m_dllFile);
			m_dllFile = nullptr;
		}

		m_isShutdown = true;
	}
}

void GameWidget::frame()
{
	const f32 dt = 1.f / 60.0f;
	m_engine->update(dt, &m_mainAllocator);
	m_engine->render();
}

void GameWidget::paintEvent(QPaintEvent*)
{
	frame();
}

void GameWidget::mousePressEvent(QMouseEvent* evt)
{
	auto button = evt->button();

	if (button == Qt::MouseButton::MiddleButton)
	{
		m_mousePos[0] = evt->x();
		m_mousePos[1] = evt->y();
		m_mouseMiddleButtonPressed = true;
	}
	else if (button == Qt::MouseButton::LeftButton)
	{
		auto x = evt->x();
		auto y = evt->y();

		vx::mat4 viewMatrix, projMatrix;
		m_engine->getViewMatrix(&viewMatrix);
		m_engine->getProjectionMatrix(&projMatrix);

		m_levelEditor->deselectEntity();

		auto result = m_physicsAspect.getShape(x, y,vx::float2(width(), height()), m_engine->getCameraPosition(), viewMatrix, projMatrix);
		if (result != nullptr)
		{
			auto entity = (Editor::EntityBase*)result->m_userData;
			m_levelEditor->setSelectedEntity(entity);
		}
	}
	else if (button == Qt::MouseButton::RightButton)
	{
		m_levelEditor->deselectEntity();
	}
}

void GameWidget::mouseReleaseEvent(QMouseEvent* evt)
{
	auto button = evt->button();

	if (button == Qt::MouseButton::MiddleButton)
	{
		m_mouseMiddleButtonPressed = false;
	}
}

void GameWidget::mouseMoveEvent(QMouseEvent* evt)
{
	if (m_mouseMiddleButtonPressed)
	{
		auto newx = evt->x();
		auto newy = evt->y();

		auto diffx = f32(newx - m_mousePos[0]) * 0.1f;
		auto diffy = f32(newy - m_mousePos[1]) * 0.1f;

		g_playerController->move(diffx, diffy);

		m_mousePos[0] = newx;
		m_mousePos[1] = newy;

		frame();
	}
}

void GameWidget::keyPressEvent(QKeyEvent* evt)
{
	if (g_playerController)
	{
		auto key = evt->key();
		if (key == Qt::Key_A)
		{
			g_playerController->move(-1.f, 0);
		}
		else if (key == Qt::Key_D)
		{
			g_playerController->move(1.f, 0);
		}
		else if (key == Qt::Key_W)
		{
			g_playerController->move(0, 1);
		}
		else if (key == Qt::Key_S)
		{
			g_playerController->move(0, -1);
		}

		frame();
	}
}

void GameWidget::onButtonCreateStaticEntity()
{
	auto entity = new EditorStaticEntityFile();
	entity->position = { 0, 0 };
	entity->shape.type = EntityShapeType::Rectangle;
	entity->shape.halfDim = { 1, 1 };
	entity->texture = vx::make_sid("green.bmp");
	entity->restitution = 0;
	entity->createdPhysics = 0;
	entity->createdRenderer = 0;

	vx::Variant variant;
	variant.ptr = entity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateStaticEntity, variant, createStaticEntityCallback));

	frame();
}

void GameWidget::onButtonCreateDynamicEntity()
{
	auto entity = new EditorDynamicEntityFile();
	entity->controller = vx::make_sid("default");
	entity->mass = 50.0f;
	entity->position = { 0, 0 };
	entity->restitution = 1;
	entity->shape.type = EntityShapeType::Circle;
	entity->shape.radius.x = 1;
	entity->texture = vx::make_sid("enemy.bmp");
	entity->createdPhysics = 0;
	entity->createdRenderer = 0;

	vx::Variant variant;
	variant.ptr = entity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateDynamicEntity, variant, createDynamicEntityCallback));

	frame();
}

void GameWidget::onButtonCreateGravityArea()
{
	auto texGravityAreaStaticHandle = vx::FileHandle("gravityAreaStatic.bmp");

	auto entity = new EditorGravityAreaFile();

	entity->position = { 0, 0 };
	entity->texture = texGravityAreaStaticHandle.m_sid;
	entity->force = {50, 0};
	entity->halfDim = {5, 5};
	entity->time = 5.0f;
	entity->type = GravityAreaType::Static;

	vx::Variant variant;
	variant.ptr = entity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateGravityArea, variant, createGravityAreaCallback));

	frame();
}

void GameWidget::onButtonCreateGravityWell()
{
	auto texGravityWellHandle = vx::FileHandle("gravityWell.bmp");

	auto entity = new EditorGravityWellFile();
	entity->position = {0, 0};
	entity->radius = 5.0f;
	entity->maxForce = 50.0f;
	entity->texture = texGravityWellHandle.m_sid;

	vx::Variant variant;
	variant.ptr = entity;
	m_engine->pushEvent(Event::Event(Event::Type::EditorEventTypes::CreateGravityWell, variant, createGravityWellCallback));

	frame();
}

void GameWidget::addStaticEntity(EditorStaticEntityFile* p)
{
	auto child = new Editor::EntityStatic(QString("Static Entity %0").arg(p->transformOffset), this, p);
	p->physicsComponent->m_userData = child;

	m_rootStaticEntities->addChild(child);
}

void GameWidget::addDynamicEntity(EditorDynamicEntityFile* p)
{
	auto child = new Editor::EntityDynamic(QString("Dynamic Entity %0").arg(p->transformOffset), this, p, 0);
	p->physicsComponent->m_userData = child;

	m_rootDynamicEntities->addChild(child);
}

void GameWidget::addGravityArea(EditorGravityAreaFile* p)
{
	auto child = new Editor::EntityGravityArea(QString("Gravity Area %0").arg(p->transformOffset), this, p);
	p->physicsComponent->m_userData = child;

	m_rootGravityAreas->addChild(child);
}

void GameWidget::addGravityWell(EditorGravityWellFile* p)
{
	auto child = new Editor::EntityGravityWell(QString("Gravity Well %0").arg(p->transformOffset), this, p);
	p->physicsComponent->m_userData = child;
	m_rootGravityWells->addChild(child);
}

void GameWidget::setSpawnData(Editor::SpawnEntityFile* p)
{
	p->physicsComponent->m_userData = m_spawn;
}

void GameWidget::setGoalData(Editor::GoalEntityFile* p)
{
	p->physicsComponent->m_userData = m_goal;
}

void GameWidget::pushEditorEvent(Event::Type::EditorEventTypes type, const vx::Variant &data)
{
	m_engine->pushEvent(Event::Event(type, data, nullptr));

	frame();
}

template<typename T, typename U>
void copyEntities(vx::array<U, DebugAllocatorResource<Allocator::MainAllocator>>* dst, QTreeWidgetItem* root, Allocator::MainAllocator* alloc)
{
	auto count = root->childCount();
	vx::array<U, DebugAllocatorResource<Allocator::MainAllocator>> tmp(alloc, count);
	for (int i = 0; i < count; ++i)
	{
		auto it = (T*)root->child(i);
		VX_ASSERT(it->getEntityType() == T::getStaticType());
		tmp.push_back(*it->getData());
	}

	dst->swap(tmp);
}

void GameWidget::saveLevel(const char* file)
{
	auto texGreenHandle = vx::FileHandle("green.bmp");
	auto texPlayerHandle = vx::FileHandle("player.bmp");
	auto texEnemyHandle = vx::FileHandle("enemy.bmp");
	auto texGravityAreaStaticHandle = vx::FileHandle("gravityAreaStatic.bmp");
	auto texGravityWellHandle = vx::FileHandle("gravityWell.bmp");
	auto texGoalHandle = vx::FileHandle("goal.bmp");
	vx::StringID textureSids[] =
	{
		texGreenHandle.m_sid,
		texPlayerHandle.m_sid,
		texEnemyHandle.m_sid,
		texGravityAreaStaticHandle.m_sid,
		texGravityWellHandle.m_sid,
		texGoalHandle.m_sid
	};

	vx::array<vx::StringID, DebugAllocatorResource<Allocator::MainAllocator>> textures(&m_mainAllocator, _countof(textureSids));
	for (int i = 0; i < _countof(textureSids); ++i)
	{
		textures.push_back(textureSids[i]);
	}

	vx::array<StaticEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> staticEntities;
	copyEntities<Editor::EntityStatic>(&staticEntities, m_rootStaticEntities, &m_mainAllocator);

	vx::array<DynamicEntityFile, DebugAllocatorResource<Allocator::MainAllocator>> dynamicEntities;
	copyEntities<Editor::EntityDynamic>(&dynamicEntities, m_rootDynamicEntities, &m_mainAllocator);

	vx::array<GravityAreaFile, DebugAllocatorResource<Allocator::MainAllocator>> gravityAreas;
	copyEntities<Editor::EntityGravityArea>(&gravityAreas, m_rootGravityAreas, &m_mainAllocator);

	vx::array<GravityWellFile, DebugAllocatorResource<Allocator::MainAllocator>> gravityWells;
	copyEntities<Editor::EntityGravityWell>(&gravityWells, m_rootGravityWells, &m_mainAllocator);

	LevelFile level;
	level.setDynamicEntities(&dynamicEntities);
	level.setStaticEntities(&staticEntities);
	level.setTextures(&textures);
	level.setGravityAreas(&gravityAreas);
	level.setGravityWells(&gravityWells);

	level.setPlayerEntity(DynamicEntityFile
	{
		vx::make_sid("player"),
		vx::make_sid("player.bmp"),
		{ 0, 0 },
		{
			vx::float2{ 1, 1 },
			EntityShapeType::Circle
		},
		1.0f,
		100.0f
	});

	level.setGoalArea(*m_goal->getData());

	u64 size = 0;
	auto levelData = level.saveToMemory(&m_mainAllocator, &size);

	vx::File outFile;
	if (outFile.create(file, vx::FileAccess::Write))
	{
		outFile.write(levelData.ptr, size);
		outFile.flush();
	}

	m_mainAllocator.deallocate(levelData, Allocator::Channels::General, "save level data");
}