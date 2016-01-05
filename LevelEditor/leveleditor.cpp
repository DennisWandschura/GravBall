#include "leveleditor.h"
#include <qstandarditemmodel.h>
#include "EditorEntityStatic.h"
#include "EditorEntityDynamic.h"
#include "EditorEntityGravityArea.h"
#include "EditorEntityGoal.h"
#include "EditorEntityPlayerSpawn.h"
#include "qevent.h"
#include "qfiledialog.h"
#include "EditorEntityGravityWell.h"

LevelEditor::LevelEditor(QWidget *parent)
	: QMainWindow(parent),
	m_selectedEntity(nullptr),
	m_gameWidget(nullptr),
	m_entityDataWidgets(),
	m_entityPlayerSpawn(nullptr),
	m_entityPlayerGoal(nullptr),
	m_rootStaticEntities(nullptr),
	m_rootDynamicEntities(nullptr)
{
	ui.setupUi(this);

	m_gameWidget = new GameWidget(ui.centralWidget);
	m_gameWidget->resize(QSize(1280, 720));
	m_gameWidget->move(QPoint(50, 50));
	m_gameWidget->setPalette(QPalette(QColor(255, 0, 0), QColor(255, 0, 0)));
	m_gameWidget->setBackgroundRole(QPalette::ColorRole::Background);
	m_gameWidget->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

	m_entityDataWidgets.initialize(ui.groupBox_Data);

	hideEntity();

	QObject::connect(ui.actionCreate_Static_Entity, SIGNAL(triggered()), m_gameWidget, SLOT(onButtonCreateStaticEntity()));
	QObject::connect(ui.actionCreate_Dynamic_Entity, SIGNAL(triggered()), m_gameWidget, SLOT(onButtonCreateDynamicEntity()));
	QObject::connect(ui.actionCreate_Gravity_Area, SIGNAL(triggered()), m_gameWidget, SLOT(onButtonCreateGravityArea()));
	QObject::connect(ui.actionCreate_Gravity_Well, SIGNAL(triggered()), m_gameWidget, SLOT(onButtonCreateGravityWell()));
	QObject::connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(onActionExit()));
	QObject::connect(ui.actionSaveFile, SIGNAL(triggered()), this, SLOT(onActionSave()));

	ui.treeWidget->setHeaderLabel("Entities");

	m_rootStaticEntities = new QTreeWidgetItem();
	m_rootStaticEntities->setText(0, "Static Entities");

	m_rootDynamicEntities = new QTreeWidgetItem();
	m_rootDynamicEntities->setText(0, "Dynamic Entities");

	m_rootGravityAreas = new QTreeWidgetItem();
	m_rootGravityAreas->setText(0, "Gravity Areas");

	m_rootGravityWells = new QTreeWidgetItem();
	m_rootGravityWells->setText(0, "Gravity Wells");

	ui.treeWidget->insertTopLevelItem(0, m_rootStaticEntities);
	ui.treeWidget->insertTopLevelItem(1, m_rootDynamicEntities);
	ui.treeWidget->insertTopLevelItem(2, m_rootGravityAreas);
	ui.treeWidget->insertTopLevelItem(3, m_rootGravityWells);

	QObject::connect(m_entityDataWidgets.m_widgetPosition->getSpinBox_x(), SIGNAL(valueChanged(double)), this, SLOT(onUpdatePosition()));
	QObject::connect(m_entityDataWidgets.m_widgetPosition->getSpinBox_y(), SIGNAL(valueChanged(double)), this, SLOT(onUpdatePosition()));

	QObject::connect(m_entityDataWidgets.m_widgetHalfDim->getSpinBox_x(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateHalfDim()));
	QObject::connect(m_entityDataWidgets.m_widgetHalfDim->getSpinBox_y(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateHalfDim()));

	QObject::connect(m_entityDataWidgets.m_widgetRadius->getSpinBox(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateRadius()));

	QObject::connect(m_entityDataWidgets.m_widgetTime->getSpinBox(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateTime()));

	QObject::connect(m_entityDataWidgets.m_widgetForce1d->getSpinBox(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateForce1D()));
	QObject::connect(m_entityDataWidgets.m_widgetForce2d->getSpinBox_x(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateForce2D()));
	QObject::connect(m_entityDataWidgets.m_widgetForce2d->getSpinBox_y(), SIGNAL(valueChanged(double)), this, SLOT(onUpdateForce2D()));

	QObject::connect(m_entityDataWidgets.m_widgetController->getQComboBox(), SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateController()));

	QObject::connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onSelectEntity(QTreeWidgetItem*, int)));

	m_gameWidget->initialize(m_rootStaticEntities, m_rootDynamicEntities, m_rootGravityAreas, m_rootGravityWells,this, &m_entityPlayerSpawn, &m_entityPlayerGoal);

	ui.treeWidget->insertTopLevelItem(4, m_entityPlayerSpawn);
	ui.treeWidget->insertTopLevelItem(5, m_entityPlayerGoal);
}

