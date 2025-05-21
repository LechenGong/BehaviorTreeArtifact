#include "Game/App.hpp"
#include "Game/VertexMap.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Renderer/ImGuiSystem.hpp"

Window* g_theWindow = nullptr;
App* g_theApp = nullptr;
//Renderer* g_theRenderer = nullptr;
RandomNumberGenerator* g_theRNG = nullptr;
InputSystem* g_theInput = nullptr;
AudioSystem* g_theAudio = nullptr;
VertexMap* g_theMap = nullptr;
ImGuiSystem* g_ImGui = nullptr;
GameConfig App::g_gameConfig = {};

double App::g_actualFrameTime = 0.00001f;

App::App()
{
}

App::~App() {}

void App::Startup( const char* commandLine )
{

	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem( eventSystemConfig );
	g_eventSystem->Startup();
	g_eventSystem->SubscribeEventCallBackFunc( "LoadGameConfig", &EventLoadGameConfig );

	LoadGameConfig( "" );

	if (commandLine && commandLine[0] != '\0')
	{
		Strings contents = Split( commandLine, ' ', true, 1 );
		if (contents.size() == 1)
		{
			g_eventSystem->FireEventEX( contents[0] );
		}
		else if (contents.size() == 2)
		{
			g_eventSystem->FireEventEX( contents[0], contents[1].c_str() );
		}
	}

	g_theMap = new VertexMap;
	InputConfig inputConfig;
	g_theInput = new InputSystem( inputConfig );
	g_theInput->Startup();
	g_theInput->LoadKeyBindingsFromXml();
	g_theInput->LoadKeyNameLookUp();

	NetSystemConfig netSystemConfig = {
		g_gameConfig.netMode,
		g_gameConfig.netSendBufferSize,
		g_gameConfig.netRecvBufferSize,
		g_gameConfig.netHostAddress
	};
	g_netSystem = new NetSystem( netSystemConfig );
	g_netSystem->Startup();
	g_netSystem->Activate();

	WindowConfig windowConfig = {
		g_theInput,
		g_gameConfig.windowTitle,
		g_gameConfig.windowAspect,
		g_gameConfig.windowFullScreen,
	};
	g_theWindow = new Window( windowConfig );
	g_theWindow->StartUp();

	RenderConfig renderConfig = {
		g_theWindow
	};
	g_theRenderer = new Renderer( renderConfig );
	g_theRenderer->Startup();

	JobSystemConfig jobSystemConfig = {
		-1
	};
	g_jobSystem = new JobSystem( jobSystemConfig );
	g_jobSystem->Startup();

	g_theRNG = new RandomNumberGenerator;

	AudioConfig audioConfig;
	g_theAudio = new AudioSystem( audioConfig );
	g_theAudio->Startup();


	DevConsoleConfig devConsoleConfig = {
		g_theRenderer,
		&m_devCamera,
		g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" ),
		25.5f,
		0.57f * g_theWindow->GetAspect(),
		100,
		300,
		false,
	};
	g_devConsole = new DevConsole( devConsoleConfig );
	g_devConsole->Startup();

	DebugRenderConfig debugRenderConfig = {
		g_theRenderer,
		g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" ),
	};
	DebugRenderSystemStartup( debugRenderConfig );

	g_ImGui = new ImGuiSystem( g_theRenderer, g_theWindow );
	g_ImGui->Startup();

	m_game = new GameAttract;
	m_game->Startup();

	m_devCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );

	g_eventSystem->SubscribeEventCallBackFunc( "pause", reinterpret_cast<void(*)()>(TogglePause), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "shutdown", reinterpret_cast<void(*)()>(ToggleShuttingDown), 1 );
	g_eventSystem->SubscribeEventCallBackFunc( "control", reinterpret_cast<void(*)()>(PrintAllControls), 1 );

	g_eventSystem->SubscribeEventCallBackFunc( "funcinputdown", reinterpret_cast<void(*)()>(DevConsoleFunctionKey), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "litinput", reinterpret_cast<void(*)()>(DevConsoleLiteralKey), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "funcinputdown", reinterpret_cast<void(*)()>(InputSystemKeyDown), 2 );
	g_eventSystem->SubscribeEventCallBackFunc( "funcinputup", reinterpret_cast<void(*)()>(InputSystemKeyUp), 2 );

	g_eventSystem->SubscribeEventCallBackFunc( "set", &testSetValue );

	g_eventSystem->FireEvent( "control" );
}

