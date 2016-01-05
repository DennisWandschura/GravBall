#pragma once

#include "EditorEntityBase.h"
#include <gamelib/GravityWell.h>

namespace Editor
{
	class EntityGravityWell : public EntityBase
	{
		EditorGravityWellFile* m_data;

		void updateShape() override;
		void updateData() override;

	public:
		EntityGravityWell(const QString &text, GameWidget* gameWidget, EditorGravityWellFile* data);
		~EntityGravityWell();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2a &p);
		void setRadius(f32 r);
		void setForce(f32 f);

		EntityShapeType getShapeType() const override;

		const vx::float2a& getPosition() const;
		f32 getRadius() const;
		f32 getForce() const;

		const EditorGravityWellFile* getData() const { return m_data; }

		static EntityType getStaticType() { return EntityType::GravityWell; }
	};
}