#include "Game/DamageEffect.hpp"
#include "Game/GameCharacter.hpp"

DamageEffect::DamageEffect( int typeIndex, float m_value )
	: m_damageType( (DamageType)typeIndex )
	, m_value( m_value )
{
}

float DamageEffect::Apply( GameCharacter* receiver )
{
	return receiver->ReceiveEffect( *this );
}

