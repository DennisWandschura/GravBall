#pragma once

#include "EditorEntityBase.h"
#include <gamelib/GravityArea.h>

namespace Editor
{
	class EntityGravityArea : public EntityBase
	{
		EditorGravityAreaFile* m_data;

		void updateShape() override;
		void updateData() override;

	public:
		EntityGravityArea(const QString &text, GameWidget* gameWidget, EditorGravityAreaFile* data);
		~EntityGravityArea();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2 &p);
		void setHalfDim(const vx::float2 &hd);
		void setForce(const vx::float2 &f);
		void setGravityAreaType(GravityAreaType type);
		void setTime(f32 time);

		EntityShapeType getShapeType() const override;

		const vx::float2& getPosition() const;
		const vx::float2& getHalfDim() const;
		const vx::float2& getForce() const;
		GravityAreaType getGravityAreaType() const;
		f32 getTime() const;
		const EditorGravityAreaFile* getData() const { return m_data; }

		static EntityType getStaticType() { return EntityType::GravityArea; }
	};
}