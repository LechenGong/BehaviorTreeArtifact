#include <vector>
#include <string>
#include <filesystem>
#include <shared_mutex>

#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/VertexMap.hpp"
#include "Game/GameCharacter.hpp"
#include "Game/CharacterFactory.hpp"
#include "Game/PlayerController.hpp"
#include "Game/PlayerCharacter.hpp"
#include "Game/PlayerAnimController.hpp"
#include "Game/AI/Rathian/RathianCharacter.hpp"
#include "Game/AI/Rathian/RathianAnimController.hpp"
#include "Game/AI/Bullfango/BullfangoCharacter.hpp"
#include "Game/AI/Bullfango/BullfangoAnimController.hpp"

#include "Engine/General/Actor.hpp"
#include "Engine/General/CharacterDefinition.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Model/FBXImporter.hpp"
#include "Engine/Renderer/ImGuiSystem.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/General/ShapeComponents/ShapeComponent.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/General/StaticMeshComponent.hpp"
#include "Engine/General/StaticMesh.hpp"

#include "Engine/Model/FBXImporter.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"

#include "Engine/BehaviorTree/TreeNodes/TreeNodeFactory.h"
#include "Game/AI/GameTreeNodeFactory.hpp"
#include "Game/AI/Bullfango/BehaviorTree/BullfangoTreeNodeFactory.hpp"

std::shared_mutex g_animStatesMutex;
std::shared_mutex g_skeletonMeshesMutex;
std::shared_mutex g_assetsMutex;

class LoadAnimationJob : public Job
{
public:
	std::string m_charaName;
	std::filesystem::directory_entry m_filePath;

	AnimationState* newAnimState = nullptr;

public:
	LoadAnimationJob( std::string charaName, std::filesystem::directory_entry filePath )
	{
		m_charaName = charaName;
		m_filePath = filePath;
	}
	~LoadAnimationJob()
	{
	}
	void Execute() override
	{
		AnimationSequence* newAnim = AnimationSequence::ImportFromXML( m_filePath.path().string() );
		/*GameRun::g_animStates[m_charaName][m_filePath.path().stem().string()]*/ newAnimState = new AnimationState( newAnim, newAnim->m_name );
	}
};

class LoadSkeletonJob : public Job
{
public:
	std::string m_charaName;
	std::filesystem::directory_entry m_filePath;

	SkeletalMesh* newSkeletalMesh = nullptr;

public:
	LoadSkeletonJob( std::string charaName )
	{
		m_charaName = charaName;
	}
	~LoadSkeletonJob()
	{
	}
	void Execute() override
	{
		/*GameRun::g_skeletonMeshes[m_charaName]*/newSkeletalMesh = SkeletalMesh::ImportFromXML( GameRun::g_characterDefs[m_charaName].m_skeletalMeshPath );
	}
};

class LoadAssetJob : public Job
{
public:
	std::string m_assetName;
	std::string m_filePath;

	StaticMesh* newStaticMesh = nullptr;

public:
	LoadAssetJob( std::string assetName, std::string filePath )
	{
		m_assetName = assetName;
		m_filePath = filePath;
	}
	~LoadAssetJob()
	{
	}
	void Execute() override
	{
		std::string str = m_filePath.substr( 0, m_filePath.size() - 4 ) + ".xml";
		/*GameRun::g_staticMeshes[m_assetName]*/ newStaticMesh = StaticMesh::ImportFromXML( str );
	}
};

class ActorUpdateJob : public Job
{
public:
	Actor* m_actor;
	float m_deltaSeconds;
public:
	ActorUpdateJob( Actor* actor, float deltaSeconds )
	{
		m_actor = actor;
		m_deltaSeconds = deltaSeconds;
	}
	~ActorUpdateJob() {};
	void Execute() override
	{
		m_actor->Update( m_deltaSeconds );
	}
};

std::unordered_map<std::string, std::unordered_map<std::string, AnimationState*>> GameRun::g_animStates;
std::unordered_map<std::string, SkeletalMesh*> GameRun::g_skeletonMeshes;
std::unordered_map<std::string, std::vector<CollisionInfo>> GameRun::g_collisionComponents;
std::unordered_map<std::string, CharacterDefinition> GameRun::g_characterDefs;
std::unordered_map<std::string, StaticMesh*> GameRun::g_staticMeshes;
std::unordered_map<std::string, BehaviorTree*> GameRun::g_behaviorTrees;

GameRun::GameRun()
{
}

GameRun::~GameRun()
{
}

