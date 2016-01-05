#pragma once
#ifndef LEVELEDITOR_H
#define LEVELEDITOR_H

class QTreeWidgetItem;

namespace Editor
{
	class EntityBase;
	class EntityStatic;
	class EntityDynamic;
	class EntityPlayerSpawn;
	class EntityGoal;
}

#include <vxLib/types.h>
#include <QtWidgets/QMainWindow>
#include "ui_leveleditor.h"
#include "GameWidget.h"
#include "EntityDataWidgets.h"

class LevelEditor : public QMainWindow
{
	Q_OBJECT

	Editor::EntityBase* m_selectedEntity;
	Ui::LevelEditorClass ui;
	GameWidget* m_gameWidget;
	EntityDataWidgets m_entityDataWidgets;
	Editor::EntityPlayerSpawn* m_entityPlayerSpawn;
	Editor::EntityGoal* m_entityPlayerGoal;
	QTreeWidgetItem* m_rootStaticEntities;
	QTreeWidgetItem* m_rootDynamicEntities;
	QTreeWidgetItem* m_rootGravityAreas;
	QTreeWidgetItem* m_rootGravityWells;

	void hideEntity();
	void closeEvent(QCloseEvent* event) override;

public:
	LevelEditor(QWidget *parent = 0);
	~LevelEditor();

	void deselectEntity();
	void setSelectedEntity(Editor::EntityBase* p);

public slots:
	void onUpdatePosition();
	void onUpdateHalfDim();
	void onUpdateRadius();
	void onUpdateController();
	void onUpdateForce1D();
	void onUpdateForce2D();
	void onUpdateTime();
	void onSelectEntity(QTreeWidgetItem*, int);
	void onActionExit();
	void onActionSave();
};

#endif // LEVELEDITOR_H
