#include "Game/PlayerController.hpp"
#include "Engine/General/Character.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Game/GameCharacter.hpp"
#include "Game/PlayerCharacter.hpp"

PlayerController::PlayerController()
{
}

PlayerController::PlayerController( Character* m_charaRef )
	: Controller( m_charaRef )
{
	GameCharacter* gameChara = dynamic_cast<GameCharacter*>(m_possessedChara);
	m_healthPreviousFrame = gameChara->m_health;
}

PlayerController::~PlayerController()
{
}

void PlayerController::InputResponse( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if (Window::IsWindowFocused())
	{
		m_possessedChara->m_camera.m_orientation.m_yawDegrees += g_theInput->GetCursorClientDelta().x * -0.1f;
		m_possessedChara->m_camera.m_orientation.m_pitchDegrees += g_theInput->GetCursorClientDelta().y * 0.1f;
		DebugAddMessage( Stringf( "%f", m_possessedChara->m_camera.m_orientation.m_pitchDegrees ).c_str(), deltaSeconds );
	}
	Mat44 rotationMatrix = m_possessedChara->m_camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp();
	Vec3 forwardVec = Vec3( rotationMatrix.GetIBasis3D().GetXY().GetNormalized() );
	Vec3 leftVec = Vec3( rotationMatrix.GetJBasis3D().GetXY().GetNormalized() );
	Vec3 upVec = Vec3( rotationMatrix.GetKBasis3D().GetXY().GetNormalized() );
	bool hasController = g_theInput->GetController( 0 ).isConnected();

	m_isSprinting = false;
	m_isImpact = false;
	
	m_moveDirection = Vec3::ZERO;

	PlayerCharacter* player = dynamic_cast<PlayerCharacter*>(m_possessedChara);

	if (g_theInput->WasKeyJustReleased( "dash" ))
	{
		m_unableToDash = false;
	}

	if (deltaSeconds > 0.f && !player->IsDead() && !IsMovementHindered())
	{
		if (g_theInput->IsKeyDown( "dash" ) || // LeftShift
			(hasController && g_theInput->GetController( 0 ).IsButtonDown( XboxButtonID::RS )))
		{	
			if (!m_isUsingItem && !m_movementDisabled)
			{
				float staminaCost = 10.f * deltaSeconds;
				if (player->m_stamina > staminaCost && !m_unableToDash)
				{
					m_isSprinting = true;
				}
				else
				{
					m_unableToDash = true;
				}
			}
			else
			{
				m_isSprinting = false;
			}
		}

		if (g_theInput->IsKeyDown( "forward" ) ||
			(hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y > 0))
		{
			float ifControllerParam = (hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y > 0) ? g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y : 1.f;
			m_moveDirection += forwardVec * abs( ifControllerParam );
		}
		if (g_theInput->IsKeyDown( "backward" ) ||
			(hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y < 0))
		{
			float ifControllerParam = (hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y < 0) ? g_theInput->GetController( 0 ).GetLeftStick().GetPosition().y : 1.f;
			m_moveDirection -= forwardVec * abs( ifControllerParam );
		}
		if (g_theInput->IsKeyDown( "leftward" ) ||
			(hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x < 0))
		{
			float ifControllerParam = (hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x < 0) ? g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x : 1.f;
			m_moveDirection += leftVec * abs( ifControllerParam );
		}
		if (g_theInput->IsKeyDown( "rightward" ) ||
			(hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x > 0))
		{
			float ifControllerParam = (hasController && g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x > 0) ? g_theInput->GetController( 0 ).GetLeftStick().GetPosition().x : 1.f;
			m_moveDirection -= leftVec * abs( ifControllerParam );
		}
	}

	GameCharacter* gameChara = dynamic_cast<GameCharacter*>(m_possessedChara);
	if (gameChara->m_health < m_healthPreviousFrame)
	{
		m_isImpact = true;
	}
	m_healthPreviousFrame = gameChara->m_health;
	if (gameChara->m_health <= 0.f)
	{
		m_isDying = true;
	}

	m_moveDirection = m_moveDirection.GetNormalized();

	if (IsAttemptingToMove() && DotProduct2D( m_moveDirection.GetXY(), m_possessedChara->GetActorWorldOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY() ) < -0.707f)
	{
		m_isTurning = true;
	}

	if (m_possessedChara->GetIsGrounded() || m_possessedChara->GetAboveGroundHeight() <= 0.1f)
	{
		if (g_theInput->IsKeyDown( "block" ) ||
			(hasController && g_theInput->GetController( 0 ).IsButtonDown( XboxButtonID::RS )))
		{
			if (!m_isAttacking && !m_isRolling && !m_isUsingItem && !m_isTurning && !m_isDying)
			{
				m_isBlocking = true;
				m_isUnblocking = false;
			}
		}
		else if (g_theInput->WasKeyJustReleased( "block" ) ||
			(hasController && g_theInput->GetController( 0 ).WasButtonJustReleased( XboxButtonID::RS )))
		{
			if (m_isBlocking)
			{
				m_isBlocking = false;
				m_isUnblocking = true;
			}
		}

		if (g_theInput->IsKeyDown( "dodge" ) ||
			(hasController && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::A )))
		{
			if (!m_isRolling && !m_isBlocking && !m_isUnblocking && !m_isUsingItem && !m_isImpact && !m_isTurning && !m_isDying && !m_isFalling && !m_movementDisabled)
			{
				if (player->m_stamina >= 25.f)
				{
					Vec3 velo = m_possessedChara->GetVelocity();
					m_possessedChara->AddVelocity( Vec3( 0.f, 0.f, 5.f ) );

					m_isRolling = true;
					player->m_stamina -= 25.f;
					player->m_notUsingStaminaTimer = 0.f;
				}
			}
		}

		if (g_theInput->IsKeyDown( "heavyAttack" ) || g_theInput->IsKeyDown( "lightAttack" ) ||
			(hasController && g_theInput->GetController( 0 ).IsNewButtonDown( XboxButtonID::X )))
		{
			if (!m_isAttacking && !m_isRolling && !m_isBlocking && !m_isUnblocking && !m_isUsingItem && !m_isImpact && !m_isTurning && !m_isDying)
			{
				m_isAttacking = true;
			}
		}

		if (g_theInput->IsKeyDown( "item" ))
		{
			if (!m_isAttacking && !m_isRolling && !m_isBlocking && !m_isUnblocking && !m_isUsingItem && !m_isImpact && !m_isTurning && !m_isDying)
			{
				m_isUsingItem = true;
			}
		}
	}

	m_possessedChara->m_camera.m_orientation.m_pitchDegrees = Clamp( m_possessedChara->m_camera.m_orientation.m_pitchDegrees, -85.f, 85.f );
	m_possessedChara->m_camera.m_orientation.m_rollDegrees = Clamp( m_possessedChara->m_camera.m_orientation.m_rollDegrees, -45.f, 45.f );
}