void GameRun::Startup()
{
	m_clock = new Clock;

	CharacterFactory::RegisterCharacterTypes();
	double k = GetCurrentTimeSeconds();
	InitializeCharacters();
	InitializeEnvironment();
	InitializeWorldCollisions();
	InitializeBehaviorTrees();

	Job* jobPointer = nullptr;
	while (((jobPointer = g_jobSystem->RetrieveJob()) != nullptr) || g_jobSystem->IsEmpty())
	{
		LoadAnimationJob* animJob = dynamic_cast<LoadAnimationJob*>(jobPointer);
		if (animJob)
		{
			GameRun::g_animStates[animJob->m_charaName][animJob->m_filePath.path().stem().string()] = animJob->newAnimState;
		}
		LoadSkeletonJob* skeletonJob = dynamic_cast<LoadSkeletonJob*>(jobPointer);
		if (skeletonJob)
		{
			GameRun::g_skeletonMeshes[skeletonJob->m_charaName] = skeletonJob->newSkeletalMesh;
		}
		LoadAssetJob* assetJob = dynamic_cast<LoadAssetJob*>(jobPointer);
		if (assetJob)
		{
			GameRun::g_staticMeshes[assetJob->m_assetName] = assetJob->newStaticMesh;
		}
		delete jobPointer;
	}

	SpawnCharacters();
	SpawnEnvironment();
	DebuggerPrintf( Stringf( "Total time consumed: %f\n", GetCurrentTimeSeconds() - k ).c_str() );

	// 	PlayerCharacter* chara1 = nullptr;
	// 	{
	// 		double k = GetCurrentTimeSeconds();
	// 
	// 		namespace fs = std::filesystem;
	// 		const fs::path xmlDir = "Data/Models/FBXs/Paladin/Xml/Animation";
	// 
	// 		for (const auto& entry : fs::directory_iterator( xmlDir ))
	// 		{
	// 			if (entry.is_regular_file() && entry.path().extension() == ".xml")
	// 			{
	// 				AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry.path().string() );
	// 				g_animStates["Paladin"][entry.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
	// 			}
	// 		}
	// 
	// // 		FBX fbx( "Data/Models/FBXs/Paladin/troll.fbx", g_theRenderer, true, true );
	// // 		chara1 = new PlayerCharacter( fbx.GetName(), fbx.m_meshes, fbx.m_skeleton, fbx.m_textures, g_theRenderer, this );
	// // 		Material* material = new Material( "Data/Materials/Paladin/Material.xml", g_theRenderer );
	// // 		chara1->GetSkeletalMesh()->BindMaterial( material );
	// 		SkeletalMesh* newSkeletalMesh = SkeletalMesh::ImportFromXML( "Data/Models/FBXs/Paladin/Xml/SkeletalMesh.xml" );
	// 		chara1 = new PlayerCharacter( "Paladin", newSkeletalMesh, this );
	// 		//chara1->GetSkeletalMesh()->ExportToXML( "Data/Models/FBXs/Paladin/Xml/PaladinSkeletalMesh.xml" );
	// 
	// 		chara1->m_animController->GetStateMachine()->SetInitialState( "Idle" );
	// 		DebuggerPrintf( Stringf( "Player Loading and Creation: %f\n", GetCurrentTimeSeconds() - k ).c_str() );
	// 	}
	// 
	// 	m_actors.push_back( chara1 );
	// 	
	// 	m_player = chara1;
	m_player->m_cameraArmPivotPos = Vec3( 0.f, 0.f, 1.f );
	m_player->m_cameraArmLength = 4.f;
	m_player->SetActorLocalPosition( Vec3( 7.f, 0.f, 22.5f ) );
	m_player->GetSkeletalMeshComponent()->SetLocalPosition( Vec3( 0.f, 0.f, 0.f ) );

	m_playerController = new PlayerController( m_player );
	m_timer = new Timer( 0.5f, m_clock );

	m_player->m_camera.SetPerspeciveView( g_theWindow->GetAspect(), 60.f, 0.1f, 200.f );
	m_player->m_camera.SetRenderBasis( Vec3( 0.f, 0.f, 1.f ), Vec3( -1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ) );
	m_player->m_camera.SetTransform( m_player->GetActorLocalPosition() + m_player->m_cameraArmPivotPos, m_player->GetActorLocalOrientation().ToEulerAngles() );

	// 	{
	// 		// 	FBX fbx("Data/Models/FBXs/Bullfango/Raw.fbx", g_theRenderer, true, true);
	// 		// 	Character* chara4 = new BullfangoCharacter( "Bullfango", fbx.m_meshes, fbx.m_skeleton, fbx.m_textures, g_theRenderer, this );
	// 		// 	Material* material = new Material( "Data/Materials/Bullfango/Material.xml", g_theRenderer );
	// 		// 	chara4->GetSkeletalMesh()->BindMaterial( material );
	// 			//chara4->GetSkeletalMesh()->ExportToXML( "Data/Models/FBXs/Bullfango/Xml/SkeletalMesh.xml" );
	// 		{
	// 			namespace fs = std::filesystem;
	// 			const fs::path animationDir = "Data/Models/FBXs/Bullfango/Animation/Combat";
	// 			const fs::path xmlDir = "Data/Models/FBXs/Bullfango/Xml/Combat";
	// 			//         	for (const auto& entry : fs::directory_iterator( animationDir ))
	// 			//         	{
	// 			//         		if (entry.is_regular_file() && entry.path().extension() == ".fbx")
	// 			//         		{
	// 			//         			FBX fbx( entry.path().string(), g_theRenderer, true, false, FBX::RootMotionConfig( true, true, false, true ) );
	// 			//         			fbx.m_animationSequences[0]->m_name = entry.path().stem().string();
	// 			//         			std::string outDir = xmlDir.string() + "/" + entry.path().stem().string() + ".xml";
	// 			//     				fbx.m_animationSequences[0]->m_looping = false;
	// 			//    					fbx.m_animationSequences[0]->m_playbackSpeed = 2.f;
	// 			//     
	// 			//         			fbx.m_animationSequences[0]->ExportToXML( outDir );
	// 			//   //  				AnimationSequence*& newAnim = fbx.m_animationSequences[0];
	// 			//   //  				g_animStates["Bullfango"][newAnim->m_name] = new AnimationState( newAnim, newAnim->m_name );
	// 			//   //  				chara4->m_animController->GetStateMachine()->AddState( g_animStates["Bullfango"][newAnim->m_name] );
	// 			//         		}
	// 			//         	}
	// 			for (const auto& entry : fs::directory_iterator( xmlDir ))
	// 			{
	// 				if (entry.is_regular_file() && entry.path().extension() == ".xml")
	// 				{
	// 					AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry.path().string() );
	// 					g_animStates["Bullfango"][entry.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
	// 				}
	// 			}
	// 		}
	// 		{
	// 			namespace fs = std::filesystem;
	// 			const fs::path animationDir = "Data/Models/FBXs/Bullfango/Animation/OffCombat";
	// 			const fs::path xmlDir = "Data/Models/FBXs/Bullfango/Xml/OffCombat";
	// 			//    		for (const auto& entry : fs::directory_iterator( animationDir ))
	// 			//    		{
	// 			//    			if (entry.is_regular_file() && entry.path().extension() == ".fbx")
	// 			//    			{
	// 			//    				FBX fbx( entry.path().string(), g_theRenderer, true, false, FBX::RootMotionConfig( true, true, false, true ) );
	// 			//    				fbx.m_animationSequences[0]->m_name = entry.path().stem().string();
	// 			//    				std::string outDir = xmlDir.string() + "/" + entry.path().stem().string() + ".xml";
	// 			// 				fbx.m_animationSequences[0]->m_looping = false;
	// 			//  				fbx.m_animationSequences[0]->m_playbackSpeed = 2.f;
	// 			//    				fbx.m_animationSequences[0]->ExportToXML( outDir );
	// 			//  //   				AnimationSequence*& newAnim = fbx.m_animationSequences[0];
	// 			//  //   				g_animStates["Bullfango"][newAnim->m_name] = new AnimationState( newAnim, newAnim->m_name );
	// 			//  //   				chara4->m_animController->GetStateMachine()->AddState( g_animStates["Bullfango"][newAnim->m_name] );
	// 			//    			}
	// 			//    		}
	// 			for (const auto& entry : fs::directory_iterator( xmlDir ))
	// 			{
	// 				if (entry.is_regular_file() && entry.path().extension() == ".xml")
	// 				{
	// 					AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry.path().string() );
	// 					g_animStates["Bullfango"][entry.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
	// 				}
	// 			}
	// 		}
	// 		{
	// 			namespace fs = std::filesystem;
	// 			const fs::path animationDir = "Data/Models/FBXs/Bullfango/Animation/Eco";
	// 			const fs::path xmlDir = "Data/Models/FBXs/Bullfango/Xml/Eco";
	// 			//    		for (const auto& entry : fs::directory_iterator( animationDir ))
	// 			//    		{
	// 			//    			if (entry.is_regular_file() && entry.path().extension() == ".fbx")
	// 			//    			{
	// 			//    				FBX fbx( entry.path().string(), g_theRenderer, true, false, FBX::RootMotionConfig( true, true, false, true ) );
	// 			//    				fbx.m_animationSequences[0]->m_name = entry.path().stem().string();
	// 			//    				std::string outDir = xmlDir.string() + "/" + entry.path().stem().string() + ".xml";
	// 			// 				fbx.m_animationSequences[0]->m_looping = false;
	// 			//  				fbx.m_animationSequences[0]->m_playbackSpeed = 2.f;
	// 			//    				fbx.m_animationSequences[0]->ExportToXML( outDir );
	// 			//  //   				AnimationSequence*& newAnim = fbx.m_animationSequences[0];
	// 			//  //   				g_animStates["Bullfango"][newAnim->m_name] = new AnimationState( newAnim, newAnim->m_name );
	// 			//  //   				chara4->m_animController->GetStateMachine()->AddState( g_animStates["Bullfango"][newAnim->m_name] );
	// 			//    			}
	// 			//    		}
	// 			for (const auto& entry : fs::directory_iterator( xmlDir ))
	// 			{
	// 				if (entry.is_regular_file() && entry.path().extension() == ".xml")
	// 				{
	// 					AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry.path().string() );
	// 					g_animStates["Bullfango"][entry.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
	// 				}
	// 			}
	// 		}
	// 		{
	// 			namespace fs = std::filesystem;
	// 			const fs::path animationDir = "Data/Models/FBXs/Bullfango/Animation/Injury";
	// 			const fs::path xmlDir = "Data/Models/FBXs/Bullfango/Xml/Injury";
	// 			//    		for (const auto& entry : fs::directory_iterator( animationDir ))
	// 			//    		{
	// 			//    			if (entry.is_regular_file() && entry.path().extension() == ".fbx")
	// 			//    			{
	// 			//    				FBX fbx( entry.path().string(), g_theRenderer, true, false, FBX::RootMotionConfig( true, true, false, true ) );
	// 			//    				fbx.m_animationSequences[0]->m_name = entry.path().stem().string();
	// 			//    				std::string outDir = xmlDir.string() + "/" + entry.path().stem().string() + ".xml";
	// 			// 				fbx.m_animationSequences[0]->m_looping = false;
	// 			//  				fbx.m_animationSequences[0]->m_playbackSpeed = 2.f;
	// 			//    				fbx.m_animationSequences[0]->ExportToXML( outDir );
	// 			//  //   				AnimationSequence*& newAnim = fbx.m_animationSequences[0];
	// 			//  //   				g_animStates["Bullfango"][newAnim->m_name] = new AnimationState( newAnim, newAnim->m_name );
	// 			//  //   				chara4->m_animController->GetStateMachine()->AddState( g_animStates["Bullfango"][newAnim->m_name] );
	// 			//    			}
	// 			//    		}
	// 			for (const auto& entry : fs::directory_iterator( xmlDir ))
	// 			{
	// 				if (entry.is_regular_file() && entry.path().extension() == ".xml")
	// 				{
	// 					AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry.path().string() );
	// 					g_animStates["Bullfango"][entry.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
	// 				}
	// 			}
	// 		}
	// 		SkeletalMesh* newSkeletalMesh = SkeletalMesh::ImportFromXML( "Data/Models/FBXs/Bullfango/Xml/SkeletalMesh.xml" );
	// 		GameCharacter* chara4 = new BullfangoCharacter( "Bullfango", newSkeletalMesh, this );
	// 		chara4->m_animController->GetStateMachine()->SetInitialState( "Idle" );
	// 		chara4->SetActorLocalPosition( Vec3( -5.f, 0.f, 0.f ) );
	// 		m_actors.push_back( chara4 );
	// 	}

	m_screenCamera.SetViewport( AABB2::ZERO_TO_ONE );
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 1600.f / g_theWindow->GetAspect() ) );
	m_debugCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );

	//  	{
	//  		int count = 5;
	//  		for (int i = -50; i <= 50; i++)
	//  		{
	//  			float thickness = (count == 5) ? 0.1f : 0.05f;
	//  			Rgba8 color = (count == 5) ? Rgba8::GREEN : Rgba8::GRAY;
	//  			DebugAddWorldLine( Vec3( (float)i, -50.f, -thickness ), Vec3( (float)i, 50.f, thickness ), thickness, -1.f, color, color, DebugRenderMode::USE_DEPTH );
	//  			(count == 5) ? count = 1 : count++;
	//  		}
	//  		count = 5;
	//  		for (int i = -50; i <= 50; i++)
	//  		{
	//  			float thickness = (count == 5) ? 0.1f : 0.05f;
	//  			Rgba8 color = (count == 5) ? Rgba8::RED : Rgba8::GRAY;
	//  			DebugAddWorldLine( Vec3( -50.f, (float)i, -0.1f ), Vec3( 50.f, (float)i, thickness ), thickness, -1.f, color, color, DebugRenderMode::USE_DEPTH );
	//  			(count == 5) ? count = 1 : count++;
	//  		}
	//  
	//  		DebugAddWorldBasis( Mat44( Vec3( 1.f, 0.f, 0.f ), Vec3( 0.f, 1.f, 0.f ), Vec3( 0.f, 0.f, 1.f ), Vec3::ZERO ), -1.f, DebugRenderMode::USE_DEPTH );
	//  		Mat44 xAxisMat;
	//  		xAxisMat.AppendZRotation( -90.f );
	//  		xAxisMat.SetTranslation3D( Vec3( 0.2f, 0.f, 0.2f ) );
	//  		DebugAddWorldText( "x - Forward", xAxisMat, 0.2f, Vec2( 1.f, 1.f ), -1.f, Rgba8::RED );
	//  		Mat44 yAxisMat;
	//  		yAxisMat.AppendZRotation( 180.f );
	//  		yAxisMat.SetTranslation3D( Vec3( 0.f, 0.2f, 0.2f ) );
	//  		DebugAddWorldText( "y - left", yAxisMat, 0.2f, Vec2( 0.f, 1.f ), -1.f, Rgba8::GREEN );
	//  		Mat44 zAxisMat;
	//  		zAxisMat.AppendXRotation( 90.f );
	//  		zAxisMat.AppendYRotation( 180.f );
	//  		zAxisMat.SetTranslation3D( Vec3( 0.f, -0.2f, 1.f ) );
	//  		DebugAddWorldText( "z - up", zAxisMat, 0.2f, Vec2( 0.f, 0.f ), -1.f, Rgba8::BLUE );
	//  	}

		// for camera arm testing purpose
	// 	for (int i = 0; i < 4; i++)
	// 	{
	// 		Vec3 center = Vec3(
	// 			g_theRNG->RollRandomFloatInRange( -10.f, 10.f ),
	// 			g_theRNG->RollRandomFloatInRange( -10.f, 10.f ),
	// 			-2.f );
	// 		EulerAngles orientation = EulerAngles(
	// 			g_theRNG->RollRandomFloatInRange( 0.f, 360.f ),
	// 			10.f,
	// 			g_theRNG->RollRandomFloatInRange( 0.f, 360.f ) );
	// 		Vec3 delta = Vec3(
	// 			g_theRNG->RollRandomFloatInRange( 1.5f, 5.f ),
	// 			g_theRNG->RollRandomFloatInRange( 1.5f, 5.f ),
	// 			g_theRNG->RollRandomFloatInRange( 1.5f, 5.f ) );
	// 		CubeComponent* newCube = new CubeComponent( delta.x, delta.y, delta.z, true, WORLD_STATIC, center, Quat( orientation ) );
	// 		m_obbList.push_back( newCube );
	// 	}
	// 	CubeComponent* floor = new CubeComponent( 100.f, 100.f, 0.5f, true, WORLD_STATIC, Vec3( 0.f, 0.f, -0.25f ) );
	// 	m_obbList.push_back( floor );

}

