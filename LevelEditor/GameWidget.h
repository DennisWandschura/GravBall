#pragma once

class QTreeWidgetItem;
struct EditorStaticEntityFile;
class EditorEntity;
struct EditorDynamicEntityFile;
struct EditorGravityAreaFile;
struct EditorGravityWellFile;
class LevelEditor;

namespace Ui
{
	class DynamicEntityForm;
}

#include <vxLib/types.h>

namespace Event
{
	namespace Type
	{
		enum class EditorEventTypes : u32;
	}
}

namespace Editor
{
	class EntityPlayerSpawn;
	class EntityGoal;
	class EntityGoal;

	struct SpawnEntityFile;
	struct GoalEntityFile;
}

#include <qwidget.h>
#include "EditorSystemAspect.h"
#include "EditorPhysicsAspect.h"
#include <gamelib/EngineBase.h>
#include <Windows.h>
#include <gamelib/MainAllocator.h>
#include <gamelib/AllocatorDebugLayer.h>
#include <gamelib/ControllerLibrary.h>
#include <vxLib/Variant.h>
#include <gamelib/Logfile.h>

class GameWidget : public QWidget
{
	Q_OBJECT

	EngineBase* m_engine;
	QTreeWidgetItem* m_rootStaticEntities;
	QTreeWidgetItem* m_rootDynamicEntities;
	QTreeWidgetItem* m_rootGravityAreas;
	QTreeWidgetItem* m_rootGravityWells;
	Editor::EntityPlayerSpawn* m_spawn;
	Editor::EntityGoal* m_goal;
	EditorSystemAspect m_systemAspect;
	EditorPhysicsAspect m_physicsAspect;
	Logfile m_logFile;
	int m_mousePos[2];
	bool m_mouseMiddleButtonPressed;
	LevelEditor* m_levelEditor;
	Allocator::MainAllocator m_mainAllocator;
	Allocator::DebugLayer m_allocatorDebugLayer;
	ControllerLibrary m_controllerLibrary;
	HMODULE m_dllFile;
	bool m_isShutdown;

	void paintEvent(QPaintEvent* evt) Q_DECL_OVERRIDE;

	void mousePressEvent(QMouseEvent* evt) override;
	void mouseReleaseEvent(QMouseEvent* evt) override;
	void mouseMoveEvent(QMouseEvent* evt) override;
	void keyPressEvent(QKeyEvent* evt) override;

	void frame();

public:
	explicit GameWidget(QWidget* parent = 0);
	~GameWidget();

	bool initialize(QTreeWidgetItem* staticEntities, QTreeWidgetItem* dynamicEntities, QTreeWidgetItem* gravityAreas, QTreeWidgetItem* rootGravityWells, LevelEditor* editor, Editor::EntityPlayerSpawn** spawn, Editor::EntityGoal** goal);
	void shutdown();

	void addStaticEntity(EditorStaticEntityFile* p);
	void addDynamicEntity(EditorDynamicEntityFile* p);
	void addGravityArea(EditorGravityAreaFile* p);
	void addGravityWell(EditorGravityWellFile* p);
	void setSpawnData(Editor::SpawnEntityFile* p);
	void setGoalData(Editor::GoalEntityFile* p);

	void pushEditorEvent(Event::Type::EditorEventTypes type, const vx::Variant &data);

	void saveLevel(const char* file);

public slots:
	void onButtonCreateStaticEntity();
	void onButtonCreateDynamicEntity();
	void onButtonCreateGravityArea();
	void onButtonCreateGravityWell();
};