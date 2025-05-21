#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

constexpr int PLAYER_VERT_NUM = 15;
constexpr int BEETLE_VERT_NUM = 6;
constexpr int BULLET_VERT_NUM = 6;
constexpr int WASP_VERT_NUM = 6;
constexpr int ASTEROID_VERT_NUM = 48;
constexpr int DEBRIS_VERT_NUM = 24;
constexpr int GEAR_VERT_NUM = 32;

class VertexMap
{
public:
	VertexMap() { LoadAllPaintVertMap(); }
	~VertexMap() = default;

	std::vector<Vertex_PCU>RUN_VERT_MAP;
	std::vector<Vertex_PCU>RUN2_VERT_MAP;
	std::vector<Vertex_PCU>GEAR_VERT_MAP;
	std::vector<Vertex_PCU>QUIT_VERT_MAP;

	void LoadAllPaintVertMap();

private:
	void LoadPaintVertMapRun();
	void LoadPaintVertMapRun2();
	void LoadPaintVertMapGear();
	void LoadPaintVertMapQuit();
};