void GameRun::Update( float deltaSeconds )
{
	m_deltaSeconds = deltaSeconds;
	if (m_timer->HasPeriodElapsed())
	{
		m_frameRate = int( 1.f / Clock::s_systemClock.GetDeltaSeconds() );
 		m_realFrameRate = int( 1.f / App::g_actualFrameTime );
		m_actualFrameTime = App::g_actualFrameTime;
	}
	m_timer->DecrementPeriodIfElapsed();

	DebugAddScreenText( Stringf( "Time: %.2f  FPS: %d(%d %.2fms)  Scale: %.2f", m_clock->GetTotalSeconds(), m_frameRate, m_realFrameRate, m_actualFrameTime * 1000.f, m_clock->GetTimeScale() ),
		m_debugCamera.GetDimensions(), 20.f, Vec2::ZERO, 0.f, Rgba8::WHITE, Rgba8::WHITE );
	DebugAddMessage( Stringf( "Player %.2f  %.2f  %.2f", m_player->GetActorWorldPosition().x, m_player->GetActorWorldPosition().y, m_player->GetActorWorldPosition().z ),
		0.f, Rgba8::WHITE, Rgba8::WHITE );
	DebugAddMessage( Stringf( "SunLight %.2f  %.2f  %.2f", m_sunOrientation.m_yawDegrees, m_sunOrientation.m_pitchDegrees, m_sunOrientation.m_rollDegrees ),
		0.f, Rgba8::WHITE, Rgba8::WHITE );
	DebugAddMessage( Stringf( "SunIntensity: %.2f  AmbientIntensity %.2f", m_sunIntensity, m_ambientIntensity ),
		0.f, Rgba8::WHITE, Rgba8::WHITE );
	//DebugAddWorldArrow( Vec3( 0.f, 0.f, 2.5f ), Vec3( 0.f, 0.f, 2.5f ) + m_sunOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), 0.2f, deltaSeconds );

	double k = GetCurrentTimeSeconds();
	InputResponse();
	
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (!m_actors[i])
			continue;
		if (m_actors[i] == m_player)
			continue;

		if (g_theApp->m_multiThreadingUpdate)
		{
			ActorUpdateJob* job = new ActorUpdateJob( m_actors[i], deltaSeconds );
			g_jobSystem->QueueNewJob( job );
		}
		else
		{
			m_actors[i]->Update( deltaSeconds );
		}
	}
	m_player->Update( deltaSeconds );

	Job* jobPointer = nullptr;
	while (((jobPointer = g_jobSystem->RetrieveJob()) != nullptr) || g_jobSystem->IsEmpty())
	{
		delete jobPointer;
	}

	UpdateCollision();

	DebugAddMessage( Stringf( "%f", GetCurrentTimeSeconds() - k ), 0.f, Rgba8::YELLOW, Rgba8::YELLOW );

	//UpdateImGui();
}

void GameRun::Shutdown()
{
	for (Actor* actor : m_actors)
	{
		delete actor;
	}
	m_actors.clear();

	for (StaticMeshComponent* asset : m_assets)
	{
		delete asset;
	}
	m_assets.clear();

	for (ShapeComponent* shape : m_worldCollisionList)
	{
		delete shape;
	}
	m_worldCollisionList.clear();

	for (auto& animMap : g_animStates)
	{
		for (auto& animPair : animMap.second)
		{
			delete animPair.second;
		}
	}
	g_animStates.clear();

	for (auto& meshPair : g_skeletonMeshes)
	{
		delete meshPair.second;
	}
	g_skeletonMeshes.clear();

	g_collisionComponents.clear();

	g_characterDefs.clear();
}

void GameRun::InitializeCharacters()
{
	XmlDocument doc;
	const char* xmlPath = "Data/Definitions/QuestInfo.xml";
	if (doc.LoadFile( xmlPath ) != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE( Stringf( "Fail to load xml %s", xmlPath ).c_str() );
		return;
	}

	XmlElement* root = doc.FirstChildElement( "QuestInfo" );
	if (!root)
	{
		ERROR_AND_DIE( Stringf( "Invalid XML format: Missing <QuestInfo> root element." ).c_str() );
		return;
	}

	for (XmlElement* spawnElem = root->FirstChildElement( "SpawnInfo" ); spawnElem; spawnElem = spawnElem->NextSiblingElement( "SpawnInfo" ))
	{
		QuestInfo::SpawnInfo spawnInfo;
		std::string name = ParseXmlAttribute( *spawnElem, "name", "" );
		int amount = 0;
		spawnElem->QueryIntAttribute( "amount", &amount );

		if (name != "")
		{
			spawnInfo.m_name = name;
			spawnInfo.m_amount = amount;
			m_questInfo.m_spawnInfo.push_back( spawnInfo );

			std::string definitionPath = "Data/Definitions/CharacterInfo/" + (std::string)name + ".xml";
			g_characterDefs[name] = CharacterDefinition( definitionPath );

			namespace fs = std::filesystem;
			const fs::path xmlDir = g_characterDefs[name].m_animationPath;
			for (const auto& entry : fs::directory_iterator( xmlDir ))
			{
				if (!entry.is_directory())
					continue;

				for (const auto& entry2 : fs::directory_iterator( entry.path() ))
				{
					if (entry2.is_regular_file() && entry2.path().extension() == ".xml")
					{
						if (g_theApp->m_multiThreadingLoad)
						{
							LoadAnimationJob* job = new LoadAnimationJob( name, entry2 );
							g_jobSystem->QueueNewJob( job );
						}
						else
						{
							AnimationSequence* newAnim = AnimationSequence::ImportFromXML( entry2.path().string() );
							g_animStates[name][entry2.path().stem().string()] = new AnimationState( newAnim, newAnim->m_name );
						}
					}
				}
			}
			if (g_theApp->m_multiThreadingLoad)
			{
				LoadSkeletonJob* job = new LoadSkeletonJob( name );
				g_jobSystem->QueueNewJob( job );
			}
			else
			{
				g_skeletonMeshes[name] = SkeletalMesh::ImportFromXML( g_characterDefs[name].m_skeletalMeshPath );
			}
		}
	}
}

