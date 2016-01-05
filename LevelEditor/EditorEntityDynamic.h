#pragma once

#include "EditorEntityBase.h"
#include <gamelib/EntityFile.h>

namespace Editor
{
	class EntityDynamic : public EntityBase
	{
		EditorDynamicEntityFile* m_data;
		int m_controller;

		void updateShape() override;
		void updateData() override;

	public:
		EntityDynamic(const QString &text, GameWidget* gameWidget, EditorDynamicEntityFile* data, int controller);
		~EntityDynamic();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2 &p);
		void setHalfDim(const vx::float2 &hd);
		void setRadius(f32 r);

		void setMass(f32 mass);
		void setRestitution(f32 r);

		void setController(int i) { m_controller = i; }

		EntityShapeType getShapeType() const override;

		const vx::float2& getPosition() const;
		const vx::float2& getHalfDim() const;
		f32 getRadius() const;
		const EditorDynamicEntityFile* getData() const { return m_data; }
		int getController() const { return m_controller; }

		static EntityType getStaticType() { return EntityType::Dynamic; }
	};
}