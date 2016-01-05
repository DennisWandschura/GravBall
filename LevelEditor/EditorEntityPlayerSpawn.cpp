#include "EditorEntityPlayerSpawn.h"
#include <gamelib/EntityShape.h>
#include <gamelib/EntityFile.h>
#include "EntityDataWidgets.h"
#include <gamelib/Event.h>
#include "GameWidget.h"

namespace Editor
{
	EntityPlayerSpawn::EntityPlayerSpawn(const QString &text, GameWidget* gameWidget, SpawnEntityFile* data)
		:EntityBase(text, gameWidget, EntityType::PlayerSpawn),
		m_data(data)
	{

	}

	EntityPlayerSpawn::~EntityPlayerSpawn()
	{
		if (m_data)
		{
			delete(m_data);
		}
	}

	void EntityPlayerSpawn::updateShape()
	{
		vx::Variant data;
		data.ptr = m_data;
		m_gameWidget->pushEditorEvent(Event::Type::EditorEventTypes::UpdateSpawnPosition, data);
	}

	void EntityPlayerSpawn::setupUI(EntityDataWidgets* widgets)
	{
		auto position = getPosition();

		widgets->m_widgetPosition->move(EntityDataWidgets::POSITION_X, EntityDataWidgets::POSITION_Y);
		widgets->m_widgetPosition->setValues(position.x, position.y);
		widgets->m_widgetPosition->show();
	}

	void EntityPlayerSpawn::setPosition(const vx::float2a &p)
	{
		m_data->position = p;
		updateShape();
	}

	const vx::float2a& EntityPlayerSpawn::getPosition() const
	{
		return m_data->position;
	}

	EntityShapeType EntityPlayerSpawn::getShapeType() const
	{
		return EntityShapeType::Rectangle;
	}
}