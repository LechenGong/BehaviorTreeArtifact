#include "Game/GameCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"

void AddVertsForStatusBar( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeftOffset, float width, float height, bool upsideDown, Rgba8 const& color )
{
	Vec2 bottomLeft = bottomLeftOffset + (upsideDown ? Vec2( height, 0 ) : Vec2::ZERO);
	Vec2 bottomRight = bottomLeft + Vec2( width, 0 );
	Vec2 topLeft = bottomLeft + Vec2( height * (upsideDown ? -1.f : 1.f), height );
	Vec2 topRight = bottomRight + Vec2( height * (upsideDown ? -1.f : 1.f), height );

	verts.push_back( Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0, 0 ) ) );
	verts.push_back( Vertex_PCU( Vec3( bottomRight ), color, Vec2( 1, 0 ) ) );
	verts.push_back( Vertex_PCU( Vec3( topRight ), color, Vec2( 1, 1 ) ) );

	verts.push_back( Vertex_PCU( Vec3( topRight ), color, Vec2( 1, 1 ) ) );
	verts.push_back( Vertex_PCU( Vec3( topLeft ), color, Vec2( 0, 1 ) ) );
	verts.push_back( Vertex_PCU( Vec3( bottomLeft ), color, Vec2( 0, 0 ) ) );
}

void AddVertsForStatusBarBorder( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeftOffset, float width, float height, float borderWidth, bool upsideDown, Rgba8 const& borderColor )
{
	Vec2 bottomLeft = bottomLeftOffset + (upsideDown ? Vec2( height, 0 ) : Vec2::ZERO);
	Vec2 bottomRight = bottomLeft + Vec2( width, 0 );
	Vec2 topLeft = bottomLeft + Vec2( height * (upsideDown ? -1.f : 1.f), height );
	Vec2 topRight = bottomRight + Vec2( height * (upsideDown ? -1.f : 1.f), height );

	Vec2 outerBottomLeft = bottomLeft - Vec2( borderWidth * (upsideDown ? 0.414f : 2.414f), borderWidth );
	Vec2 outerBottomRight = bottomRight + Vec2( borderWidth * (!upsideDown ? 0.414f : 2.414f), -borderWidth );
	Vec2 outerTopLeft = topLeft - Vec2( borderWidth * (!upsideDown ? 0.414f : 2.414f), -borderWidth );
	Vec2 outerTopRight = topRight + Vec2( borderWidth * (upsideDown ? 0.414f : 2.414f), borderWidth );

	// bottom border
// 	verts.push_back( Vertex_PCU( Vec3( outerBottomLeft ), borderColor ) );
// 	verts.push_back( Vertex_PCU( Vec3( outerBottomRight ), borderColor ) );
// 	verts.push_back( Vertex_PCU( Vec3( bottomRight ), borderColor ) );
// 
// 	verts.push_back( Vertex_PCU( Vec3( bottomRight ), borderColor ) );
// 	verts.push_back( Vertex_PCU( Vec3( bottomLeft ), borderColor ) );
// 	verts.push_back( Vertex_PCU( Vec3( outerBottomLeft ), borderColor ) );

	AddVertsForRect2D( verts, outerBottomLeft, outerBottomRight, bottomRight, bottomLeft, borderColor );

	// top border
	verts.push_back( Vertex_PCU( Vec3( topLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( topRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerTopRight ), borderColor ) );

	verts.push_back( Vertex_PCU( Vec3( outerTopRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerTopLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( topLeft ), borderColor ) );

	// left border
	verts.push_back( Vertex_PCU( Vec3( outerBottomLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( bottomLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( topLeft ), borderColor ) );

	verts.push_back( Vertex_PCU( Vec3( topLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerTopLeft ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerBottomLeft ), borderColor ) );

	// right border
	verts.push_back( Vertex_PCU( Vec3( bottomRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerBottomRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( outerTopRight ), borderColor ) );

	verts.push_back( Vertex_PCU( Vec3( outerTopRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( topRight ), borderColor ) );
	verts.push_back( Vertex_PCU( Vec3( bottomRight ), borderColor ) );
}