void GameRun::InitializeEnvironment()
{
	std::string filename = "Data/Definitions/ZoneInfo.xml";

	struct ObjectData 
	{
		std::string filePath;
		Vec3 position;
		Vec3 orientation;
		Vec3 scale;
	};
	std::vector<ObjectData>objects;

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile( filename.c_str() ) != tinyxml2::XML_SUCCESS) 
	{
		ERROR_AND_DIE( (Stringf( "Failed to load XML file: %s" ), filename).c_str() );
	}

	tinyxml2::XMLElement* root = doc.FirstChildElement( "Objects" );
	if (!root) 
	{
		ERROR_AND_DIE( Stringf( "Missing <Objects> root element." ).c_str() );
	}

	for (tinyxml2::XMLElement* elem = root->FirstChildElement( "Object" ); elem; elem = elem->NextSiblingElement( "Object" )) 
	{
		ObjectData obj;
		const char* path = elem->Attribute( "filePath" );
		if (path) 
		{
			obj.filePath = path;
		}

		tinyxml2::XMLElement* posElem = elem->FirstChildElement( "Position" );
		if (posElem) 
		{
			posElem->QueryFloatAttribute( "x", &obj.position.x );
			posElem->QueryFloatAttribute( "y", &obj.position.y );
			posElem->QueryFloatAttribute( "z", &obj.position.z );
		}

		tinyxml2::XMLElement* orientElem = elem->FirstChildElement( "Orientation" );
		if (orientElem) 
		{
			orientElem->QueryFloatAttribute( "x", &obj.orientation.x );
			orientElem->QueryFloatAttribute( "y", &obj.orientation.y );
			orientElem->QueryFloatAttribute( "z", &obj.orientation.z );
		}

		tinyxml2::XMLElement* scaleElem = elem->FirstChildElement( "Scale" );
		if (scaleElem)
		{
			scaleElem->QueryFloatAttribute( "x", &obj.scale.x );
			scaleElem->QueryFloatAttribute( "y", &obj.scale.y );
			scaleElem->QueryFloatAttribute( "z", &obj.scale.z );
		}

		objects.push_back( obj );
	}

	for (int i = 0; i < objects.size(); i++)
	{
		std::string assetName = std::filesystem::path( objects[i].filePath ).stem().string();

		if (g_theApp->m_multiThreadingLoad)
		{
			LoadAssetJob* job = new LoadAssetJob( assetName, objects[i].filePath );
			g_jobSystem->QueueNewJob( job );
		}
		else
		{
			std::string str = objects[i].filePath.substr( 0, objects[i].filePath.size() - 4 ) + ".xml";
			GameRun::g_staticMeshes[assetName] = StaticMesh::ImportFromXML( str );
		}

		ZoneInfo::SpawnInfo spawnInfo;
		spawnInfo.m_name = assetName;
		spawnInfo.m_position = objects[i].position;
		spawnInfo.m_orientation = Quat( objects[i].orientation );
		spawnInfo.m_scale = objects[i].scale;
		m_zoneInfo.m_spawnInfo.push_back( spawnInfo );
	}
}

void GameRun::InitializeWorldCollisions()
{
	std::string filename = "Data/Definitions/ZoneInfo.xml";

	tinyxml2::XMLDocument doc;
	if (doc.LoadFile( filename.c_str() ) != tinyxml2::XML_SUCCESS)
	{
		ERROR_AND_DIE( (Stringf( "Failed to load XML file: %s" ), filename).c_str() );
	}

	tinyxml2::XMLElement* root = doc.FirstChildElement( "Objects" );
	if (!root)
	{
		ERROR_AND_DIE( Stringf( "Missing <Objects> root element." ).c_str() );
	}

	for (tinyxml2::XMLElement* elem = root->FirstChildElement( "Object" ); elem; elem = elem->NextSiblingElement( "Object" ))
	{
		const char* path = elem->Attribute( "filePath" );
		tinyxml2::XMLElement* collisionGroupElem = elem->FirstChildElement( "Collisions" );
		if (collisionGroupElem)
		{
			for (XmlElement* collisionElem = collisionGroupElem->FirstChildElement( "Collision" );
				collisionElem; collisionElem = collisionElem->NextSiblingElement( "Collision" ))
			{
				CollisionShape shape = (CollisionShape)ParseXmlAttribute( *collisionElem, "shape", 0 );
				switch (shape)
				{
				case CollisionShape::CAPSULE:
					break;
				case CollisionShape::SPHERE:
					break;
				case CollisionShape::CUBE:
				{
					Vec3 position = ParseXmlAttribute( *collisionElem, "position", Vec3::ZERO );
					Quat orientation = Quat( ParseXmlAttribute( *collisionElem, "orientation", EulerAngles::ZERO ) );
					Vec3 scale = ParseXmlAttribute( *collisionElem, "scale", Vec3::ONE );
					float length = ParseXmlAttribute( *collisionElem, "length", 0.f );
					float width = ParseXmlAttribute( *collisionElem, "width", 0.f );
					float height = ParseXmlAttribute( *collisionElem, "height", 0.f );
					CubeComponent* newCube = new CubeComponent( length, width, height, true, WORLD_STATIC, position, orientation, scale );
					std::filesystem::path filePath( path );
					newCube->m_name = filePath.stem().string();
					m_worldCollisionList.push_back( newCube );
					break;
				}
				}
			}
		}
	}
}

void GameRun::InitializeBehaviorTrees()
{
	TreeNodeFactory::RegisterNodeTypes();
	GameTreeNodeFactory::RegisterNodeTypes();
	BullfangoTreeNodeFactory::RegisterNodeTypes();

	namespace fs = std::filesystem;
	const fs::path xmlDir = "Data/Definitions/BehaviorTrees";
	for (const auto& entry : fs::directory_iterator( xmlDir ))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".xml")
		{
			BehaviorTree* newTree = new BehaviorTree( m_clock );
			newTree->ImportFromXML( entry.path().string() );
			newTree->m_name = entry.path().stem().string();
			g_behaviorTrees[entry.path().stem().string()] = newTree;
		}
	}
}

void GameRun::SpawnCharacters()
{
	for (auto spawnInfo : m_questInfo.m_spawnInfo)
	{
		for (int i = 0; i < spawnInfo.m_amount; i++)
		{
			std::string name = spawnInfo.m_name;

			GameCharacter* chara = CharacterFactory::CreateCharacter( name, g_skeletonMeshes[name], this );

			if (g_behaviorTrees.find(name) != g_behaviorTrees.end())
				chara->m_controller->GetBehaviorTree() = g_behaviorTrees[name];

			chara->m_animController->GetStateMachine()->SetInitialState( "Idle" );

			int index;
			for (index = 0; index < m_actors.size(); index++)
			{
				if (!m_actors[index])
				{
					break;
				}
			}
			if (index == m_actors.size())
			{
				m_actors.push_back( chara );
				chara->SetUID( ActorUID( m_actorSalt, index ) );
				m_actorSalt++;
			}

			if (name == "Paladin")
			{
				m_player = (PlayerCharacter*)chara;
			}
			else if (name == "Rathian")
			{
				chara->m_animController->GetStateMachine()->SetInitialState( "Sleeping" );
				chara->SetActorWorldPosition( Vec3(
					69.5f,
					-2.f,
					0.0f
				) );
				chara->SetActorWorldOrientation( Quat( EulerAngles( 180.f, 0.f, 0.f ) ) );
				chara->SetActorWorldScale( chara->GetActorWorldScale() * 0.5f );
			}
			else if (name == "Bullfango")
			{
				chara->m_animController->GetStateMachine()->SetInitialState( "Idle" );
				chara->SetActorWorldPosition( Vec3(
					30.f,
					9.25f,
					0.1f
				) );
				chara->SetActorWorldOrientation( Quat( EulerAngles( 180.f, 0.f, 0.f ) ) );
			}
			else
			{
				chara->SetActorWorldPosition( Vec3(
					g_theRNG->RollRandomFloatInRange( 18.f, 52.f ),
					g_theRNG->RollRandomFloatInRange( -11.f, 11.f ),
					0.5f
				) );
				chara->SetActorWorldOrientation( Quat( EulerAngles( g_theRNG->RollRandomFloatInRange( 0.f, 360.f ), 0.f, 0.f ) ) );
			}
		}
	}
}

