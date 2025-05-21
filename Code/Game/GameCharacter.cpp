#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/General/ShapeComponents/ShapeComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/General/ShapeComponents/CubeComponent.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/Math/RaycastUtil.hpp"

#include "Game/GameCharacter.hpp"
#include "Game/DamageEffect.hpp"
#include "Game/Game.hpp"

GameCharacter::GameCharacter()
	: Character()
{
	g_eventSystem->SubscribeEventCallBackFunc( "toggleCollision", &ToggleAttack );
}

GameCharacter::GameCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game )
	: Character( name, skeletalMesh )
{
	m_gameRef = game;
}

GameCharacter::GameCharacter( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game )
	: Character( name, meshes, skeleton, textures )
{
	m_gameRef = game;
}

GameCharacter::~GameCharacter()
{
}

float GameCharacter::ReceiveEffect( DamageEffect const& effect )
{
	float drr = GetDamageReductionRate();
	switch (effect.m_damageType)
	{
	case DamageType::DAMAGE_PHYSICAL:
		return effect.m_value * drr;
		break;
	case DamageType::DAMAGE_FIRE:
		return effect.m_value * drr * (1.f - m_fireResist * 0.01f);
		break;
	case DamageType::DAMAGE_WATER:
		return effect.m_value * drr * (1.f - m_waterResist * 0.01f);
		break;
	case DamageType::DAMAGE_THUNDER:
		return effect.m_value * drr * (1.f - m_thunderResist * 0.01f);
		break;
	case DamageType::DAMAGE_ICE:
		return effect.m_value * drr * (1.f - m_iceResist * 0.01f);
		break;
	case DamageType::DAMAGE_DRAGON:
		return effect.m_value * drr * (1.f - m_dragonResist * 0.01f);
		break;
	case DamageType::EFFECT_POSION:
		m_poison += effect.m_value;
		return 0;
		break;
	case DamageType::EFFECT_STUN:
		m_stun += effect.m_value;
		return 0;
		break;
	case DamageType::EFFECT_PARALYSIS:
		m_paralysis += effect.m_value;
		return 0;
		break;
	case DamageType::EFFECT_SLEEP:
		m_sleep += effect.m_value;
		return 0;
		break;
	case DamageType::EFFECT_BLAST:
		m_blast += effect.m_value;
		return 0;
		break;
	}
	return 0;
}

void GameCharacter::ReceiveDamage( float value, bool blocked )
{
	m_health -= value * (blocked ? 0.5f : 1.f);
}

bool GameCharacter::ToggleAttack( Character* character, int collisionIndex, bool flag, std::vector<int>* damageTypeIndex, std::vector<float>* damageValue )
{
	GameCharacter* gameCharacter = (GameCharacter*)character;
	gameCharacter->m_attackStates[collisionIndex].m_attackerRef = gameCharacter;
	if (flag)
	{
		gameCharacter->m_attackStates[collisionIndex].StartAttack();
		for (int i = 0; i < damageTypeIndex->size(); i++)
		{
			gameCharacter->m_attackStates[collisionIndex].m_effects.push_back( DamageEffect( (*damageTypeIndex)[i], (*damageValue)[i] ) );
		}
	}
	else
	{
		gameCharacter->m_attackStates[collisionIndex].EndAttack();
	}
	return false;
}

void GameCharacter::IgnoreCharacterFromCurrentAttack( Character* receiver )
{
	for (auto& attack : m_attackStates)
	{
		attack.second.m_hitTargets.push_back( receiver->GetUID() );
	}
}

