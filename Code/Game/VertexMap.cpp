#include "Game/VertexMap.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <vector>

void VertexMap::LoadAllPaintVertMap()
{
	LoadPaintVertMapRun();
	LoadPaintVertMapRun2();
	LoadPaintVertMapGear();
	LoadPaintVertMapQuit();
}

void VertexMap::LoadPaintVertMapGear()
{
	GEAR_VERT_MAP.reserve( 96 );

	for (int index = 0; index < 8; index++)
	{
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 100.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 40.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index - 11.25f ) ) ) * 40.f,
			Rgba8::WHITE
		) );
		
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index - 11.25f ) ) ) * 40.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index - 11.25f ) ) ) * 100.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 100.f,
			Rgba8::WHITE
		) );

		//-----------------------------------------------
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 33.75f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 33.75f ) ) ) * 40.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 40.f,
			Rgba8::WHITE
		) );

		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 40.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 11.25f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
		GEAR_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 45.f * index + 33.75f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
	}
}

void VertexMap::LoadPaintVertMapRun()
{
	RUN_VERT_MAP.reserve( 3 );

	RUN_VERT_MAP.push_back( Vertex_PCU( +80.0f, +00.0f, 255, 255, 255, 255 ) );
	RUN_VERT_MAP.push_back( Vertex_PCU( -80.0f, +80.0f, 255, 255, 255, 255 ) );
	RUN_VERT_MAP.push_back( Vertex_PCU( -80.0f, -80.0f, 255, 255, 255, 255 ) );
}

void VertexMap::LoadPaintVertMapRun2()
{
	RUN2_VERT_MAP.reserve( 3 );

	RUN2_VERT_MAP.push_back( Vertex_PCU( +80.0f, +00.0f, 255, 255, 255, 255 ) );
	RUN2_VERT_MAP.push_back( Vertex_PCU( -80.0f, +80.0f, 255, 255, 255, 255 ) );
	RUN2_VERT_MAP.push_back( Vertex_PCU( -80.0f, -80.0f, 255, 255, 255, 255 ) );
}

void VertexMap::LoadPaintVertMapQuit()
{
	QUIT_VERT_MAP.reserve( 78 );

	for (float index = 5.75; index < 17.75; index++)
	{
		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index + 12.f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index + 12.f ) ) ) * 50.f,
			Rgba8::WHITE
		) );
		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index - 12.f ) ) ) * 50.f,
			Rgba8::WHITE
		) );

		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index - 12.f ) ) ) * 50.f,
			Rgba8::WHITE
		) );
		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index - 12.f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
		QUIT_VERT_MAP.push_back( Vertex_PCU(
			Vec3( Vec2( TransformPolarToCarteDegreeNormal( 24.f * index + 12.f ) ) ) * 80.f,
			Rgba8::WHITE
		) );
	}

	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( +15.f, 100.f ) ),
		Rgba8::WHITE
	) );
	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( +15.f, 10.f ) ),
		Rgba8::WHITE
	) );
	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( -15.f, 10.f ) ),
		Rgba8::WHITE
	) );

	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( -15.f, 10.f ) ),
		Rgba8::WHITE
	) );
	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( -15.f, 100.f ) ),
		Rgba8::WHITE
	) );
	QUIT_VERT_MAP.push_back( Vertex_PCU(
		Vec3( Vec2( +15.f, 100.f ) ),
		Rgba8::WHITE
	) );
}
