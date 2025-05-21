#pragma once

#include <vector>

#include "Engine/General/ActorUID.hpp"
#include "Engine/Math/Vec3.hpp"

#include "Game/DamageEffect.hpp"

class GameCharacter;

class Attack
{
public:
	std::vector<ActorUID> m_hitTargets;
	bool m_isActive = false;
	GameCharacter* m_attackerRef = nullptr;
	std::vector<DamageEffect> m_effects;

	Attack() {};
	Attack( GameCharacter* attackerRef );

	void StartAttack();
	float ApplyAttack( GameCharacter* receiver, Vec3 const& attackCollisionCenter );
	void EndAttack();
};