#pragma once

#include "EditorEntityBase.h"

namespace Editor
{
	struct GoalEntityFile;

	class EntityGoal : public EntityBase
	{
		GoalEntityFile* m_data;

		void updateShape() override;
		void updateData() override;

	public:
		EntityGoal(const QString &text, GameWidget* gameWidget, GoalEntityFile* data);
		~EntityGoal();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2a &p);
		void setHalfDim(const vx::float2a &hd);

		EntityShapeType getShapeType() const override;

		const vx::float2a& getPosition() const;
		const vx::float2a& getHalfDim() const;

		static EntityType getStaticType() { return EntityType::Goal; }

		const GoalEntityFile* getData() const { return m_data; }
	};
}