#include "Engine/Core/EngineCommon.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"
#include "Game/AI/Bullfango/BullfangoCharacter.hpp"
#include "Game/Game.hpp"

BullfangoController::BullfangoController( Character* charaRef )
	: Controller( charaRef )
{
	//
}

BullfangoController::~BullfangoController()
{
}

void BullfangoController::InputResponse( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void BullfangoController::Update( float deltaSeconds )
{
	InputResponse( deltaSeconds );

	BullfangoCharacter* chara = dynamic_cast<BullfangoCharacter*>(m_possessedChara);
	GameCharacter* targetRef = dynamic_cast<GameCharacter*>(chara->m_gameRef->GetActorByUID( chara->m_targetUID ));
	
	m_isImpact = false;

	if (chara->m_health < m_healthPreviousFrame)
	{
		m_isImpact = true;
	}
	m_healthPreviousFrame = chara->m_health;
	if (chara->m_health <= 0.f)
	{
		m_isDying = true;
	}

	if (!targetRef) // off combat
	{
		TickBehaviorTree( deltaSeconds );
	}
	else // in combat
	{
		if (!m_isBTFinished)
		{
			BehaviorTree::Context btContext;
			btContext.chara = m_possessedChara;
			m_btTree->Abort( &btContext );
		}

		if (chara->m_health < m_healthPreviousFrame)
		{
			m_isImpact = true;
		}
		m_healthPreviousFrame = chara->m_health;

		if (!m_isAttacking)
		{
			m_isAttacking = true;
		}
	}
}

bool BullfangoController::IsSprinting() const
{
	return m_isSprinting;
}

bool BullfangoController::IsImpact() const
{
	return m_isImpact;
}

bool BullfangoController::IsTurning() const
{
	return m_isTurning;
}

bool BullfangoController::IsFalling() const
{
	return m_isFalling;
}

bool BullfangoController::IsDying() const
{
	return m_isDying;
}

bool BullfangoController::IsAttemptingToMove() const
{
	return false;
}

bool BullfangoController::IsAttacking() const
{
	return m_isAttacking;
}

void BullfangoController::ResetAllState()
{
	m_isSprinting = false;
	m_isImpact = false;
	m_isTurning = false;
	m_isAttacking = false;
	m_isDying = false;
}

bool BullfangoController::IsMovementHindered() const
{
	return m_isAttacking || m_isImpact || m_isTurning || m_isDying;
}

Vec3 BullfangoController::GetMoveDirection() const
{
	return Vec3();
}
