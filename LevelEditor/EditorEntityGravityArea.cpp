#include "editorEntityGravityArea.h"
#include "EntityDataWidgets.h"
#include "GameWidget.h"
#include <gamelib/Event.h>

namespace Editor
{
	EntityGravityArea::EntityGravityArea(const QString &text, GameWidget* gameWidget, EditorGravityAreaFile* data)
		:EntityBase(text, gameWidget, Editor::EntityType::GravityArea),
		m_data(data)
	{
	}

	EntityGravityArea::~EntityGravityArea()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityGravityArea::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateGravityAreaShape, data);
	}

	void EntityGravityArea::updateData()
	{

	}

	void EntityGravityArea::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();
		auto halfDim = getHalfDim();
		auto force = getForce();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();

		widgets->m_widgetHalfDim->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 1);
		widgets->m_widgetHalfDim->setValues(halfDim.x, halfDim.y);
		widgets->m_widgetHalfDim->show();

		widgets->m_widgetForce2d->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 2);
		widgets->m_widgetForce2d->setValues(force.x, force.y);
		widgets->m_widgetForce2d->show();

		widgets->m_widgetTime->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 3);
		widgets->m_widgetTime->setValue(getTime());
		widgets->m_widgetTime->show();
	}

	void EntityGravityArea::setPosition(const vx::float2 &p)
	{
		m_data->position = p;

		updateShape();
	}

	void EntityGravityArea::setHalfDim(const vx::float2 &hd)
	{
		m_data->halfDim = hd;

		updateShape();
	}

	void EntityGravityArea::setForce(const vx::float2 &f)
	{
		m_data->force = f;
	}

	void EntityGravityArea::setGravityAreaType(GravityAreaType type)
	{
		m_data->type = type;
	}

	void EntityGravityArea::setTime(f32 time)
	{
		m_data->time = time;

		if (time != 0.0f)
		{
			m_data->type = GravityAreaType::Dynamic;
		}
		else
		{
			m_data->type = GravityAreaType::Static;
		}
	}

	EntityShapeType EntityGravityArea::getShapeType() const
	{
		return m_data->getShapeType();
	}

	const vx::float2& EntityGravityArea::getPosition() const
	{
		return m_data->position;
	}

	const vx::float2& EntityGravityArea::getHalfDim() const
	{
		return m_data->halfDim;
	}

	const vx::float2& EntityGravityArea::getForce() const
	{
		return m_data->force;
	}

	GravityAreaType EntityGravityArea::getGravityAreaType() const
	{
		return m_data->type;
	}

	f32 EntityGravityArea::getTime() const
	{
		return m_data->time;
	}
}