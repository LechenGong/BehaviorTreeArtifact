#pragma once

#include <vector>
#include <unordered_map>

#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/General/ActorUID.hpp"

#include "Game/QuestInfo.hpp"
#include "Game/ZoneInfo.hpp"

class App;
class Clock;
class Timer;
class PlayerController;
class PlayerCharacter;
class Character;
class CharacterDefinition;
class Actor;
class AnimationState;
class CubeComponent;
class CapsuleComponent;
class SphereComponent;
class ShapeComponent;
class StaticMesh;
class SkeletalMesh;
class StaticMeshComponent;
struct CollisionInfo;
struct OBB3;
class BehaviorTree;

class Game
{
public:
	Game() {};
	virtual ~Game() = default;

	virtual void Startup() = 0;
	virtual void Update( float deltaSeconds ) = 0;
	virtual void Render() const = 0;
	virtual void Shutdown() = 0;

	virtual void UpdateImGui() {};

	//Input Game----------------------------------
	virtual void InputResponse() = 0;

	virtual int GetGameMode() const = 0;
	virtual float GetDeltaSeconds() const = 0;

	Clock* m_clock = nullptr;

protected:
	float m_deltaSeconds;
};

class GameRun : public Game
{
public:
	GameRun();
	~GameRun();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void RenderUI() const;
	void Shutdown() override;

	void InitializeCharacters();
	void InitializeEnvironment();
	void InitializeWorldCollisions();
	void InitializeBehaviorTrees();
	void SpawnCharacters();
	void SpawnEnvironment();

	void UpdateCollision();

	void UpdateImGui() override;

	//Input Game----------------------------------
	void InputResponse() override;

	int GetGameMode() const override;
	float GetDeltaSeconds() const override { return m_deltaSeconds; }

	Actor* GetActorByUID( ActorUID const& uid ) const;
	ActorUID GetPlayerUID() const;

	std::vector<ShapeComponent*> m_worldCollisionList;

	static std::unordered_map<std::string, std::unordered_map<std::string, AnimationState*>> g_animStates; // [CharaName][AnimName]
	static std::unordered_map<std::string, SkeletalMesh*> g_skeletonMeshes;	// [CharaName][Mesh]
	static std::unordered_map<std::string, std::vector<CollisionInfo>> g_collisionComponents; // [CharaName][CollisionBoxesInfo]
	static std::unordered_map<std::string, CharacterDefinition> g_characterDefs; // [CharacterName][Definition]
	static std::unordered_map<std::string, StaticMesh*> g_staticMeshes; // [AssetName][Mesh]
	static std::unordered_map<std::string, BehaviorTree*> g_behaviorTrees; // [CharaName][TreeDefinition]

	bool m_debugdraw = false;

	static const unsigned int MAX_ACTOR_SALT = 0x0000FFFEu;
	unsigned int m_actorSalt = MAX_ACTOR_SALT + 1;
	std::vector<Actor*> m_actors;
	std::vector<StaticMeshComponent*> m_assets;

private:
	Camera m_screenCamera;
	Camera m_worldCamera;
	Camera m_debugCamera;
	PlayerCharacter* m_player = nullptr;
	PlayerController* m_playerController = nullptr;
	Timer* m_timer = nullptr;

	

	int m_frameRate;
	int m_realFrameRate;
	double m_actualFrameTime;

	EulerAngles m_sunOrientation = EulerAngles( 120.f, 45.f, 0.f );
	float m_sunIntensity = 0.f;
	float m_ambientIntensity = 1.f;

	QuestInfo m_questInfo;
	ZoneInfo m_zoneInfo;
};

class GameAttract : public Game
{
public:
	GameAttract();
	~GameAttract();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void Shutdown() override;
	int GetGameMode() const override;
	float GetDeltaSeconds() const override { return m_deltaSeconds; }

	void InputResponse() override;
	void MenuFunctionButton();

private:
	void RollingAnimation( float directionModifier, float deltaSeconds );
	void AbortAnimation();
	void ResetRollAnim();

	enum class NextState
	{
		ToGold,
		ToSetting,
		ToQuit,
		StateCount
	};
	std::vector<Vertex_PCU> PullPaintBaseOnState( NextState state ) const;
	NextState m_selectedState = NextState::ToGold;
	std::vector<Vertex_PCU> m_shapes[5];
	float const ROLL_TIME = 0.7f;
	float m_rollTimer = 0.f;
	bool m_isRolling = false;
	float m_rollingOffset = 0.f;
	float m_rollingDirection = 0.f;

private:
	Camera m_screenCamera;
	Timer* m_blinkTimer = nullptr;

	Vec3 m_sunDirection = Vec3( 2.f, 1.f, -1.f );
	float m_sunIntensity = 0.85f;
	float m_ambientIntensity = 0.35f;

	void RenderUI() const;

	float m_age = 0.f;
};

class GameSetting : public Game
{
public:
	GameSetting();
	~GameSetting();

	void Startup() override;
	void Update( float deltaSeconds ) override;
	void Render() const override;
	void Shutdown() override;
	int GetGameMode() const override;
	float GetDeltaSeconds() const override { return m_deltaSeconds; }

	void InputResponse() override;
	void InputCategory();
	void InputKeybind();
	void InputAudio();

	void DisplayScrollUp();
	void DisplayScrollDown();

	Camera m_screenCamera;

private:
	int m_totalCategories = 2;
	int m_totalElements;
	// 0 = false, positive number = index
	int m_selectingCategory = 0;
	// 0 = false, positive number = index
	int m_selectingElement = 0;

private:
	std::vector<Vertex_PCU> m_categorys;
	Vec2 m_displayOffset = Vec2::ZERO;
	Timer* m_blinkTimer = nullptr;
	bool m_blinkShow = true;

	std::vector<Vertex_PCU> m_naviMain;
	std::vector<Vertex_PCU> m_naviAudio;
	std::vector<Vertex_PCU> m_naviKeybind;
	std::vector<Vertex_PCU> m_naviKeybind1;
};