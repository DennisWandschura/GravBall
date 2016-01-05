#pragma once

#include <gamelib/PhysicsComponent.h>
#include <gamelib/EntityFile.h>
#include <vector>

class StaticGrid
{
	struct Grid
	{

	};

public:
	void create(const PhysicsComponent* entities, size_t count)
	{
		std::vector<u8> used;
		used.reserve(count);
		for (size_t i = 0; i < count; ++i)
		{
			used.push_back(0);
		}

		size_t remaining = count;

		std::vector<vx::float2a> dims;

		vx::float2a minDim{1, 1};
		while (remaining != 0)
		{
			size_t insertedCount = 0;
			for (size_t i = 0; i < count; ++i)
			{
				auto use = used[i];
				if (use == 0)
				{
					auto &it = entities[i];

					if (it.m_shapeType == EntityShapeType::Circle)
					{
						auto radius = it.m_radius.x;
						if (radius <= minDim.x &&
							radius <= minDim.y)
						{
							--remaining;
							used[i] = 1;
							++insertedCount;
						}
					}
					else
					{
						auto halfDim = it.m_halfDim;
						if (halfDim.x <= minDim.x &&
							halfDim.y <= minDim.y)
						{
							--remaining;
							used[i] = 1;
							insertedCount++;
						}
					}
				}
			}

			if (insertedCount != 0)
			{
				dims.push_back(minDim);
			}

			minDim *= 2.0f;
		}
	}
};