void GameRun::SpawnEnvironment()
{
	for (auto spawnInfo : m_zoneInfo.m_spawnInfo)
	{
		std::string name = spawnInfo.m_name;
		StaticMeshComponent* smc = new StaticMeshComponent( g_staticMeshes[name] );
		for (MeshT& a : smc->GetStaticMesh()->GetMeshes())
		{
			a.material->m_shader = g_theRenderer->CreateShader( "Data/Shaders/Phong.hlsl", VertexType::VERTEX_PCUTBN );
			a.material->m_vertexType = VertexType::VERTEX_PCUTBN;
			a.material->m_color.a = 255;

			if (a.name.find( "VertexColorBlend" ) != std::string::npos ||
				a.name == "LOD_1_Group_0_Sub_2__B_yane01_01")
			{
				for (auto& vert : a.vertexes)
				{
					vert.m_color = Rgba8::WHITE;
				}
			}
		}
		smc->SetLocalPosition( spawnInfo.m_position );
		smc->SetLocalOrientation( spawnInfo.m_orientation );
		smc->SetLocalScale( spawnInfo.m_scale );
		m_assets.push_back( smc );
	}
}

void GameRun::UpdateCollision()
{
	// Handle Attack between characters
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (!m_actors[i])
			continue;
		GameCharacter* characterI = dynamic_cast<GameCharacter*>(m_actors[i]);
		if (characterI->IsDead())
			continue;

		for (int j = 0; j < m_actors.size(); j++)
		{
			if (!m_actors[j])
				continue;
			GameCharacter* characterJ = dynamic_cast<GameCharacter*>(m_actors[j]);
			if (characterI == characterJ)
				continue;
			if (characterJ->IsDead())
				continue;

			if (!characterI->m_attackStates.empty() && !characterJ->m_isInvincible)
				GameCharacter::DoesAttackHits( characterI, characterJ );
			if (!characterJ->m_attackStates.empty() && !characterI->m_isInvincible)
				GameCharacter::DoesAttackHits( characterJ, characterI );
		}
	}

	// Handle Collision push back between characters
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (!m_actors[i])
			continue;
		GameCharacter* characterI = dynamic_cast<GameCharacter*>(m_actors[i]);
		if (!characterI)
			continue;
// 		if (characterI->IsDead())
// 			continue;

		for (int j = i + 1; j < m_actors.size(); j++)
		{
			if (!m_actors[j])
				continue;
			GameCharacter* characterJ = dynamic_cast<GameCharacter*>(m_actors[j]);
			if (!characterJ)
				continue;
// 			if (characterJ->IsDead())
// 				continue;

			Vec3 mtvA, mtvB;
			bool flag = GameCharacter::DoCharactersOverlap( characterI, characterJ, mtvA, mtvB );
			if (!flag)
				continue;

			m_actors[i]->SetActorWorldPosition( m_actors[i]->GetActorWorldPosition() + mtvA );
			m_actors[j]->SetActorWorldPosition( m_actors[j]->GetActorWorldPosition() + mtvB );
		}
	}
}

void GameRun::UpdateImGui()
{

	ImGui::NewFrame();

	//ImGui::NewFrame();
	ImGui::Begin( "Camera Settings" );

	static std::string length = std::to_string( m_player->m_cameraArmLength );
	static std::string offsetX = std::to_string( m_player->m_cameraArmPivotPos.x );
	static std::string offsetY = std::to_string( m_player->m_cameraArmPivotPos.y );
	static std::string offsetZ = std::to_string( m_player->m_cameraArmPivotPos.z );

	ImGui::SetNextItemWidth( 100.f );
	if (ImGui::InputText( "CameraArmLength", &length[0], length.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue ))
	{
		try
		{
			m_player->m_cameraArmLength = std::stof( length );
		}
		catch (...)
		{

		}
	}
	ImGui::SetNextItemWidth( 100.f );
	if (ImGui::InputText( "CameraPivotOffsetX", &offsetX[0], offsetX.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue ))
	{
		try
		{
			m_player->m_cameraArmPivotPos.x = std::stof( offsetX );
		}
		catch (...)
		{

		}
	}
	ImGui::SetNextItemWidth( 100.f );
	if (ImGui::InputText( "CameraPivotOffsetY", &offsetY[0], offsetY.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue ))
	{
		try
		{
			m_player->m_cameraArmPivotPos.y = std::stof( offsetY );
		}
		catch (...)
		{

		}
	}
	ImGui::SetNextItemWidth( 100.f );
	if (ImGui::InputText( "CameraPivotOffsetZ", &offsetZ[0], offsetZ.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue ))
	{
		try
		{
			m_player->m_cameraArmPivotPos.z = std::stof( offsetZ );
		}
		catch (...)
		{

		}
	}

	ImGui::End();
}

void GameRun::Render() const
{
	g_theRenderer->BeginCamera( m_player->m_camera );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	LightingConstants lightCons;
	Vec3 k = m_sunOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D();
	lightCons.sunDirection[0] = k.x;
	lightCons.sunDirection[1] = k.y;
	lightCons.sunDirection[2] = k.z;
	lightCons.sunIntensity = m_sunIntensity;
	lightCons.ambientIntensity = m_ambientIntensity;
	lightCons.worldEyePosition[0] = m_player->m_camera.m_position.x;
	lightCons.worldEyePosition[1] = m_player->m_camera.m_position.y;
	lightCons.worldEyePosition[2] = m_player->m_camera.m_position.z;
	lightCons.UseNormalMap = false;
	lightCons.UseSpecularMap = false;
	g_theRenderer->SetLightingConstants( lightCons );
	for (int i = 0; i < m_actors.size(); i++)
	{
		if (!m_actors[i])
			continue;

		m_actors[i]->Render();
	}

 	for (int i = 0; i < m_assets.size(); i++)
 	{
 		if (!m_assets[i])
 			continue;
 
 		g_theRenderer->SetBlendMode( BlendMode::OPAQUE );
 		g_theRenderer->SetSamplerMode( SamplerMode::BILINEAR_WARP );
 		g_theRenderer->SetModelConstants( m_assets[i]->GetWorldTransform() * Mat44::CreateZRotationDegrees( 90.f ) * Mat44::CreateXRotationDegrees( 90.f ) );
 		m_assets[i]->Render();
 	}
 	g_theRenderer->SetSamplerMode( SamplerMode::POINT_CLAMP );

	if (m_debugdraw)
	{
		for (int i = 0; i < m_worldCollisionList.size(); i++)
		{
			if (!m_worldCollisionList[i])
				continue;

			std::vector<Vertex_PCU> verts;

			AddVertsForOBB3D( verts, m_worldCollisionList[i]->CalculateBoundsOBB3D(), Rgba8::GRAY );
			g_theRenderer->BindShader( nullptr );
			g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/Test_StbiFlippedAndOpenGL.png" ) );
			g_theRenderer->SetModelConstants();
			g_theRenderer->SetRasterizerState( RasterizerMode::WIREFRAME_CULL_BACK );
			g_theRenderer->DrawVertexArray( verts );
		}
		g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	}

	g_theRenderer->EndCamera( m_player->m_camera );

	RenderUI();

	DebugRenderWorld( m_player->m_camera, DebugRenderMode::ALWAYS );
	DebugRenderWorld( m_player->m_camera, DebugRenderMode::USE_DEPTH );
	DebugRenderWorld( m_player->m_camera, DebugRenderMode::NUL );
	DebugRenderWorld( m_player->m_camera, DebugRenderMode::X_RAY );
	DebugRenderScreen( m_debugCamera );

	//g_ImGui->Render();
}

