#include "EditorEntityDynamic.h"
#include "EntityDataWidgets.h"
#include "GameWidget.h"
#include <gamelib/Event.h>

namespace Editor
{
	EntityDynamic::EntityDynamic(const QString &text, GameWidget* gameWidget, EditorDynamicEntityFile* data, int controller)
		:EntityBase(text, gameWidget, EntityType::Dynamic),
		m_data(data),
		m_controller(controller)
	{

	}

	EntityDynamic::~EntityDynamic()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityDynamic::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();

		widgets->m_widgetMass->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 2);
		widgets->m_widgetRestitution->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 3);

		widgets->m_widgetController->setSelected(m_controller);
		widgets->m_widgetController->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y + EntityDataWidgets::SPACING * 4);

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

		widgets->m_widgetMass->show();
		widgets->m_widgetRestitution->show();
		widgets->m_widgetController->show();
	}

	void EntityDynamic::setPosition(const vx::float2 &p)
	{
		m_data->position = p;

		updateShape();
	}

	void EntityDynamic::setHalfDim(const vx::float2 &hd)
	{
		m_data->shape.halfDim = hd;

		updateShape();
	}

	void EntityDynamic::setRadius(f32 r)
	{
		m_data->shape.radius.x = m_data->shape.radius.y = r;

		updateShape();
	}

	void EntityDynamic::setMass(f32 mass)
	{
		m_data->mass = mass;
	}

	void EntityDynamic::setRestitution(f32 r)
	{
		m_data->restitution = r;
	}

	void EntityDynamic::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateDynamicEntityShape, data);
	}

	void EntityDynamic::updateData()
	{
	}

	EntityShapeType EntityDynamic::getShapeType() const
	{
		return m_data->shape.type;
	}

	const vx::float2& EntityDynamic::getPosition() const { return m_data->position; }
	const vx::float2& EntityDynamic::getHalfDim() const { return m_data->shape.halfDim; }
	f32 EntityDynamic::getRadius() const { return m_data->shape.radius.x; }
}