bool GameCharacter::DoesAttackHits( GameCharacter* attacker, GameCharacter* receiver )
{
	Vec3 placeholder;

// 	if (!ShapeComponent::DoShapesOverlap( attacker->m_boundingCollision, receiver->m_boundingCollision, placeholder ))
// 		return false;

	auto attackerCollisions = attacker->GetAllCollisionInfo();
	auto receiverCollisions = receiver->GetAllCollisionInfo();

	for (int i = 0; i < attackerCollisions.size(); i++)
	{
		if (attackerCollisions[i].use != CollisionUsage::ATTACK)
			continue;
		if (!attacker->m_collisionsEnabled[i])
			continue;

		ShapeComponent* attackerCollision = nullptr;
		if (attackerCollisions[i].shape == CollisionShape::CAPSULE)
		{
			attackerCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}
		else if (attackerCollisions[i].shape == CollisionShape::SPHERE)
		{
			attackerCollision = new SphereComponent( SphereComponent::CreateSphereComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}

		if (!ShapeComponent::DoShapesOverlap( attackerCollision, receiver->m_boundingCollision, placeholder ))
			continue;

		for (int j = 0; j < receiverCollisions.size(); j++)
		{
			if (receiverCollisions[j].use != CollisionUsage::BODY)
				continue;
			if (!receiver->m_collisionsEnabled[j])
				continue;

			ShapeComponent* receiverCollision = nullptr;
			if (receiverCollisions[j].shape == CollisionShape::CAPSULE)
			{
				receiverCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}
			else if (receiverCollisions[j].shape == CollisionShape::SPHERE)
			{
				receiverCollision = new SphereComponent( SphereComponent::CreateSphereComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}

			if (ShapeComponent::DoShapesOverlap( attackerCollision, receiverCollision, placeholder ))
			{
				float damage = attacker->m_attackStates[i].ApplyAttack( receiver, attackerCollision->GetFunctionalCenter() );

				if (damage > 0 && receiver->m_name == "Paladin")
				{
					Vec3 facingDirection = Vec3( attackerCollision->GetFunctionalCenter().GetXY() - receiver->GetActorWorldPosition().GetXY() );
					Mat44 rotationMat;
					rotationMat.SetIJK3D( facingDirection, facingDirection.GetRotatedAboutZDegrees( 90.f ), Vec3::UP );
					receiver->SetActorWorldOrientation( Quat( rotationMat ) );
				}

				attackerCollision->DetachFromParent();
				receiverCollision->DetachFromParent();
				delete attackerCollision;
				delete receiverCollision;
				return true;
			}
			receiverCollision->DetachFromParent();
			delete receiverCollision;
		}
		attackerCollision->DetachFromParent();
		delete attackerCollision;
	}
	return false;
}

float GameCharacter::GetDamageReductionRate() const
{
	return 80.f / (80.f + m_defense);
}

void GameCharacter::RecoverHP( float amount )
{
	m_health = Clamp( m_health + amount, 0.f, m_maxHealth );
}

float GameCharacter::GetAboveGroundHeight()
{
	float min = FLT_MAX;
	for (int i = 0; i < m_gameRef->m_worldCollisionList.size(); i++)
	{
		if (m_boundingCollision->CollisionResultAgainst( *m_gameRef->m_worldCollisionList[i] ) != CollisionType::BLOCKING)
			break;

		switch (m_gameRef->m_worldCollisionList[i]->GetCollisionShape())
		{
		case CollisionShape::CAPSULE:
		{
// 			RaycastResult3D result;
// 			if (RaycastCapsule3D( result, GetActorWorldPosition(), -Vec3::UP, 100.f, shape->CalculateBoundsOBB3D() ))
// 			{
// 				SetIsGrounded( result.m_impactDist > 0.f );
// 			}
			break;
		}
		case CollisionShape::SPHERE:
		{
			break;
		}
		case CollisionShape::CUBE:
		{
			CubeComponent* cube = dynamic_cast<CubeComponent*>(m_gameRef->m_worldCollisionList[i]);
			RaycastResult3D result;
			CapsuleComponent* boundingCapsule = dynamic_cast<CapsuleComponent*>(m_boundingCollision);
			if (RaycastOBB3D( result, boundingCapsule->GetWorldBoneStart() - Vec3( 0.f, 0.f, boundingCapsule->GetScaledRadius() ), -Vec3::UP, 100.f, cube->CalculateBoundsOBB3D() ))
			{
				min = MIN( min, result.m_impactDist );
			}
			break;
		}
		default:
			break;
		}
	}
	return min;
}

float GameCharacter::IsDead() const
{
	return m_health <= 0.f;
}