void App::Run()
{
	while (!g_theApp->IsShuttingDown())
	{
		g_theApp->RunFrame();

		//SwapBuffers( reinterpret_cast<HDC>(g_theWindow->GetDeviceContext()) );
	}
}

void App::Shutdown()
{
	delete g_ImGui;

	DebugRenderSystemShutdown();

	g_devConsole->Shutdown();
	delete g_devConsole;

	m_game->Shutdown();
	delete m_game;

	g_theAudio->Shutdown();
	delete g_theAudio;

	g_jobSystem->Shutdown();
	delete g_jobSystem;

	g_theRenderer->Shutdown();
	delete g_theRenderer;

	g_theWindow->Shutdown();
	delete g_theWindow;

	g_netSystem->Shutdown();
	delete g_netSystem;

	g_eventSystem->Shutdown();
	delete g_eventSystem;

	g_theInput->Shutdown();
	delete g_theInput;
}

void App::BeginFrame()
{
	g_theInput->BeginFrame();
	g_eventSystem->BeginFrame();
	g_netSystem->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	g_jobSystem->BeginFrame();
	g_theAudio->BeginFrame();
	g_devConsole->BeginFrame();
	g_ImGui->BeginFrame();
	Clock::TickSystemClock();
}

void App::Reboot()
{
	m_game->Shutdown();
	delete m_game;
	m_game = new GameAttract;
	m_game->Startup();
}

void App::InputResponse()
{
	if (g_theInput->IsKeyDown( 'T' ) ||
		g_theInput->GetController( 0 ).IsButtonDown( XboxButtonID::RS ))
	{
		m_game->m_clock->SetTimeScale( 0.1f );
	}
	if (g_theInput->IsNewKeyPressed( 'P' ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::Y ))
	{
		m_game->m_clock->TogglePause();
	}
	if (g_theInput->IsNewKeyPressed( 'O' ))
	{
		m_game->m_clock->StepSingleFrame();
	}

	if (g_theInput->WasKeyJustReleased( 'T' ) ||
		g_theInput->GetController( 0 ).WasButtonJustReleased( XboxButtonID::RS ))
	{
		m_game->m_clock->SetTimeScale( 1.f );
	}

// 	if (g_theInput->IsNewKeyPressed( KEYCODE_F8 ))
// 	{
// 		Reboot();
// 	}
}

void App::LoadGameConfig( const char* sourcePath )
{
	XmlDocument mapDefXml;
	const char* filePath = (sourcePath == nullptr || sourcePath[0] == '\0') ? "Data/GameConfig.xml" : sourcePath;
	XmlError xmlResult = mapDefXml.LoadFile( filePath );
	GUARANTEE_OR_DIE( xmlResult == tinyxml2::XML_SUCCESS, Stringf( "failed to load xml file" ) );
	XmlElement* rootElement = mapDefXml.RootElement();
	GUARANTEE_OR_DIE( rootElement, Stringf( "rootElement is nullptr" ) );
	g_gameConfig.musicVolume = ParseXmlAttribute( *rootElement, "musicVolume", 1.f );
	g_gameConfig.sfxVolume = ParseXmlAttribute( *rootElement, "sfxVolume", 1.f );

	g_gameConfig.netMode = ParseXmlAttribute( *rootElement, "netMode", g_gameConfig.netMode );
	g_gameConfig.netSendBufferSize = ParseXmlAttribute( *rootElement, "netSendBufferSize", g_gameConfig.netSendBufferSize );
	g_gameConfig.netRecvBufferSize = ParseXmlAttribute( *rootElement, "netRecvBufferSize", g_gameConfig.netRecvBufferSize );
	g_gameConfig.netHostAddress = ParseXmlAttribute( *rootElement, "netHostAddress", g_gameConfig.netHostAddress );

	g_gameConfig.windowAspect = ParseXmlAttribute( *rootElement, "windowAspect", g_gameConfig.windowAspect );
	g_gameConfig.windowFullScreen = ParseXmlAttribute( *rootElement, "windowFullScreen", g_gameConfig.windowFullScreen );
	g_gameConfig.windowSize = ParseXmlAttribute( *rootElement, "windowSize", g_gameConfig.windowSize );
	if (g_gameConfig.windowSize == IntVec2( -1, -1 )) g_gameConfig.windowSize = IntVec2( 1382, 691 );
	g_gameConfig.windowPosition = ParseXmlAttribute( *rootElement, "windowPosition", g_gameConfig.windowPosition );
	if (g_gameConfig.windowPosition == IntVec2( -1, -1 )) g_gameConfig.windowPosition = IntVec2( -1, -1 );
	g_gameConfig.windowTitle = ParseXmlAttribute( *rootElement, "windowTitle", g_gameConfig.windowTitle );
}

