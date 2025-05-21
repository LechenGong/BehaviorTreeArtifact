#pragma once

class GameCharacter;

enum class DamageType
{
	DAMAGE_PHYSICAL,
	DAMAGE_FIRE,
	DAMAGE_WATER,
	DAMAGE_THUNDER,
	DAMAGE_ICE,
	DAMAGE_DRAGON,
	EFFECT_POSION,
	EFFECT_STUN,
	EFFECT_PARALYSIS,
	EFFECT_SLEEP,
	EFFECT_BLAST,
};

class DamageEffect
{
public:
	DamageEffect( int typeIndex, float m_value );

	float Apply( GameCharacter* receiver );

public:
	DamageType const m_damageType;
	float const m_value;
};