void GameRun::RenderUI() const
{
	g_theRenderer->BeginCamera( m_screenCamera );
	std::vector<Vertex_PCU> verts;
	//AddVertsForStatusBar( verts, Vec2( 100.f, 760.f ), m_player->m_maxHealth * 0.01f * 600.f, 5.f, true, Rgba8( 0, 0, 0, 150 ) );
	AddVertsForStatusBar( verts, Vec2( 160.f, 710.f / g_theWindow->GetAspect() * 2.f ), Clamp(m_player->m_health, 0.f, 999.f) * 0.01f * 600.f, 10.f, true, Rgba8::GREEN );
	AddVertsForStatusBar( verts, Vec2( 160.f + Clamp(m_player->m_health, 0.f, 999.f) * 0.01f * 600.f, 710.f / g_theWindow->GetAspect() * 2.f ), Clamp(m_player->m_autoRecoverableHP, 0.f, 999.f) * 0.01f * 600.f, 10.f, true, Rgba8::RED );
	AddVertsForStatusBarBorder( verts, Vec2( 160.f, 710.f / g_theWindow->GetAspect() * 2.f ), m_player->m_maxHealth * 0.01f * 600.f, 10.f, 4.f, true, Rgba8::GRAY );
	//AddVertsForStatusBar( verts, Vec2( 100.f, 760.f ), m_player->m_maxStamina * 0.01f * 600.f, 5.f, true, Rgba8( 0, 0, 0, 150 ) );
	Rgba8 staminaColor = Interpolate( Rgba8::RED, Rgba8::ORANGE, Clamp( m_player->m_stamina / 25.f, 0.f, 1.f ) );
	AddVertsForStatusBar( verts, Vec2( 160.f, 690.f / g_theWindow->GetAspect() * 2.f ), m_player->m_stamina * 0.01f * 600.f, 10.f, false, staminaColor );
	AddVertsForStatusBarBorder( verts, Vec2( 160.f, 690.f / g_theWindow->GetAspect() * 2.f ), m_player->m_maxStamina * 0.01f * 600.f, 10.f, 4.f, false, Rgba8::GRAY );
// 	Vec2 center = Vec2( 64.4f, 745.f );
// 	float radius = 40.f;
// 	AddVertsForRect2D( verts, center - Vec2( 0.f, radius ), center + Vec2( radius, 0.f ), center + Vec2( 0.f, radius ), center - Vec2( radius, 0.f ), Rgba8::GRAY );
// 	radius = 36.f;
// 	AddVertsForRect2D( verts, center - Vec2( 0.f, radius ), center + Vec2( radius, 0.f ), center + Vec2( 0.f, radius ), center - Vec2( radius, 0.f ), Rgba8( 139, 0, 0 ) );
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode( BlendMode::ALPHA );
	g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_NONE );
	g_theRenderer->CreateShader( "Default" );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( verts );

	Vec2 center = Vec2( 104.4f, 745.f / g_theWindow->GetAspect() * 2.f ) + Vec2( -20.f, -40.f / g_theWindow->GetAspect() * 2.f );
	Vec2 halfSize = Vec2( 54.f, 60.f ) * 1.5f;
	AABB2 quad( center - halfSize, center + halfSize );
	std::vector<Vertex_PCU> vertClock;
	AddVertsForAABB2D( vertClock, quad, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
	

	center = Vec2( 112.f, 743.f / g_theWindow->GetAspect() * 2.f ) + Vec2( -20.f, -40.f / g_theWindow->GetAspect() * 2.f );
	halfSize = Vec2( 33.f, 33.f ) * 1.5f;
	quad = AABB2( center - halfSize, center + halfSize );

	std::vector<Vertex_PCU> vertClockBase;
	AddVertsForRect2D( vertClockBase, center + Vec2( 0.f, -31.f ), center + Vec2( 31.f, 0.f ), center + Vec2( 0.f, 31.f ), center + Vec2( -31.f, 0.f ), Rgba8( 153, 49, 48 ) );
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->DrawVertexArray( vertClockBase );

	std::vector<Vertex_PCU> vertClockInner;
	AddVertsForAABB2D( vertClockInner, quad, Rgba8( 133, 29, 36 ), Vec2::ZERO, Vec2::ONE );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/ClockInner.tga" ) );
	g_theRenderer->DrawVertexArray( vertClockInner );

	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/Clock.png" ) );
	g_theRenderer->DrawVertexArray( vertClock );

	std::vector<Vertex_PCU> vertPointerSta;
	OBB2 pointerStaOBB( center + Vec2( 0.f, 11.f ) * 1.5f, Vec2::RIGHT, Vec2( 15.f, 4.f ) * 1.5f );
	pointerStaOBB.RotateAboutPivot( center, 60.f );
	AddVertsForOBB2D( vertPointerSta, pointerStaOBB, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/PointerSta.png" ) );
	g_theRenderer->DrawVertexArray( vertPointerSta );

	std::vector<Vertex_PCU> vertPointerMov;
	OBB2 pointerMovOBB( center + Vec2( 0.f, 13.5f ) * 1.5f, Vec2::RIGHT, Vec2( 17.5f, 4.f ) * 1.5f );
	pointerMovOBB.RotateAboutPivot( center, m_clock->GetTotalSeconds() * -6.f );
	AddVertsForOBB2D( vertPointerMov, pointerMovOBB, Rgba8::WHITE, Vec2::ZERO, Vec2::ONE );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/PointerMov.png" ) );
	g_theRenderer->DrawVertexArray( vertPointerMov );

	center = Vec2( 1400.f, 100.f / g_theWindow->GetAspect() * 2.f );
	halfSize = Vec2( 61.f, 61.f );
	std::vector<Vertex_PCU> vertItemFrame;
	quad = AABB2( center - halfSize, center + halfSize );
	AddVertsForAABB2D( vertItemFrame, quad, Rgba8::GRAY, Vec2::ZERO, Vec2::ONE );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/ItemFrame.tga" ) );
	g_theRenderer->DrawVertexArray( vertItemFrame );

	halfSize = Vec2( 48.f, 48.f ) * 0.75f;
	std::vector<Vertex_PCU> vertItem;
	quad = AABB2( center - halfSize, center + halfSize );
	AddVertsForAABB2D( vertItem, quad, Rgba8( 71, 207, 145 ), Vec2::ZERO, Vec2::ONE );
	g_theRenderer->BindTexture( g_theRenderer->CreateOrGetTextureFromFile( "Data/Images/UI/Icons/row-1-column-6.png" ) );
	g_theRenderer->DrawVertexArray( vertItem );

	g_theRenderer->EndCamera( m_screenCamera );
}   

void GameRun::InputResponse()
{
	if (g_theInput->IsNewKeyPressed( KEYCODE_ESC ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::BACK ))
	{
		g_theApp->SetQuitting( true );
	}

	if (g_theInput->IsKeyDown( "sunYawLeft" ))
	{
		m_sunOrientation.m_yawDegrees += 45.f * m_deltaSeconds;
		while (m_sunOrientation.m_yawDegrees > 360.f) m_sunOrientation.m_yawDegrees -= 360.f;
	}
	if (g_theInput->IsKeyDown( "sunYawRight" ))
	{
		m_sunOrientation.m_yawDegrees -= 45.f * m_deltaSeconds;
		while (m_sunOrientation.m_yawDegrees < 0.f) m_sunOrientation.m_yawDegrees += 360.f;
	}
	if (g_theInput->IsKeyDown( "sunPitchUp" ))
	{
		m_sunOrientation.m_pitchDegrees += 45.f * m_deltaSeconds;
		while (m_sunOrientation.m_pitchDegrees > 360.f) m_sunOrientation.m_pitchDegrees -= 360.f;
	}
	if (g_theInput->IsKeyDown( "sunPitchDown" ))
	{
		m_sunOrientation.m_pitchDegrees -= 45.f * m_deltaSeconds;
		while (m_sunOrientation.m_pitchDegrees < 0.f) m_sunOrientation.m_pitchDegrees += 360.f;
	}
	if (g_theInput->IsNewKeyPressed( "sunIntensityUp" ))
	{
		m_sunIntensity = Clamp( m_sunIntensity + 0.1f, 0.f, 1.f );
		m_ambientIntensity = 1.f - m_sunIntensity;
	}
	if (g_theInput->IsNewKeyPressed( "sunIntensityDown" ))
	{
		m_sunIntensity = Clamp( m_sunIntensity - 0.1f, 0.f, 1.f );
		m_ambientIntensity = 1.f - m_sunIntensity;
	}
	if (g_theInput->IsNewKeyPressed( 'K' ))
	{
		m_debugdraw = !m_debugdraw;
	}
}

int GameRun::GetGameMode() const
{
	return 1;
}

Actor* GameRun::GetActorByUID( ActorUID const& uid ) const
{
	if (!uid.isValid())
		return nullptr;
	unsigned int index = uid.GetIndex();
	if (index < 0 || index >( unsigned int )(m_actors.size()))
		return nullptr;
	if (m_actors[index] == nullptr)
		return nullptr;

	return (m_actors[index]->GetUID() == uid) ? m_actors[index] : nullptr;
}

ActorUID GameRun::GetPlayerUID() const
{
	return m_player->GetUID();
}

//----------------------------------------------------------------------------------------------

GameAttract::GameAttract()
{
}

GameAttract::~GameAttract()
{
}

void GameAttract::Startup()
{
	//g_theRenderer->SetRasterizerState( RasterizerMode::SOLID_CULL_BACK );
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	m_clock = new Clock;
	m_blinkTimer = new Timer( 1.6f, m_clock );

	m_sunDirection = Vec3( 2.f, 1.f, -1.f );
	m_sunIntensity = 0.f;
	m_ambientIntensity = 1.f;

	m_shapes[0] = PullPaintBaseOnState( NextState::ToGold );
	m_shapes[1] = PullPaintBaseOnState( NextState::ToSetting );
	m_shapes[2] = PullPaintBaseOnState( NextState::ToQuit );
}

void GameAttract::Update( float deltaSeconds )
{
	//m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	if (m_blinkTimer->HasPeriodElapsed())
	{
		m_age += 0.f;
	}
	m_blinkTimer->DecrementPeriodIfElapsed();
	m_age += deltaSeconds;

	if (m_rollTimer > 0)
	{
		RollingAnimation( m_rollingDirection, deltaSeconds );
	}
	else
	{
		if (static_cast<int>(m_rollingOffset) != 0)
		{
			if (m_rollingDirection > 0)
				m_selectedState = (NextState)(((int)m_selectedState + 1) % (int)NextState::StateCount);
			else if (m_rollingDirection < 0)
				m_selectedState = (NextState)(((int)m_selectedState + (int)NextState::StateCount - 1) % (int)NextState::StateCount);
			m_rollingOffset = 0;
			m_blinkTimer->Start();
		}
	}
	InputResponse();
}

void GameAttract::Render() const
{
	g_theRenderer->BeginCamera( m_screenCamera );
	Vec3 sunDirFixed = m_sunDirection;
	sunDirFixed.z = -1.f;
	g_theRenderer->SetLightingConstants( sunDirFixed, m_sunIntensity, m_ambientIntensity );
	RenderUI();

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader( g_theRenderer->CreateShader( "Default" ) );
	g_theRenderer->BindTexture( &font->GetTexture() );

	if (m_rollingOffset == 0.f)
	{
		std::vector<Vertex_PCU> displayText;
		displayText.reserve( 50 );
		switch ((NextState)m_selectedState)
		{
		case GameAttract::NextState::ToGold:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "PLAY", Rgba8::GREEN );
			break;
		case GameAttract::NextState::ToSetting:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "SETTINGS", Rgba8::GRAY );
			break;
		case GameAttract::NextState::ToQuit:
			font->AddVertsForTextInBox2D( displayText, AABB2( 500.f, 0.f, 1600.f, 800.f ), 150.f, "QUIT", Rgba8::RED );
			break;
		}
		g_theRenderer->DrawVertexArray( displayText );
	}


	g_theRenderer->EndCamera( m_screenCamera );
}

