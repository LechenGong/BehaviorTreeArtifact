#include "Game/PlayerAnimController.hpp"
#include "Game/PlayerCharacter.hpp"
#include "Game/PlayerController.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"

#include "Game/App.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Engine/General/SkeletalMesh.hpp"

PlayerAnimController::PlayerAnimController()
	: AnimationController()
{
}

PlayerAnimController::PlayerAnimController( Character* character )
	: AnimationController( character )
{
	GetStateMachine()->InitializeState( "Paladin", GameRun::g_animStates );
}

PlayerAnimController::~PlayerAnimController()
{
}

void PlayerAnimController::Update( float deltaSeconds )
{
	m_stateMachine->Update( deltaSeconds );

	if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
	{
		m_charaRef->m_controller->ResetAllState();
	}

	for (int i = 0; i < m_stateMachine->GetOngoingAnimations().size(); i++)
	{
		AnimationState* currentState = m_stateMachine->GetOngoingAnimation( i ).GetCurrentState();

		if (!currentState)
			continue;

		std::string currentStateName = currentState->GetStateName();

		PlayerController* playerCtrl = dynamic_cast<PlayerController*>(m_charaRef->m_controller);

		if (!playerCtrl)
			ERROR_AND_DIE( "Charater doesn't have controller" );

		if (currentStateName != "Knocking Down")
		{
			if (playerCtrl->IsImpact())
			{
				if (playerCtrl->IsDying())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Dying" );
					m_stateMachine->GetOngoingAnimation( 1 ).TransitTo( "", 0.f, true );
					playerCtrl->ResetUsingItem();
					playerCtrl->m_movementDisabled = true;
					playerCtrl->m_turningDisabled = true;
					return;
				}
				else
				{
					if (playerCtrl->m_blockedSucceeded)
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Impact Blocking", 0.f );
						playerCtrl->m_blockedSucceeded = false;
						playerCtrl->m_movementDisabled = true;
						playerCtrl->m_turningDisabled = true;
					}
					else
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Knocking Down" );
						m_stateMachine->GetOngoingAnimation( 1 ).TransitTo( "", 0.f, true );
						playerCtrl->ResetUsingItem();
						m_charaRef->GetSkeletalMesh()->SetVisibility( false, 2 );
						m_charaRef->GetSkeletalMesh()->SetVisibility( false, 3 );
						playerCtrl->m_movementDisabled = true;
						playerCtrl->m_turningDisabled = true;
						dynamic_cast<PlayerCharacter*>(m_charaRef)->m_isInvincible = true;
					}
					return;
				}
			}
			else if (currentStateName != "Falling" && m_charaRef->m_offGroundTimer >= 0.3f)
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Falling", 0.3f, true );
				playerCtrl->SetIsFalling( true );
				playerCtrl->m_movementDisabled = true;
			}
		}

		if (currentStateName == "Idle")
		{
			if (playerCtrl->IsTurning())
			{
				Vec3 selfLocation = m_charaRef->GetActorWorldPosition();
				Vec2 selfForwardNormal = m_charaRef->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
				float dotP = DotProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
				//float crossP = CrossProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
				if (dotP <= -0.707f)
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "180 Turning Walking" );
				}
				// 				else
				// 				{
				// 					if (crossP > 0)
				// 					{
				// 						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Left" );
				// 					}
				// 					else
				// 					{
				// 						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Right" );
				// 					}
				// 				}
			}
			else if (playerCtrl->IsBlocking())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Blocking", 0.1f );
			}
			else if (playerCtrl->IsRolling())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling", 0.2f );
			}
			else if (playerCtrl->IsAttacking())
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Knock" );
				}
				else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Slash" );
				}
			}
			else if (playerCtrl->IsAttemptingToMove())
			{
				if (playerCtrl->IsSprinting())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Running" );
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking" );
				}
			}
			if (playerCtrl->IsUsingItem())
			{
				if (!m_stateMachine->GetOngoingAnimation( 1 ).currentState ||
					m_stateMachine->GetOngoingAnimation( 1 ).blendAlpha == 0.f)
				{
					m_stateMachine->GetOngoingAnimation( 1 ).TransitTo( "Drinking", (m_stateMachine->GetOngoingAnimation( 1 ).currentState && m_stateMachine->GetOngoingAnimation( 1 ).blendAlpha > 0.f) ? 0.3f : 0.f );
					m_stateMachine->GetOngoingAnimation( 1 ).rootJoint = "mixamorig:Spine";
					m_stateMachine->GetOngoingAnimation( 1 ).blendFullDuration = 0.3f;
					m_stateMachine->GetOngoingAnimation( 1 ).blendRemainTimer = 0.3f;
				}
			}
		}
		else if (currentStateName == "180 Turning Walking" ||
			currentStateName == "90 Turning Left" ||
			currentStateName == "90 Turning Right")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (playerCtrl->IsAttemptingToMove())
				{
					if (playerCtrl->IsTurning())
					{
						Vec3 selfLocation = m_charaRef->GetActorWorldPosition();
						Vec2 selfForwardNormal = m_charaRef->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
						float dotP = DotProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
						//float crossP = CrossProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
						if (dotP <= -0.707f)
						{
							m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "180 Turning Walking" );
						}
						// 						else
						// 						{
						// 							if (crossP > 0)
						// 							{
						// 								m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Left" );
						// 							}
						// 							else
						// 							{
						// 								m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Right" );
						// 							}
						// 						}
					}
					else
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking" );
					}
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
				}
			}
		}
		else if (currentStateName == "180 Turning Running")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (playerCtrl->IsAttemptingToMove())
				{
					if (playerCtrl->IsTurning())
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "180 Turning Running" );
					}
					else
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Running" );
					}
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
				}
			}
		}
		else if (currentStateName == "Running")
		{
			if (playerCtrl->IsTurning())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "180 Turning Running" );
			}
			if (playerCtrl->IsBlocking())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Blocking" );
			}
			else if (playerCtrl->IsRolling())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling", 0.2f );
			}
			else if (playerCtrl->IsAttacking())
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Spinning Slash" );
				}
				else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Jump" );
				}
			}
			else if (playerCtrl->IsAttemptingToMove())
			{
				if (!playerCtrl->IsSprinting())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking" );
				}
			}
			else
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
		}
		else if (currentStateName == "Walking")
		{
			if (playerCtrl->IsTurning())
			{
				Vec3 selfLocation = m_charaRef->GetActorWorldPosition();
				Vec2 selfForwardNormal = m_charaRef->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
				float dotP = DotProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
				//float crossP = CrossProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
				if (dotP <= -0.707f)
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "180 Turning Walking" );
				}
				// 				else
				// 				{
				// 					if (crossP > 0)
				// 					{
				// 						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Left" );
				// 					}
				// 					else
				// 					{
				// 						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "90 Turning Right" );
				// 					}
				// 				}
			}
			if (playerCtrl->IsBlocking())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Blocking" );
			}
			else if (playerCtrl->IsRolling())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling", 0.2f );
			}
			else if (playerCtrl->IsAttacking())
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Knock" );
				}
				else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Slash" );
				}
			}
			else if (playerCtrl->IsAttemptingToMove())
			{
				if (playerCtrl->IsSprinting())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Running" );
				}
			}
			else
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			if (playerCtrl->IsUsingItem())
			{
				if (!m_stateMachine->GetOngoingAnimation( 1 ).currentState ||
					m_stateMachine->GetOngoingAnimation( 1 ).blendAlpha == 0.f)
				{
					m_stateMachine->GetOngoingAnimation( 1 ).TransitTo( "Drinking", (m_stateMachine->GetOngoingAnimation( 1 ).currentState && m_stateMachine->GetOngoingAnimation( 1 ).blendAlpha > 0.f) ? 0.3f : 0.f );
					m_stateMachine->GetOngoingAnimation( 1 ).rootJoint = "mixamorig:Spine";
					m_stateMachine->GetOngoingAnimation( 1 ).blendFullDuration = 0.3f;
					m_stateMachine->GetOngoingAnimation( 1 ).blendRemainTimer = 0.3f;
				}
			}
		}
		else if (currentStateName == "Drinking")
		{
			if (m_stateMachine->GetOngoingAnimation( 1 ).HasCurrentAnimationEnded())
			{
				playerCtrl->ResetUsingItem();
			}
			if (m_stateMachine->GetOngoingAnimation( 1 ).currentAnimationTimer < 1.0f &&
				m_stateMachine->GetOngoingAnimation( 1 ).currentAnimationTimer + deltaSeconds >= 1.0f &&
				m_stateMachine->GetOngoingAnimation( 1 ).currentState != nullptr)
			{
				dynamic_cast<PlayerCharacter*>(m_charaRef)->RecoverHP( 30.f );
			}
		}
		else if (currentStateName == "Knocking Down")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (m_charaRef->GetIsGrounded())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Getting Up" );
					m_charaRef->GetSkeletalMesh()->SetVisibility( true, 2 );
					m_charaRef->GetSkeletalMesh()->SetVisibility( true, 3 );
				}
				playerCtrl->m_movementDisabled = true;
				playerCtrl->m_turningDisabled = true;
			}
		}
		else if (currentStateName == "Getting Up")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
				dynamic_cast<PlayerCharacter*>(m_charaRef)->m_isInvincible = false;
			}
		}
		else if (currentStateName == "Atk Slash")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 0.76f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 0.85f)
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Knock" );
				}
				else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Smash" );
				}
			}
		}
		else if (currentStateName == "Atk Smash")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() <= SIMUTANEOUS_PRESS_THRESHOLD)
			{
				if (g_theInput->WereNewKeysPressedRecently( { "lightAttack", "heavyAttack" } ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Combo", 0.3f, true, m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() );
				}
			}
		}
		else if (currentStateName == "Atk Knock")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 0.65f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() <= SIMUTANEOUS_PRESS_THRESHOLD)
			{
				if (g_theInput->WereNewKeysPressedRecently( { "lightAttack", "heavyAttack" } ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Combo", 0.3f, true, m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() );
				}
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() >= 0.73)
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ) || g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Rise" );
				}
			}
		}
		else if (currentStateName == "Atk Kick")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 0.88f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() <= SIMUTANEOUS_PRESS_THRESHOLD)
			{
				if (g_theInput->WereNewKeysPressedRecently( { "lightAttack", "heavyAttack" } ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Combo", 0.3f, true, m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() );
				}
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() >= 0.97)
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ) || g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Slash", 0.3f );
				}
			}
		}
		else if (currentStateName == "Atk Rise")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.36f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.38f)
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Kick" );
				}
				else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Smash" );
				}
			}
		}
		else if (currentStateName == "Atk Spinning Slash")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.1f)
			{
				if (g_theInput->IsNewKeyPressed( "lightAttack" ) || g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Spinning Fade Slash" );
				}
				if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.3f)
				{
					if (playerCtrl->IsRolling())
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling", 0.2f );
					}
				}
			}
		}
		else if (currentStateName == "Atk Spinning Fade Slash")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 0.8f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() <= SIMUTANEOUS_PRESS_THRESHOLD)
			{
				if (g_theInput->WereNewKeysPressedRecently( { "lightAttack", "heavyAttack" } ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Combo", 0.3f, true, m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() );
				}
			}
		}
		else if (currentStateName == "Atk Jump")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.32f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 1.42f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
				if (g_theInput->IsNewKeyPressed( "lightAttack" ) || g_theInput->IsNewKeyPressed( "heavyAttack" ))
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Spinning Fade Slash" );
				}
			}
		}
		else if (currentStateName == "Atk Combo")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle" );
			}
			else if (m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime() > 3.2f)
			{
				if (playerCtrl->IsRolling())
				{
					TurnBeforeRoll();
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Rolling" );
				}
			}
		}
		else if (currentStateName == "Rolling")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (playerCtrl->IsAttacking())
				{
					if (g_theInput->IsNewKeyPressed( "lightAttack" ))
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Knock" );
					}
					else if (g_theInput->IsNewKeyPressed( "heavyAttack" ))
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Atk Slash" );
					}
				}
				else if (playerCtrl->IsAttemptingToMove())
				{
					if (playerCtrl->IsSprinting())
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Running", 0.2f );
					}
					else
					{
						m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking", 0.2f );
					}
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.2f );
				}
			}
		}
		else if (currentStateName == "Blocking")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (playerCtrl->IsBlocking())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle Blocking", 0.1f );
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Unblocking", 0.1f );
				}
			}
		}
		else if (currentStateName == "Unblocking")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.1f );
			}
		}
		else if (currentStateName == "Idle Blocking")
		{
			if (!playerCtrl->IsBlocking())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Unblocking", 0.1f );
			}
		}
		else if (currentStateName == "Impact Blocking")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				if (!playerCtrl->IsBlocking())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Unblocking", 0.1f );
				}
				else if (playerCtrl->IsBlocking())
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Blocking", 0.1f );
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.1f );
				}
			}
		}
		else if (currentStateName == "Falling")
		{
			if (m_charaRef->GetIsGrounded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Landing Stand", 0.f, true, 0.2f );
				playerCtrl->m_movementDisabled = true;
				playerCtrl->m_turningDisabled = true;
			}
		}
		else if (currentStateName == "Landing Stand")
		{
			if (m_stateMachine->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.1f );
				playerCtrl->SetIsFalling( false );
			}
		}
	}
}

