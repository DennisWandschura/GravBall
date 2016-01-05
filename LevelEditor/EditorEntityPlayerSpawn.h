#pragma once

#include "EditorEntityBase.h"

namespace Editor
{
	struct SpawnEntityFile;

	class EntityPlayerSpawn : public EntityBase
	{
		SpawnEntityFile* m_data;

		void updateShape() override;
		void updateData() override {}

	public:
		EntityPlayerSpawn(const QString &text, GameWidget* gameWidget, SpawnEntityFile* data);
		~EntityPlayerSpawn();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2a &p);

		EntityShapeType getShapeType() const override;

		const vx::float2a& getPosition() const;

		static EntityType getStaticType() { return EntityType::PlayerSpawn; }

		const SpawnEntityFile* getData() const { return m_data; }
	};
}