#include <algorithm>

#include "Engine/Math/MathUtils.hpp"

#include "Game/Attack.hpp"
#include "Game/GameCharacter.hpp"
#include "Game/DamageEffect.hpp"
#include "Game/Game.hpp"
#include "Game/PlayerCharacter.hpp"
#include "Game/PlayerController.hpp"


Attack::Attack( GameCharacter* attackerRef )
{
	m_attackerRef = attackerRef;
}

 void Attack::StartAttack()
 {
 	m_hitTargets.clear();
 	m_isActive = true;
 }
 
 float Attack::ApplyAttack( GameCharacter* receiver, Vec3 const& attackCollisionCenter )
 {
 	if (!m_isActive)
 		return 0.f;
 
 	ActorUID receiverUid = receiver->GetUID();
    if (std::find( m_hitTargets.begin(), m_hitTargets.end(), receiverUid ) != m_hitTargets.end())
        return 0.f;
    
    if (receiver->m_immunity)
        return 0.f;

    float totalDamage = 0.f;
    if (receiver->GetName() == "Paladin")
    {
        bool blocked = false;
        PlayerCharacter* player = dynamic_cast<PlayerCharacter*>(receiver);
        if (player)
        {
            PlayerController* playerController = dynamic_cast<PlayerController*>(player->m_controller);
            if (playerController->IsBlocking())
            {
                Vec3 playerToAttackBox = attackCollisionCenter - player->GetActorWorldPosition();
                Vec3 playerForward = player->GetActorWorldOrientation().GetForwardVector();
                if (DotProduct3D( playerForward, playerToAttackBox ) >= 0.5f)
                {
                    blocked = true;
                    playerController->m_blockedSucceeded = true;
                }
            }
			for (auto effect : m_effects)
			{
                if ((int)effect.m_damageType < 6)
                    totalDamage += effect.Apply( receiver );
			}
            receiver->ReceiveDamage( totalDamage, blocked );
            m_attackerRef->IgnoreCharacterFromCurrentAttack( receiver );
        }
    }
    else
    {
		for (auto& effect : m_effects)
		{
			totalDamage += effect.Apply( receiver );
		}
        receiver->ReceiveDamage( totalDamage );
        m_attackerRef->IgnoreCharacterFromCurrentAttack( receiver );
    }

    

    if (receiver->GetName() != "Paladin")
    {
        if (receiver->m_targetUID == ActorUID::INVALID)
        {
            receiver->m_targetUID = m_attackerRef->GetUID();
        }
        else
        {
            if (m_attackerRef->GetUID() == receiver->m_gameRef->GetPlayerUID())
            {
                receiver->m_targetUID = m_attackerRef->GetUID();
            }
        }
    }
    return totalDamage;
 }
 
 void Attack::EndAttack()
 {
 	m_isActive = false;
    m_effects.clear();
 }