void PlayerAnimController::TurnBeforeRoll()
{
	PlayerController* playerCtrl = dynamic_cast<PlayerController*>(m_charaRef->m_controller);

	if (playerCtrl->GetMoveDirection() == Vec3::ZERO)
		return;

	Vec3 selfLocation = m_charaRef->GetActorWorldPosition();
	Vec2 selfForwardNormal = m_charaRef->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
	float dotP = DotProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
	float crossP = CrossProduct2D( selfForwardNormal, playerCtrl->GetMoveDirection().GetXY().GetNormalized() );
	if (dotP > 0.707)
	{
		//
	}
	else if (dotP <= -0.707)
	{
		EulerAngles euler = m_charaRef->GetActorWorldOrientation().ToEulerAngles();
		euler.m_yawDegrees += 180.f;
		m_charaRef->SetActorWorldOrientation( euler );
	}
	else
	{
		if (crossP > 0)
		{
			EulerAngles euler = m_charaRef->GetActorWorldOrientation().ToEulerAngles();
			euler.m_yawDegrees += 90.f;
			m_charaRef->SetActorWorldOrientation( euler );
		}
		else
		{
			EulerAngles euler = m_charaRef->GetActorWorldOrientation().ToEulerAngles();
			euler.m_yawDegrees -= 90.f;
			m_charaRef->SetActorWorldOrientation( euler );
		}
	}
}
