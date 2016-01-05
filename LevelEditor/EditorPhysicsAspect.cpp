#include "EditorPhysicsAspect.h"
#include <gamelib/PhysicsComponent.h>
#include <gamelib/MainAllocator.h>
#include <gamelib/Event.h>
#include <vxLib/math/matrix.h>
#include <gamelib/EntityFile.h>
#include <gamelib/GravityWell.h>
#include <gamelib/gravityarea.h>

EditorPhysicsAspect::EditorPhysicsAspect()
	:m_dynamicComponents(),
	m_staticCircleComponents(),
	m_staticRectComponents()
{

}

EditorPhysicsAspect::~EditorPhysicsAspect()
{

}

bool EditorPhysicsAspect::initialize(const PhysicsAspectBaseDesc &desc)
{
	m_dynamicComponents = vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>(desc.allocator, desc.capacityDynamic);
	m_staticCircleComponents = vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>(desc.allocator, desc.capacityStatic);
	m_staticRectComponents = vx::array<PhysicsComponent, DebugAllocatorPhysics<Allocator::MainAllocator>>(desc.allocator, desc.capacityStatic);

	return true;
}

void EditorPhysicsAspect::shutdow(Allocator::MainAllocator* allocator)
{
	m_staticRectComponents.release();
	m_staticCircleComponents.release();
	m_dynamicComponents.release();
}

void EditorPhysicsAspect::update(f32 dt)
{
	const __m128 vdt = { dt, dt, dt, dt };

	for (auto &it : m_dynamicComponents)
	{
		auto force = vx::loadFloat2a(&it.m_force);
		auto position = vx::loadFloat2a(&it.m_position);

		position = _mm_add_ps(position, force);

		vx::storeFloat2a(&it.m_force, force);
		vx::storeFloat2a(&it.m_position, position);
	}
}

PhysicsComponent* EditorPhysicsAspect::addDynamicShape(const vx::float2a &p, f32 radius, f32 mass, f32 restitution, void* userData)
{
	PhysicsComponent component{};
	component.m_position = p;
	component.m_userData = userData;
	component.m_invMass = (mass == 0.0f) ? 0.0f : 1.0f / mass;
	component.m_radius.x = radius;
	component.m_shapeType = EntityShapeType::Circle;
	component.m_restitution = restitution;
	component.m_type = PhysicsType::Dynamic;
	component.m_userData = userData;

	m_dynamicComponents.push_back(component);

	return &m_dynamicComponents.back();
}

PhysicsComponent* EditorPhysicsAspect::addStaticShape(const vx::float2a &p, f32 radius, f32 restitution, void* userData)
{
	PhysicsComponent component{};
	component.m_position = p;
	component.m_shapeType = EntityShapeType::Circle;
	component.m_radius = { radius, radius };
	component.m_restitution = restitution;
	component.m_invMass = 0;
	component.m_type = PhysicsType::Static;
	component.m_userData = userData;

	m_staticCircleComponents.push_back(component);

	assert(false);

	return &m_staticCircleComponents.back();
}

PhysicsComponent* EditorPhysicsAspect::addStaticShape(const vx::float2a &p, const vx::float2a &halfDim, f32 restitution, void* userData)
{
	PhysicsComponent component{};
	component.m_position = p;
	component.m_shapeType = EntityShapeType::Rectangle;
	component.m_halfDim = halfDim;
	component.m_restitution = restitution;
	component.m_invMass = 0;
	component.m_type = PhysicsType::Static;
	component.m_userData = userData;

	m_staticRectComponents.push_back(component);

	return &m_staticRectComponents.back();
}

