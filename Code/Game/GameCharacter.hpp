#pragma once

#include "Game/Attack.hpp"

#include "Engine/General/Character.hpp"

class Attack;
class DamageEffect;
class GameRun;

class GameCharacter : public Character
{
public:
	GameCharacter();
	GameCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game );
	GameCharacter( std::string name, std::vector<MeshT*>meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game );
	virtual ~GameCharacter();

	virtual float ReceiveEffect( DamageEffect const& effect );
	virtual void ReceiveDamage( float value, bool blocked = false );

	static bool ToggleAttack( Character* character, int collisionIndex, bool flag, std::vector<int>* damageTypeIndex, std::vector<float>* damageValue );

	virtual void RecoverHP( float amount );

public:
	std::unordered_map<int, Attack> m_attackStates;
	void IgnoreCharacterFromCurrentAttack( Character* receiver );

	static bool DoesAttackHits( GameCharacter* attacker, GameCharacter* receiver );

public:
	float GetDamageReductionRate() const;

public:
	float GetAboveGroundHeight() override;
	float m_aboveGroundHeight = 0.f;

	float IsDead() const;

	bool m_isInvincible = false;

// public:
// 	void TurnAboutZDegree( float degree );

public:
	GameRun* m_gameRef = nullptr;
	ActorUID m_targetUID = ActorUID::INVALID;

public:
	bool m_immunity = false;

public:
	float m_health;
	float m_stamina;
	float m_attack;
	float m_defense;
	float m_poison;
	float m_stun;
	float m_paralysis;
	float m_sleep;
	float m_blast;

	float m_fireResist;
	float m_waterResist;
	float m_thunderResist;
	float m_iceResist;
	float m_dragonResist;

	float m_maxHealth;
	float m_maxStamina;
	float m_baseAttack;
	float m_baseDefense;
	float m_maxPoisonPool;
	float m_maxStunPool;
	float m_maxParalysisPool;
	float m_maxSleepPool;
	float m_maxBlastPool;
};
