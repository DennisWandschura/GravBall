#pragma once

#include "EditorEntityBase.h"
#include <gamelib/EntityFile.h>

namespace Editor
{
	class EntityStatic : public EntityBase
	{
		EditorStaticEntityFile* m_data;

		void updateShape() override;
		void updateData() override;

	public:
		EntityStatic(const QString &text, GameWidget* gameWidget, EditorStaticEntityFile* data);
		~EntityStatic();

		void setupUI(EntityDataWidgets* widgets) override;

		void setPosition(const vx::float2 &p);
		void setHalfDim(const vx::float2 &hd);
		void setRadius(f32 r);

		EntityShapeType getShapeType() const override;

		const vx::float2& getPosition() const;
		const vx::float2& getHalfDim() const;
		f32 getRadius() const;

		const EditorStaticEntityFile* getData() const { return m_data; }

		static EntityType getStaticType() { return EntityType::Static; }
	};
}