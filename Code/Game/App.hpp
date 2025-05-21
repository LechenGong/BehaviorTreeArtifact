#pragma once

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Renderer/Camera.hpp"

enum class AppState
{
	IN_ATTRACT,
	IN_GAME,
	IN_SETTING,
	STATE_COUNT
};

struct GameConfig
{
	float musicVolume = 1.f;
	float sfxVolume = 1.f;

	std::string netMode = "";
	int netSendBufferSize = 0;
	int netRecvBufferSize = 0;
	std::string netHostAddress = "";

	float windowAspect = 2.f;
	bool windowFullScreen = false;
	IntVec2 windowSize = IntVec2( 1382, 691 );
	IntVec2 windowPosition = IntVec2( -1, -1 );
	std::string windowTitle = "";
};

class App
{
public:
	App();
	~App();

	void Startup( const char* commandLine );
	void Shutdown();
	void Run();
	void RunFrame();
	void Reboot();
		 
	bool IsQuitting()	const { return m_isQuitting; }
	bool IsPaused()		const { return m_isPaused; }
	bool IsSlowed()		const { return m_isSlowed; }
	bool IsProceeding() const { return m_isProceeding; }
	bool IsShuttingDown() const { return m_isShuttingDown; }

	void SetPause( bool flag )		{ m_isPaused = flag; }
	void SetSlowed( bool flag )		{ m_isSlowed = flag; }
	void SetQuitting( bool flag )	{ m_isQuitting = flag; }
	void SetProceeding( bool flag ) { m_isProceeding = flag; }
	void SetShuttingDown( bool flag ) { m_isShuttingDown = flag; }

	int GetCurrentState() const { return m_game->GetGameMode(); }
	bool IsInState( AppState state ) const { return m_game->GetGameMode() == (int)state; }
	void SetState( AppState state );

	void LoadGameConfig( const char* sourcePath );

	static GameConfig g_gameConfig;
	static double g_actualFrameTime;
	bool m_disableInput = false;

	bool m_multiThreadingLoad = true;
	bool m_multiThreadingUpdate = false;
		 
private: 
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

	void UpdateGame( float deltaSeconds );

	void InputResponse();

	double m_frameStartTime;

	float m_VolumeSFX = 1.0f;
	float m_VolumeMusic = 1.0f;
	float m_screenShake = 1.0f;

	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowed = false;
	bool m_isProceeding = false;
	bool m_isShuttingDown = false;

	float m_age = 0.f;

	Game* m_game = nullptr;

	Camera m_devCamera;

	AppState m_nextState = AppState::STATE_COUNT;
};

bool TogglePause();
bool ToggleShuttingDown();
bool PrintAllControls();
bool EventLoadGameConfig( const char* command );

bool testSetValue( const char* command );