LevelEditor::~LevelEditor()
{
	if (m_gameWidget)
	{
		delete(m_gameWidget);
		m_gameWidget = nullptr;
	}
}

void LevelEditor::hideEntity()
{
	ui.groupBox_Data->hide();
	m_entityDataWidgets.hideAll();
}

void LevelEditor::closeEvent(QCloseEvent* event)
{
	m_gameWidget->shutdown();
	event->accept();
}

void LevelEditor::onUpdatePosition()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetPosition->getValues();
		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::Static)
		{
			reinterpret_cast<Editor::EntityStatic*>(m_selectedEntity)->setPosition(value);
		}
		else if (type == Editor::EntityType::Dynamic)
		{
			reinterpret_cast<Editor::EntityDynamic*>(m_selectedEntity)->setPosition(value);
		}
		else if (type == Editor::EntityType::GravityArea)
		{
			reinterpret_cast<Editor::EntityGravityArea*>(m_selectedEntity)->setPosition(value);
		}
		else if (type == Editor::EntityType::Goal)
		{
			reinterpret_cast<Editor::EntityGoal*>(m_selectedEntity)->setPosition(value);
		}
		else if (type == Editor::EntityType::PlayerSpawn)
		{
			reinterpret_cast<Editor::EntityPlayerSpawn*>(m_selectedEntity)->setPosition(value);
		}
		else if (type == Editor::EntityType::GravityWell)
		{
			reinterpret_cast<Editor::EntityGravityWell*>(m_selectedEntity)->setPosition(value);
		}
	}
}

void LevelEditor::onUpdateHalfDim()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetHalfDim->getValues();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::Static)
		{
			reinterpret_cast<Editor::EntityStatic*>(m_selectedEntity)->setHalfDim(value);
		}
		else if (type == Editor::EntityType::Dynamic)
		{
			reinterpret_cast<Editor::EntityDynamic*>(m_selectedEntity)->setHalfDim(value);
		}
		else if (type == Editor::EntityType::GravityArea)
		{
			reinterpret_cast<Editor::EntityGravityArea*>(m_selectedEntity)->setHalfDim(value);
		}
		else if (type == Editor::EntityType::Goal)
		{
			reinterpret_cast<Editor::EntityGoal*>(m_selectedEntity)->setHalfDim(value);
		}
	}
}

void LevelEditor::onUpdateRadius()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetRadius->getValue();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::GravityWell)
		{
			reinterpret_cast<Editor::EntityGravityWell*>(m_selectedEntity)->setRadius(value);
		}
	}
}

void LevelEditor::onUpdateController()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetController->getSelectedIndex();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::Dynamic)
		{
			reinterpret_cast<Editor::EntityDynamic*>(m_selectedEntity)->setController(value);
		}
	}
}

void LevelEditor::onUpdateForce1D()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetForce1d->getValue();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::GravityWell)
		{
			reinterpret_cast<Editor::EntityGravityWell*>(m_selectedEntity)->setForce(value);
		}
	}
}

void LevelEditor::onUpdateForce2D()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetForce2d->getValues();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::GravityArea)
		{
			reinterpret_cast<Editor::EntityGravityArea*>(m_selectedEntity)->setForce(value);
		}
	}
}

void LevelEditor::onUpdateTime()
{
	if (m_selectedEntity)
	{
		auto value = m_entityDataWidgets.m_widgetTime->getValue();

		auto type = m_selectedEntity->getEntityType();
		if (type == Editor::EntityType::GravityArea)
		{
			reinterpret_cast<Editor::EntityGravityArea*>(m_selectedEntity)->setTime(value);
		}
	}
}

void LevelEditor::deselectEntity()
{
	m_selectedEntity = nullptr;
	hideEntity();
}

void LevelEditor::setSelectedEntity(Editor::EntityBase* p)
{
	p->setupUI(&m_entityDataWidgets);
	m_entityDataWidgets.m_parent->show();

	m_selectedEntity = p;
}

void LevelEditor::onSelectEntity(QTreeWidgetItem* item, int l)
{
	deselectEntity();

	auto parent = item->parent();
	if (parent || item == m_entityPlayerGoal || item == m_entityPlayerSpawn)
	{
		auto entity = (Editor::EntityBase*)item;
		setSelectedEntity(entity);
	}
}

void LevelEditor::onActionExit()
{
	close();
}

void LevelEditor::onActionSave()
{
	auto str = QFileDialog::getSaveFileName(this, tr("Save File"),
		"untitled.level",
		tr("Level (*.level)"));

	auto tmp = str.toStdString();
	m_gameWidget->saveLevel(tmp.c_str());

	//fileName = QFileDialog::getOpenFileName(this,
	//	tr("Open Image"), "/home/jana", tr("Image Files (*.png *.jpg *.bmp)"));
}