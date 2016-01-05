#pragma once

#include "LabelSpinboxWidget.h"
#include "LabelSpinboxSpinboxWidget.h"
#include "LabelComboboxWidget.h"

struct EntityDataWidgets
{
	static const u32 POSITION_X{ 10 };
	static const u32 POSITION_Y{ 20 };
	static const u32 SPACING{ 35 };

	QWidget* m_parent;
	LabelSpinboxSpinboxWidget* m_widgetPosition;
	LabelSpinboxSpinboxWidget* m_widgetHalfDim;
	LabelSpinboxSpinboxWidget* m_widgetForce2d;
	LabelSpinboxWidget* m_widgetForce1d;
	LabelSpinboxWidget* m_widgetRadius;
	LabelSpinboxWidget* m_widgetMass;
	LabelSpinboxWidget* m_widgetRestitution;
	LabelSpinboxWidget* m_widgetTime;
	LabelComboboxWidget* m_widgetController;

	EntityDataWidgets() :
		m_parent(nullptr),
		m_widgetPosition(nullptr),
		m_widgetHalfDim(nullptr),
		m_widgetForce2d(nullptr),
		m_widgetForce1d(nullptr),
		m_widgetRadius(nullptr),
		m_widgetMass(nullptr),
		m_widgetRestitution(nullptr),
		m_widgetTime(nullptr),
		m_widgetController(nullptr)
	{}

	void initialize(QWidget* parent)
	{
		m_parent = parent;
		m_widgetPosition = new LabelSpinboxSpinboxWidget(parent, "Position:");
		m_widgetHalfDim = new LabelSpinboxSpinboxWidget(parent, "Half Dim:");
		m_widgetHalfDim->setMinMax({ 0.1f, 100.0f }, { 0.1f, 100.0f });
		m_widgetRadius = new LabelSpinboxWidget(parent, "Radius:");
		m_widgetRadius->setMinMax(0.1f, 50.0f);
		m_widgetMass = new LabelSpinboxWidget(parent, "Mass:");
		m_widgetMass->setMinMax(0.1f, 5000.0f);
		m_widgetRestitution = new LabelSpinboxWidget(parent, "Restitution:");
		m_widgetRestitution->setMinMax(0, 10.0f);

		m_widgetForce2d = new LabelSpinboxSpinboxWidget(parent, "Force:");
		m_widgetForce2d->setMinMax({ -9999.0f, 9999.0f }, { -9999.0f, 9999.0f });

		m_widgetForce1d = new LabelSpinboxWidget(parent, "Force:");
		m_widgetForce1d->setMinMax( -9999.0f, 9999.0f);

		m_widgetTime =new LabelSpinboxWidget(parent, "Time:");
		m_widgetTime->setMinMax(0, 60.0f);

		m_widgetController = new LabelComboboxWidget(parent, "Controller:");
		m_widgetController->addItem("default");
		m_widgetController->addItem("static");
		m_widgetController->setSelected(0);
	}

	void hideAll()
	{
		m_widgetPosition->hide();
		m_widgetHalfDim->hide();
		m_widgetForce2d->hide();
		m_widgetForce1d->hide();
		m_widgetRadius->hide();
		m_widgetMass->hide();
		m_widgetRestitution->hide();
		m_widgetTime->hide();
		m_widgetController->hide();
	}
};