void GameAttract::Shutdown()
{
	delete m_blinkTimer;
	m_blinkTimer = nullptr;

	delete m_clock;
	m_clock = nullptr;
}

int GameAttract::GetGameMode() const
{
	return (int)AppState::IN_ATTRACT;
}

void GameAttract::InputResponse()
{
	MenuFunctionButton();
}

void GameAttract::RenderUI() const
{

	float blinkFrac = abs( m_blinkTimer->GetElapsedFraction() - 0.5f ) + 0.5f;
	g_theRenderer->BindTexture( nullptr );

	unsigned char tempAlpha = static_cast<unsigned char>(255 * blinkFrac);
	Vec2 rollingShakeOffset = 8.f * Vec2( g_theRNG->RollRandomFloatInRange( -1.5f, 1.5f ) * m_rollTimer / ROLL_TIME, g_theRNG->RollRandomFloatInRange( -1.5f, 1.5f ) * m_rollTimer / ROLL_TIME );
	for (int i = -2; i <= 2; i++)
	{
		NextState stateIcon = (NextState)((i + (int)NextState::StateCount + (int)m_selectedState) % (int)NextState::StateCount);
		Rgba8 color;
		switch (stateIcon)
		{
		case GameAttract::NextState::ToGold:
			color = Rgba8::GREEN;
			break;
		case GameAttract::NextState::ToSetting:
			color = Rgba8::GRAY;
			break;
		case GameAttract::NextState::ToQuit:
			color = Rgba8::RED;
			break;
		case GameAttract::NextState::StateCount:
			break;
		default:
			break;
		}
		if (i == 0 && m_rollingOffset == 0.f)
		{
			color.a = tempAlpha;
		}
		else
		{
			color.a = 50;
		}
		Vec2 selectionOffset = Vec2( 0.f, static_cast<float>(i * -1.f * 800.f / 3.f) );
		g_theRenderer->SetModelConstants( Mat44::CreateTranslation2D( Vec2( 400.f, 400.f ) + rollingShakeOffset + selectionOffset + Vec2( 0.f, m_rollingOffset ) ), color );
		g_theRenderer->DrawVertexArray( m_shapes[(i + (int)NextState::StateCount + (int)m_selectedState) % (int)NextState::StateCount] );
	}
}

void GameAttract::MenuFunctionButton()
{
	if (g_theInput->IsNewKeyPressed( "confirm" ))
	{
		if (m_rollingOffset == 0.f)
		{
			switch (m_selectedState)
			{
			case GameAttract::NextState::ToGold:
				g_theApp->SetState( AppState::IN_GAME );
				break;
			case GameAttract::NextState::ToSetting:
				g_theApp->SetState( AppState::IN_SETTING );
				break;
			case GameAttract::NextState::ToQuit:
				g_theApp->SetQuitting( true );
				break;
			case GameAttract::NextState::StateCount:
				break;
			default:
				break;
			}
		}
	}
	if (g_theInput->IsNewKeyPressed( "cancel" ))
	{
		g_theApp->SetQuitting( true );
	}
	for (int i = 0; i < g_theInput->GetControllerCount(); i++)
	{
		if (g_theInput->GetController( i ).isConnected())
		{
			if (g_theInput->GetController( i ).IsNewButtonDown( XboxButtonID::START ))
			{
				g_theApp->SetState( AppState::IN_GAME );
				break;
			}
			if (g_theInput->GetController( i ).IsNewButtonDown( XboxButtonID::BACK ))
			{
				g_theApp->SetQuitting( true );
			}
		}
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::UP ))
	{
		if (m_rollTimer > 0)
			return;

		ResetRollAnim();
		m_rollingDirection = -1.0f;
	}
	else if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::DOWN ))
	{
		if (m_rollTimer > 0)
			return;

		ResetRollAnim();
		m_rollingDirection = 1.0f;
	}
}

void GameAttract::RollingAnimation( float directionModifier, float deltaSeconds )
{
	float totalDistance = static_cast<float>(1600.f / 6.f);
	float slowDistance = totalDistance * 0.2f;
	float fastDistance = totalDistance * 0.8f;
	float slowSpeed = slowDistance * 2.0f;
	float fastSpeed = fastDistance * 5.0f;
	float timeElapsed = ROLL_TIME - m_rollTimer;
	if (timeElapsed <= 0.5f)
	{
		m_rollingOffset += directionModifier * slowSpeed * deltaSeconds;
	}
	else if (timeElapsed <= ROLL_TIME)
	{
		m_rollingOffset += directionModifier * fastSpeed * deltaSeconds;
	}

	m_rollTimer -= deltaSeconds;
}

void GameAttract::AbortAnimation()
{

}

void GameAttract::ResetRollAnim()
{
	m_rollTimer = ROLL_TIME;
}

std::vector<Vertex_PCU> GameAttract::PullPaintBaseOnState( NextState state ) const
{
	std::vector<Vertex_PCU>emptyVector;
	switch (state)
	{
	case NextState::ToGold:
		return g_theMap->RUN_VERT_MAP;
	case NextState::ToSetting:
		return g_theMap->GEAR_VERT_MAP;
	case NextState::ToQuit:
		return g_theMap->QUIT_VERT_MAP;
	default:
		return emptyVector;
	}
}


GameSetting::GameSetting()
{
}

GameSetting::~GameSetting()
{
}

void GameSetting::Startup()
{
	m_screenCamera.SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );
	font->AddVertsForText2D( m_categorys, Vec2( 100.f, 500.f ), 40.f, " Audio  ", Rgba8::WHITE, 0.7f );
	font->AddVertsForText2D( m_categorys, Vec2( 100.f, 300.f ), 40.f, " Keybind", Rgba8::WHITE, 0.7f );

	font->AddVertsForTextInBox2D( m_naviMain, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nSpace key to select", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviAudio, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nArrow Left/Right key to change volume", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviKeybind, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Arrow Up/Down key to navigate up and down\nSpace key to change keybinding", Rgba8::WHITE, 0.7f, Vec2::ZERO );
	font->AddVertsForTextInBox2D( m_naviKeybind1, AABB2( 0.f, 0.f, 1600.f, 800.f ), 20.f, "Press any key to bind\nEsc key to cancel", Rgba8::WHITE, 0.7f, Vec2::ZERO );

	m_selectingCategory = 1;

	m_clock = new Clock;
	m_blinkTimer = new Timer( 0.3f, m_clock );
}

void GameSetting::Update( float deltaSeconds )
{
	UNUSED( deltaSeconds );
	if (m_blinkTimer->HasPeriodElapsed())
	{
		m_blinkTimer->Start();
		m_blinkShow = !m_blinkShow;
	}
	InputResponse();
}

