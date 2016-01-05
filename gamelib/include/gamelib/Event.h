#pragma once

enum class EntityShapeType : u16;

#include <vxLib/Variant.h>
#include <vxLib/math/Vector.h>

namespace Event
{
#if _EDITOR
	typedef void(*EventDataCallbackFn)(vx::Variant);
#endif

	namespace Editor
	{
		struct UpdateStaticEntityPosition
		{
			vx::float2a position;
			u32 transformIndex;
			EntityShapeType shapeType;
		};
	}

	struct RendererRebuildDynamicEntitiesData
	{
		void* data;
		size_t count;
	};

	struct RestartLevelData
	{
		u8 restartEntity;
		u8 restartPhysics;
		u8 restartRender;

		bool finished() const
		{
			return (restartEntity != 0) && (restartPhysics != 0) && (restartRender != 0);
		}
	};

	namespace Type
	{
		enum Types : u32
		{
			EventTypeInvalid = 0,
			EventTypeFile = 1 << 0,
			EventTypeIngame = 1 << 1,
			EventTypeEditor = 1 << 2
		};

		enum class FileEventTypes : u32
		{
			LoadedLevel,
			LoadTexture,
			LoadedTexture
		};

		enum class EditorEventTypes : u32
		{
			CreatePlayer,
			RendererCreatedPlayer,

			CreateSpawn,
			UpdateSpawnPosition,

			CreateGoal,
			UpdateGoalShape,

			CreateStaticEntity,
			UpdateStaticEntityShape,

			CreateDynamicEntity,
			UpdateDynamicEntityShape,

			CreateGravityArea,
			UpdateGravityAreaShape,

			CreateGravityWell,
			UpdateGravityWellShape
		};

		enum class IngameEventTypes : u32
		{
			DestroyDynamicEntity,
			RendererRebuildDynamicEntities,
			RestartLevelBegin,
			RestartLevel,
			RestartLevelFinished,
			FinishedLevel
		};
	}

	class Event
	{
		Type::Types m_type;
		u32 m_evtType;
		vx::Variant m_data;
#if _EDITOR
		EventDataCallbackFn m_callback;
#endif

	public:
#if _EDITOR
		Event() :m_type(), m_evtType(), m_data(), m_callback(nullptr) {}
		Event(Type::FileEventTypes evtType, vx::Variant data, EventDataCallbackFn callback) :m_type(Type::EventTypeFile), m_evtType((u32)evtType), m_data(data), m_callback(callback) {}
		Event(Type::EditorEventTypes evtType, vx::Variant data, EventDataCallbackFn callback) :m_type(Type::EventTypeEditor), m_evtType((u32)evtType), m_data(data), m_callback(callback) {}
		Event(Type::IngameEventTypes evtType, vx::Variant data, EventDataCallbackFn callback) :m_type(Type::EventTypeIngame), m_evtType((u32)evtType), m_data(data), m_callback(callback) {}
#else
		Event() : m_type(), m_evtType(), m_data() {}
		Event(Type::FileEventTypes evtType, vx::Variant data) : m_type(Type::EventTypeFile), m_evtType((u32)evtType), m_data(data) {}
		Event(Type::EditorEventTypes evtType, vx::Variant data) :m_type(Type::EventTypeEditor), m_evtType((u32)evtType), m_data(data) {}
		Event(Type::IngameEventTypes evtType, vx::Variant data) :m_type(Type::EventTypeIngame), m_evtType((u32)evtType), m_data(data) {}
#endif
		~Event() {}

#if _EDITOR
		void callback()
		{
			if (m_callback)
			{
				m_callback(m_data);
			}
		}

		EventDataCallbackFn getCallback() const { return m_callback; }
#endif

		Type::Types getType() const { return m_type; }

		vx::Variant& getData() { return m_data; }
		const vx::Variant& getData() const { return m_data; }

		Type::FileEventTypes asFileEvent() const { return (Type::FileEventTypes)m_evtType; }
		Type::EditorEventTypes asEditorEvent() const { return (Type::EditorEventTypes)m_evtType; }
		Type::IngameEventTypes asIngameEvent() const { return (Type::IngameEventTypes)m_evtType; }
	};
}