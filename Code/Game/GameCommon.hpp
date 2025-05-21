#pragma once

#include <vector>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
// #include "Engine/Input/InputSystem.hpp"
// #include "Engine/Audio/AudioSystem.hpp"
//#include "Engine/Math/RandomNumberGenerator.hpp"

class Renderer;
class App;
class RandomNumberGenerator;
class InputSystem;
class VertexMap;
class AudioSystem;
class Window;
class VertexMap;
class ImGuiSystem;

extern Renderer* g_theRenderer;
extern App* g_theApp;
extern RandomNumberGenerator* g_theRNG;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern VertexMap* g_theMap;
extern ImGuiSystem* g_ImGui;

void AddVertsForStatusBar( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, float width, float height, bool upsideDown, Rgba8 const& color );

void AddVertsForStatusBarBorder( std::vector<Vertex_PCU>& verts, Vec2 const& bottomLeft, float width, float height, float borderWidth, bool reversed, Rgba8 const& borderColor );
