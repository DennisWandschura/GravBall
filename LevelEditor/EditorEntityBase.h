#pragma once

class GameWidget;
struct EntityDataWidgets;

#include <vxLib/math/Vector.h>
#include <qtreewidget.h>

enum class EntityShapeType : u16;

namespace Editor
{
	enum class EntityType :u32 { Dynamic, Static, GravityArea, GravityWell, PlayerSpawn, Goal};

	class EntityBase : public QTreeWidgetItem
	{
	protected:
		GameWidget* m_gameWidget;
		EntityType m_type;

		virtual void updateShape() = 0;
		virtual void updateData() = 0;

	public:
		EntityBase(const QString &text, GameWidget* gameWidget, EntityType entityType)
			: QTreeWidgetItem(),
			m_gameWidget(gameWidget),
			m_type(entityType)
		{
			setText(0, text);
		}

		virtual ~EntityBase() {}

		virtual void setupUI(EntityDataWidgets* widgets) = 0;

		EntityType getEntityType() const { return m_type; }

		virtual EntityShapeType getShapeType() const = 0;
	};
}