void PlayerController::Update( float deltaSeconds )
{
	InputResponse( deltaSeconds );
}

bool PlayerController::IsSprinting() const
{
	return m_isSprinting;
}

bool PlayerController::IsImpact() const
{
	return m_isImpact;
}

bool PlayerController::IsTurning() const
{
	return m_isTurning;
}

bool PlayerController::IsFalling() const
{
	return m_isFalling;
}

bool PlayerController::IsDying() const
{
	return m_isDying;
}

bool PlayerController::IsAttemptingToMove() const
{
	return m_moveDirection.GetLengthXYSquared() > 0.f/* && ( m_possessedChara->GetIsGrounded() || m_possessedChara->GetAboveGroundHeight() <= 1.f)*/;
}

bool PlayerController::IsAttacking() const
{
	return m_isAttacking;
}

bool PlayerController::IsRolling() const
{
	return m_isRolling;
}

bool PlayerController::IsBlocking() const
{
	return m_isBlocking;
}

bool PlayerController::IsUnblocking() const
{
	return m_isUnblocking;
}

bool PlayerController::IsMovementHindered() const
{
	return m_isAttacking || m_isBlocking || m_isRolling || m_isImpact || m_isTurning || m_isDying;
}

bool PlayerController::IsUsingItem() const
{
	return m_isUsingItem;
}

void PlayerController::ResetAllState()
{
	m_isSprinting = false;
	m_isImpact = false;
	m_isTurning = false;
	m_isAttacking = false;
	m_isRolling = false;
	//m_isBlocking = false;
	m_isUnblocking = false;
	//m_isUsingItem = false;
	m_blockedSucceeded = false;
	m_isDying = false;

	m_movementDisabled = false;
	m_turningDisabled = false;

	if (m_moveDirection.GetLengthSquared() > 0 && 
		DotProduct2D( m_moveDirection.GetXY(), m_possessedChara->GetActorWorldOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY() ) <= -0.414f)
		m_isTurning = true;
}

void PlayerController::ResetUsingItem()
{
	m_isUsingItem = false;
}

Vec3 PlayerController::GetMoveDirection() const
{
	return m_moveDirection;
}