void EditorPhysicsAspect::handleEditorEvent(const Event::Event &evt)
{
	auto type = evt.asEditorEvent();
	switch (type)
	{
	case Event::Type::EditorEventTypes::CreateStaticEntity:
	{
		EditorStaticEntityFile* entity = (EditorStaticEntityFile*)evt.getData().ptr;
		if (entity->createdPhysics == 0)
		{
			entity->physicsComponent = addStaticShape(entity->position, entity->shape.halfDim, 0, nullptr);
			entity->physicsOffset = (u32)(entity->physicsComponent - m_staticRectComponents.data());
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateStaticEntityShape:
	{
		EditorStaticEntityFile* entity = (EditorStaticEntityFile*)evt.getData().ptr;
		if (entity->shape.type == EntityShapeType::Rectangle)
		{
			m_staticRectComponents[entity->physicsOffset].m_position = entity->position;
			m_staticRectComponents[entity->physicsOffset].m_halfDim = entity->shape.halfDim;
		}
	}break;
	case Event::Type::EditorEventTypes::CreateDynamicEntity:
	{
		EditorDynamicEntityFile* entity = (EditorDynamicEntityFile*)evt.getData().ptr;
		if (entity->createdPhysics == 0)
		{
			auto p = addDynamicShape(entity->position, entity->shape.radius.x, entity->mass, entity->restitution, nullptr);
			entity->physicsIndex = (u32)(p - m_dynamicComponents.data());
			entity->physicsComponent = p;
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateDynamicEntityShape:
	{
		EditorDynamicEntityFile* entity = (EditorDynamicEntityFile*)evt.getData().ptr;
		if (entity->shape.type == EntityShapeType::Circle)
		{
			m_dynamicComponents[entity->physicsIndex].m_position = entity->getPosition();
			m_dynamicComponents[entity->physicsIndex].m_radius.x = entity->getRadius();
		}
	}break;
	case Event::Type::EditorEventTypes::CreateSpawn:
	{
		auto entity = (Editor::SpawnEntityFile*)evt.getData().ptr;
		if (entity->createdPhysics == 0)
		{
			entity->physicsComponent = addStaticShape(entity->position, vx::float2(1), 0, nullptr);
			entity->physicsOffset = (u32)(entity->physicsComponent - m_staticRectComponents.data());
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateSpawnPosition:
	{
		auto entity = (Editor::SpawnEntityFile*)evt.getData().ptr;
		m_staticRectComponents[entity->physicsOffset].m_position = entity->position;
	}break;
	case Event::Type::EditorEventTypes::CreateGoal:
	{
		auto entity = (Editor::GoalEntityFile*)evt.getData().ptr;
		if (entity->createdPhysics == 0)
		{
			entity->physicsComponent = addStaticShape(entity->position, entity->halfDim, 0, nullptr);
			entity->physicsOffset = (u32)(entity->physicsComponent - m_staticRectComponents.data());
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGoalShape:
	{
		auto entity = (Editor::GoalEntityFile*)evt.getData().ptr;
		m_staticRectComponents[entity->physicsOffset].m_position = entity->position;
		m_staticRectComponents[entity->physicsOffset].m_halfDim = entity->halfDim;
	}break;
	case Event::Type::EditorEventTypes::CreateGravityArea:
	{
		auto entity = (EditorGravityAreaFile*)evt.getData().ptr;
		if (entity->createdPhysics == 0)
		{
			entity->physicsComponent = addStaticShape(entity->position, entity->halfDim, 0, nullptr);
			entity->physicsOffset = (u32)(entity->physicsComponent - m_staticRectComponents.data());
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGravityAreaShape:
	{
		auto entity = (EditorGravityAreaFile*)evt.getData().ptr;
		m_staticRectComponents[entity->physicsOffset].m_position = entity->position;
		m_staticRectComponents[entity->physicsOffset].m_halfDim = entity->halfDim;
	}break;
	case Event::Type::EditorEventTypes::CreateGravityWell:
	{
		auto entity = (EditorGravityWellFile*)evt.getData().ptr;
		if(entity->createdPhysics == 0)
		{
			entity->physicsComponent = addStaticShape(entity->position, vx::float2(entity->radius, entity->radius), 0, nullptr);
			entity->physicsOffset = (u32)(entity->physicsComponent - m_staticRectComponents.data());
			entity->createdPhysics = 1;
		}
	}break;
	case Event::Type::EditorEventTypes::UpdateGravityWellShape:
	{
		auto entity = (EditorGravityWellFile*)evt.getData().ptr;
		m_staticRectComponents[entity->physicsOffset].m_position = entity->position;
		m_staticRectComponents[entity->physicsOffset].m_radius = { entity->radius,entity->radius };
	}break;
	default:
		break;
	}
}

void EditorPhysicsAspect::handleEvent(const Event::Event &evt)
{
	if (evt.getType() == Event::Type::EventTypeEditor)
	{
		handleEditorEvent(evt);
	}
}

vx::float4a getRayDir(int x, int y, const vx::float2 &resolution, const vx::mat4 &viewMatrix, const vx::mat4 &projMatrix)
{
	f32 ndc_x = f32(x) / resolution.x;
	f32 ndc_y = f32(y) / resolution.y;

	ndc_x = ndc_x * 2.0f - 1.0f;
	ndc_y = 1.0f - ndc_y * 2.0f;

	auto invProjMatrix = vx::MatrixInverse(projMatrix);

	vx::float4a ray_clip(ndc_x, ndc_y, -1, 1);
	vx::float4a ray_eye = vx::Vector4Transform(invProjMatrix, ray_clip);
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);

	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::normalize3(ray_world);

	return ray_world;
}

struct Box
{
	vx::float3 min, max;
};

struct Sphere
{
	vx::float3 c;
	f32 r;
};

int testRayAABB(const vx::float3 &p, const vx::float3 &d, const Box &a)
{
	f32 tmin = 0;
	f32 tmax = FLT_MAX;

	auto ood = 1.0f / d;
	auto t1 = (a.min - p) * ood;
	auto t2 = (a.max - p) * ood;

	if (t1.x > t2.x) std::swap(t1.x, t2.x);
	if (t1.y > t2.y) std::swap(t1.y, t2.y);
	if (t1.z > t2.z) std::swap(t1.z, t2.z);

	tmin = vx::max(tmin, t1.x);
	tmin = vx::max(tmin, t1.y);
	tmin = vx::max(tmin, t1.z);

	tmax = vx::min(tmax, t2.x);
	tmax = vx::min(tmax, t2.y);
	tmax = vx::min(tmax, t2.z);

	if (tmin > tmax)return 0;

	return 1;
}

int testRaySphere(const vx::float3 &p, const vx::float3 &d, const Sphere &s)
{
	auto m = p - s.c;

	auto c = vx::dot3(m, m) - s.r * s.r;
	if (c <= 0.0f)return 1;

	auto b = vx::dot3(m, d);
	if (b > 0.0f) return 0;

	auto disc = b*b - c;
	if (disc < 0.0f)return 0;

	return 1;
}

PhysicsComponent* EditorPhysicsAspect::getShape(int x, int y, const vx::float2 &resolution, const __m128 &cameraPosition, const vx::mat4 &viewMatrix, const vx::mat4 &projMatrix)
{
	auto rayDir = getRayDir(x, y, resolution, viewMatrix, projMatrix);

	vx::float3 p;
	vx::storeFloat3(&p, cameraPosition);

	vx::float3 d(rayDir.x, rayDir.y, rayDir.z);

	//x = x - resolution.x / 2;
	//y = resolution.y / 2 - y;

	PhysicsComponent* result = nullptr;
	for (auto &it : m_staticRectComponents)
	{
		auto center = it.m_position;
		auto hd = it.m_halfDim;

		Box bounds;
		bounds.min.x = center.x - hd.x;
		bounds.min.y = center.y - hd.y;
		bounds.min.z = -0.1f;

		bounds.max.x = center.x + hd.x;
		bounds.max.y = center.y + hd.y;
		bounds.max.z = 0.1f;

		if (testRayAABB(p, d, bounds))
		{
			result = &it;
			break;
		}
	}

	if (result == nullptr)
	{
		for (auto &it : m_dynamicComponents)
		{
			if (it.m_shapeType == EntityShapeType::Rectangle)
			{
				auto center = it.m_position;
				auto hd = it.m_halfDim;
				Box bounds;
				bounds.min.x = center.x - hd.x;
				bounds.min.y = center.y - hd.y;
				bounds.min.z = -0.1f;

				bounds.max.x = center.x + hd.x;
				bounds.max.y = center.y + hd.y;
				bounds.max.z = 0.1f;

				if (testRayAABB(p, d, bounds))
				{
					result = &it;
					break;
				}
			}
			else
			{
				Sphere s;
				s.c = vx::float3(it.m_position, 0);
				s.r = it.m_radius.x;
				if (testRaySphere(p, d, s))
				{
					result = &it;
					break;
				}
			}
		}
	}

	return result;
}