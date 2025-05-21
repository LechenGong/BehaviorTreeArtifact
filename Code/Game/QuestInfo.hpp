#pragma once

#include <vector>
#include <string>

class QuestInfo
{
public:
	struct SpawnInfo
	{
		std::string m_name;
		int m_amount;
	};
	std::vector<SpawnInfo> m_spawnInfo;
};
