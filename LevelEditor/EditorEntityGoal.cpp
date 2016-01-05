#include "EditorEntityGoal.h"
#include <gamelib/EntityFile.h>
#include <gamelib/EntityShape.h>
#include "EntityDataWidgets.h"
#include <gamelib/Event.h>
#include "GameWidget.h"

namespace Editor
{
	EntityGoal::EntityGoal(const QString &text, GameWidget* gameWidget, GoalEntityFile* data)
		:EntityBase(text, gameWidget, EntityType::Goal),
		m_data(data)
	{

	}

	EntityGoal::~EntityGoal()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityGoal::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateGoalShape, data);
	}

	void EntityGoal::updateData()
	{

	}

	void EntityGoal::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();
		auto halfDim = getHalfDim();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();

		widgets->m_widgetHalfDim->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING);
		widgets->m_widgetHalfDim->setValues(halfDim.x, halfDim.y);
		widgets->m_widgetHalfDim->show();
	}

	void EntityGoal::setPosition(const vx::float2a &p)
	{
		m_data->position = p;
		updateShape();
	}

	void EntityGoal::setHalfDim(const vx::float2a &hd)
	{
		m_data->halfDim = hd;
		updateShape();
	}

	const vx::float2a& EntityGoal::getPosition() const
	{
		return m_data->position;
	}

	const vx::float2a& EntityGoal::getHalfDim() const
	{
		return m_data->halfDim;
	}

	EntityShapeType EntityGoal::getShapeType() const
	{
		return EntityShapeType::Rectangle;
	}
}