#include "EditorEntityStatic.h"
#include "GameWidget.h"
#include <gamelib/Event.h>
#include "EntityDataWidgets.h"

namespace Editor
{
	EntityStatic::EntityStatic(const QString &text, GameWidget* gameWidget, EditorStaticEntityFile* data)
		:EntityBase(text, gameWidget, EntityType::Static),
		m_data(data)
	{

	}

	EntityStatic::~EntityStatic()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityStatic::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();

		switch (getShapeType())
		{
		case EntityShapeType::Rectangle:
		{
			auto halfDim = getHalfDim();

			widgets->m_widgetHalfDim->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING);
			widgets->m_widgetHalfDim->setValues(halfDim.x, halfDim.y);
			widgets->m_widgetHalfDim->show();

		}break;
		case EntityShapeType::Circle:
		{
			auto radius = getRadius();

			widgets->m_widgetRadius->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING);
			widgets->m_widgetRadius->setValue(radius);
			widgets->m_widgetRadius->show();
		}break;
		default:
			break;
		}
	}

	void EntityStatic::setPosition(const vx::float2 &p)
	{
		m_data->position = p;
		updateShape();
	}

	void EntityStatic::setHalfDim(const vx::float2 &hd)
	{
		m_data->shape.halfDim = hd;
		updateShape();
	}

	void EntityStatic::setRadius(f32 r)
	{
		m_data->shape.radius.x = m_data->shape.radius.y = r;
		updateShape();
	}

	void EntityStatic::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateStaticEntityShape, data);
	}

	void EntityStatic::updateData()
	{

	}

	EntityShapeType EntityStatic::getShapeType() const
	{
		return m_data->shape.type;
	}

	const vx::float2& EntityStatic::getPosition() const { return m_data->position; }
	const vx::float2& EntityStatic::getHalfDim() const { return m_data->shape.halfDim; }
	f32 EntityStatic::getRadius() const { return m_data->shape.radius.x; }
}