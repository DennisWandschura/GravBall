#include "EditorEntityGravityWell.h"
#include "EntityDataWidgets.h"
#include <gamelib/Event.h>
#include "GameWidget.h"

namespace Editor
{
	EntityGravityWell::EntityGravityWell(const QString &text, GameWidget* gameWidget, EditorGravityWellFile* data)
		:EntityBase(text, gameWidget, EntityType::GravityWell),
		m_data(data)
	{

	}

	EntityGravityWell::~EntityGravityWell()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityGravityWell::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateGravityWellShape, data);
	}

	void EntityGravityWell::updateData()
	{

	}

	void EntityGravityWell::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();
		auto radius = getRadius();
		auto force = getForce();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();

		widgets->m_widgetRadius->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 1);
		widgets->m_widgetRadius->setValue(radius);
		widgets->m_widgetRadius->show();

		widgets->m_widgetForce1d->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 2);
		widgets->m_widgetForce1d->setValue(force);
		widgets->m_widgetForce1d->show();
	}

	void EntityGravityWell::setPosition(const vx::float2a &p)
	{
		m_data->position = p;
		updateShape();
	}

	void EntityGravityWell::setRadius(f32 r)
	{
		m_data->radius = r;
		updateShape();
	}

	void EntityGravityWell::setForce(f32 f)
	{
		m_data->maxForce = f;
	}

	EntityShapeType EntityGravityWell::getShapeType() const
	{
		return m_data->getShapeType();
	}

	const vx::float2a& EntityGravityWell::getPosition() const
	{
		return m_data->position;
	}

	f32 EntityGravityWell::getRadius() const
	{
		return m_data->radius;
	}

	f32 EntityGravityWell::getForce() const
	{
		return m_data->maxForce;
	}
}