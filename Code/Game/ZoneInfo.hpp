#pragma once

#include <vector>
#include <string>

#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Quat.hpp"

class ZoneInfo
{
public:
	struct SpawnInfo
	{
		std::string m_name;
		Vec3 m_position;
		Quat m_orientation;
		Vec3 m_scale;
	};
	std::vector<SpawnInfo> m_spawnInfo;
};