void App::Update()
{
	m_frameStartTime = GetCurrentTimeSeconds();

	InputResponse();
	
	if ((HWND)g_theWindow->GetHwnd() != GetActiveWindow() ||
		g_devConsole->GetMode() == DISPLAY ||
		m_game->GetGameMode() == (int)AppState::IN_ATTRACT ||
		g_theInput->IsKeyDown( KEYCODE_F8 ))
	{
		g_theInput->SetCursorMode( false, false );
	}
	else
	{
		g_theInput->SetCursorMode( true, true );
	}

	float deltaSeconds = Clock::s_systemClock.GetDeltaSeconds();
	UpdateGame( deltaSeconds );
	if (IsQuitting())
	{
		if (m_game->GetGameMode() == (int)AppState::IN_ATTRACT)
		{
			m_isShuttingDown = true;
		}
		else
		{
			Reboot();
		}
		SetQuitting( false );
	}
	m_age += deltaSeconds;
}

void App::Render() const
{
	g_theRenderer->ClearScreen( Rgba8( 50, 50, 50, 255 ) );

	m_game->Render();

	g_devConsole->Render( m_devCamera.GetBounds() );

	g_actualFrameTime = GetCurrentTimeSeconds() - m_frameStartTime;
}

void App::EndFrame()
{
	g_theInput->EndFrame();
	g_eventSystem->EndFrame();
	g_netSystem->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	g_jobSystem->EndFrame();
	g_theAudio->EndFrame();
	g_devConsole->EndFrame();
	g_ImGui->EndFrame();

	if (m_nextState != AppState::STATE_COUNT)
	{
		m_game->Shutdown();
		delete m_game;
		switch (m_nextState)
		{
		case AppState::IN_GAME:
			m_game = new GameRun;
			break;
		case AppState::IN_ATTRACT:
			m_game = new GameAttract;
			break;
		case AppState::IN_SETTING:
			m_game = new GameSetting;
			break;
		case AppState::STATE_COUNT:
			break;
		default:
			break;
		}
		m_game->Startup();
		m_nextState = AppState::STATE_COUNT;
	}
}

void App::RunFrame()
{
	BeginFrame();
	Update();
	Render();
	EndFrame();
}

void App::UpdateGame( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	m_game->Update( m_game->m_clock->GetDeltaSeconds() );
}

void App::SetState( AppState state )
{
	m_nextState = state;
}

bool TogglePause()
{
	g_theApp->SetPause( !g_theApp->IsPaused() );
	if (g_theApp->IsPaused())
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, "Game Paused" );
	}
	else
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, "Game Unpaused" );
	}
	return true;
}

bool ToggleShuttingDown()
{
	g_theApp->SetShuttingDown( !g_theApp->IsShuttingDown() ); 
	return true;
}

bool PrintAllControls()
{
	std::unordered_map<std::string, Key> keybinding = g_theInput->GetKeybinding();

	//for (auto iter = keybinding.begin(); iter != keybinding.end(); iter++)

	for (std::string str : g_theInput->m_keybindingOrder)
	{
		Key key = keybinding[str];
		if (key.value == 0)
			continue;
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, Stringf( "%s: %s", g_theInput->GetButtonNameByValue( key.value ).c_str(), g_theInput->GetKeyNameByValue( key.value ).c_str() ) );
	}
	
	return true;
}

bool EventLoadGameConfig( const char* command )
{
	std::string commandStr = command;
	if (commandStr.length() > 5 && commandStr.substr( 0, 5 ) == "File=")
	{
		std::string filePath = commandStr.substr( 5 );
		g_theApp->LoadGameConfig( filePath.c_str() );
	}
	return true;
}

bool testSetValue( const char* command )
{
	std::string commandStr = command;
	g_devConsole->AddLine( Rgba8::YELLOW, std::string( commandStr ) );
	Strings contents = SplitWithQuotation( commandStr, '=', true, true );
	if (contents.size() < 1) return false;

	g_devConsole->AddLine( Rgba8::CYAN, contents[0] + " -> " + contents[1] );
	return false;
}