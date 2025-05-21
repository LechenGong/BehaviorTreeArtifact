#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in VERY few places (and .CPPs only)
#include <cassert>
#include <crtdbg.h>

#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED( applicationInstanceHandle );

	g_theApp = new App();
	g_theApp->Startup( commandLineString );
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}