void GameSetting::Render() const
{
	std::vector<Vertex_PCU> m_elementsAudio;
	std::vector<Vertex_PCU> m_elementsKeybind;

	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/SquirrelFixedFont.png" );

	std::vector<Vertex_PCU> categorySelection;
	categorySelection.reserve( 100 );
	font->AddVertsForText2D( categorySelection, Vec2( 100.f, 700 - m_selectingCategory * 200.f ), 40.f, ">       <", Rgba8::WHITE, 0.7f );

	std::vector<Vertex_PCU> elementSelection;
	elementSelection.reserve( 100 );
	font->AddVertsForText2D( elementSelection, Vec2( 1000.f, 810.f - m_selectingElement * 50.f ), 20.f, ">          <", Rgba8::WHITE, 0.7f );

	g_theRenderer->BeginCamera( m_screenCamera );
	g_theRenderer->BindTexture( &font->GetTexture() );

	if (m_selectingElement == 0) g_theRenderer->DrawVertexArray( m_naviMain );
	else if (m_selectingCategory == 1 && m_selectingElement != 0) g_theRenderer->DrawVertexArray( m_naviAudio );
	else if (m_selectingCategory == 2 && m_selectingElement != 0)
		if (g_theApp->m_disableInput) g_theRenderer->DrawVertexArray( m_naviKeybind1 );
		else g_theRenderer->DrawVertexArray( m_naviKeybind );

	g_theRenderer->SetModelConstants( Mat44(), Rgba8( 255, 255, 255, m_blinkShow || !(m_selectingElement == 0) ? 255 : 0 ) );
	g_theRenderer->DrawVertexArray( categorySelection );

	g_theRenderer->SetModelConstants( Mat44().CreateTranslation2D( m_displayOffset ), Rgba8( 255, g_theApp->m_disableInput ? 0 : 255, g_theApp->m_disableInput ? 0 : 255, m_blinkShow && m_selectingElement > 0 ? 255 : 0 ) );
	g_theRenderer->DrawVertexArray( elementSelection );

	if (m_selectingCategory == 1)
	{
		m_elementsAudio.reserve( 200 );

		font->AddVertsForText2D( m_elementsAudio, Vec2( 700.f, 760.f ), 20.f, "Music Volume", Rgba8::WHITE, 0.7f );
		font->AddVertsForTextInBox2D( m_elementsAudio, AABB2( 1000.f, 760.f, 1168.f, 810.f ), 20.f, " " + std::to_string( (int)(App::g_gameConfig.musicVolume * 100.f) ) + " ", Rgba8::WHITE, 0.7f, Vec2( 0.5f, 0.f ) );

		font->AddVertsForText2D( m_elementsAudio, Vec2( 700.f, 710.f ), 20.f, "SFX   Volume", Rgba8::WHITE, 0.7f );
		font->AddVertsForTextInBox2D( m_elementsAudio, AABB2( 1000.f, 710.f, 1168.f, 760.f ), 20.f, " " + std::to_string( (int)(App::g_gameConfig.sfxVolume * 100.f) ) + " ", Rgba8::WHITE, 0.7f, Vec2( 0.5f, 0.f ) );


		g_theRenderer->SetModelConstants( Mat44().CreateTranslation2D( m_displayOffset ), Rgba8::WHITE );
		g_theRenderer->DrawVertexArray( m_elementsAudio );
	}
	else if (m_selectingCategory == 2)
	{
		m_elementsKeybind.reserve( 1000 );
		int counter = 0;
		auto keybinding = g_theInput->GetKeybinding();

		for (int i = 0; i < g_theInput->m_keybindingOrder.size(); i++)
		{
			if (keybinding[g_theInput->m_keybindingOrder[i]].canBeModified)
			{
				font->AddVertsForText2D( m_elementsKeybind, Vec2( 700.f, 760.f - counter * 50.f ), 20.f, g_theInput->m_keybindingOrder[i], Rgba8::WHITE, 0.7f );
				font->AddVertsForTextInBox2D( m_elementsKeybind, AABB2( 1000.f, 760.f - counter * 50.f, 1168.f, 810.f - counter * 50.f ), 20.f, " " + g_theInput->GetButtonNameByValue( keybinding[g_theInput->m_keybindingOrder[i]].value ) + " ", Rgba8::WHITE, 0.7f, Vec2( 0.5f, 0.f ) );
				counter++;
			}
		}

		g_theRenderer->SetModelConstants( Mat44().CreateTranslation2D( m_displayOffset ), Rgba8::WHITE );
		g_theRenderer->DrawVertexArray( m_elementsKeybind );
	}

	g_theRenderer->SetModelConstants();
	g_theRenderer->DrawVertexArray( m_categorys );

	g_theRenderer->EndCamera( m_screenCamera );
}

void GameSetting::Shutdown()
{
}

int GameSetting::GetGameMode() const
{
	return (int)AppState::IN_SETTING;
}

void GameSetting::InputResponse()
{
	if (m_selectingElement)
	{
		if (m_selectingCategory == 1)
		{
			InputAudio();
		}
		else if (m_selectingCategory == 2)
		{
			InputKeybind();
		}
	}
	else if (m_selectingCategory)
	{
		InputCategory();
	}
}

void GameSetting::InputCategory()
{
	bool hasConnected = g_theInput->GetController( 0 ).isConnected();
	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ) ||
		(hasConnected && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::UP )))
	{
		m_blinkTimer->Start();
		m_selectingCategory--;
		if (m_selectingCategory == 0)
		{
			m_selectingCategory = m_totalCategories;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ) ||
		(hasConnected && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::DOWN )))
	{
		m_blinkTimer->Start();
		m_selectingCategory++;
		if (m_selectingCategory == m_totalCategories + 1)
		{
			m_selectingCategory = 1;
		}
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_ESC ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::B ))
	{
		m_selectingCategory = 0;
		g_theInput->SaveKeyBindingsToXml();
		//g_theApp->SaveGameConfig();
		g_theApp->SetQuitting( true );
	}

	if (g_theInput->IsNewKeyPressed( KEYCODE_SPACE ) ||
		g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::A ))
	{
		m_selectingElement = 1;
		switch (m_selectingCategory)
		{
		case 1:
			m_totalElements = 2;
			break;
		case 2:
			m_totalElements = g_theInput->GetModifiableKeyCount();
			break;
		default:
			break;
		}
	}

}

void GameSetting::InputKeybind()
{
	if (g_theApp->m_disableInput)
	{
		unsigned int latestInput;
		if (g_theInput->m_latestPressedKey != UINT_MAX)
		{
			if (g_theInput->m_latestPressedKey == KEYCODE_ESC)
			{
				g_theApp->m_disableInput = false;
			}
			else
			{
				latestInput = g_theInput->m_latestPressedKey;
				int oldValue = g_theInput->GetKeyValueByName( g_theInput->m_keybindingOrder[m_selectingElement - 1] );
				std::string conflictFunction = g_theInput->GetKeyNameByValue( latestInput );
				g_theInput->UpdateCurrentKeybinding( g_theInput->m_keybindingOrder[m_selectingElement - 1], latestInput );
				g_theInput->UpdateCurrentKeybinding( conflictFunction, oldValue );
				g_theApp->m_disableInput = false;
			}
		}
	}
	else
	{
		if (g_theInput->IsNewKeyPressed( KEYCODE_UP ))
		{
			m_selectingElement--;
			if (m_selectingElement <= 0)
			{
				m_selectingElement = m_totalElements;
			}
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ))
		{
			m_selectingElement++;
			if (m_selectingElement > m_totalElements)
			{
				m_selectingElement = 1;
			}
		}
		if (g_theInput->IsNewKeyPressed( "confirm" ))
		{
			g_theApp->m_disableInput = true;
			g_theInput->m_latestPressedKey = UINT_MAX;
		}
		if (g_theInput->IsNewKeyPressed( "cancel" ))
		{
			m_selectingElement = 0;
			m_displayOffset = Vec2::ZERO;
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELUP ))
		{
			DisplayScrollUp();
		}
		if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELDOWN ))
		{
			DisplayScrollDown();
		}
	}
}

void GameSetting::InputAudio()
{
	if (g_theInput->IsNewKeyPressed( KEYCODE_UP ))
	{
		m_selectingElement--;
		if (m_selectingElement <= 0)
		{
			m_selectingElement = m_totalElements;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_DOWN ))
	{
		m_selectingElement++;
		if (m_selectingElement > m_totalElements)
		{
			m_selectingElement = 1;
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_LEFT ))
	{
		if (m_selectingElement == 1)
		{
			App::g_gameConfig.musicVolume = Clamp( App::g_gameConfig.musicVolume - 0.05f, 0.0, 1.0 );
		}
		else if (m_selectingElement == 2)
		{
			App::g_gameConfig.sfxVolume = Clamp( App::g_gameConfig.sfxVolume - 0.05f, 0.0, 1.0 );
		}
	}
	if (g_theInput->IsNewKeyPressed( KEYCODE_RIGHT ))
	{
		if (m_selectingElement == 1)
		{
			App::g_gameConfig.musicVolume = Clamp( App::g_gameConfig.musicVolume + 0.05f, 0.0, 1.0 );
		}
		else if (m_selectingElement == 2)
		{
			App::g_gameConfig.sfxVolume = Clamp( App::g_gameConfig.sfxVolume + 0.05f, 0.0, 1.0 );
		}
	}


	if (g_theInput->IsNewKeyPressed( "cancel" ))
	{
		m_selectingElement = 0;
		m_displayOffset = Vec2::ZERO;
	}
	// 	if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELUP ))
	// 	{
	// 		DisplayScrollUp();
	// 	}
	// 	if (g_theInput->IsNewKeyPressed( KEYCODE_MOUSEWHEELDOWN ))
	// 	{
	// 		DisplayScrollDown();
	// 	}
}

void GameSetting::DisplayScrollUp()
{
	m_displayOffset -= Vec2( 0.f, 50.f );
}

void GameSetting::DisplayScrollDown()
{
	m_displayOffset += Vec2( 0.f, 50.f );
}
