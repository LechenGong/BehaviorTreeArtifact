#include "Game/AI/Bullfango/BullfangoAnimController.hpp"
#include "Game/AI/Bullfango/BullfangoCharacter.hpp"
#include "Game/Game.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"

#include "Game/GameCharacter.hpp"

BullfangoAnimController::BullfangoAnimController()
	: AnimationController()
{
}

BullfangoAnimController::BullfangoAnimController( Character* character )
	: AnimationController( character )
{
	GetStateMachine()->InitializeState( "Bullfango", GameRun::g_animStates );
}

BullfangoAnimController::~BullfangoAnimController()
{
}
#include "Engine/Input/InputSystem.hpp"
#include "Game/App.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"

void BullfangoAnimController::Update( float deltaSeconds )
{
	if (!m_stateMachine)
		return;

	m_stateMachine->Update( deltaSeconds );

	if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
	{
		m_charaRef->m_controller->ResetAllState();
	}

	std::string currentStateName = m_stateMachine->GetOngoingAnimation( 0 ).GetCurrentState()->GetStateName();

	BullfangoController* bullfangoCtrl = dynamic_cast<BullfangoController*>(m_charaRef->m_controller);
	BullfangoCharacter* chara = dynamic_cast<BullfangoCharacter*>(m_charaRef);
	GameCharacter* targetRef = dynamic_cast<GameCharacter*>(chara->m_gameRef->GetActorByUID( chara->m_targetUID ));

	size_t pos = currentStateName.find( ' ' );
	std::string firstWord = (pos != std::string::npos) ? currentStateName.substr( 0, pos ) : currentStateName;

	if (firstWord != "Impact")
	{
		if (bullfangoCtrl->IsImpact())
		{
			if (bullfangoCtrl->IsDying())
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Dying Standing" );
				return;
			}
			else
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Impact Small" );
				return;
			}
		}
	}

	if (currentStateName == "Idle")
	{
		if (bullfangoCtrl->IsAttacking())
		{
			Vec3 targetLocation = targetRef->GetActorWorldPosition();
			Vec3 selfLocation = chara->GetActorWorldPosition();
			float distSquareToTarget = GetDistanceSquared2D( targetLocation.GetXY(), selfLocation.GetXY() );
			Vec3 selfForwardNormal = Vec3( chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY() );
			float dotP = DotProduct2D( selfForwardNormal.GetXY(), (targetLocation.GetXY() - selfLocation.GetXY()) );
			if (distSquareToTarget <= 4.f)
			{
				Vec3 sp = selfForwardNormal.GetRotatedAboutZDegrees( 45.f ).GetNormalized();
				float dotP2 = DotProduct2D( sp.GetXY(), (targetLocation.GetXY() - selfLocation.GetXY()) );
				if (dotP2 >= 0)
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Swiping A Front", 0.016f );
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Swiping A Left", 0.016f );
				}
			}
			else
			{
				if (dotP >= -0.7f)
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Begin Forward", 0.016f );
				}
				else
				{
					m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Begin Backward", 0.016f );
				}
			}
		}
		else if (bullfangoCtrl->m_isSleeping)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Lying Down", 0.016f );
		}
		else if (bullfangoCtrl->m_isFindingFood)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Smelling Up", 0.016f );
		}
		else if (bullfangoCtrl->m_isEating)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Digging and Eating", 0.016f );
		}
		else if (bullfangoCtrl->m_isMovingTo)
		{
			Vec3 targetLocation = bullfangoCtrl->m_moveToDest;
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY().GetNormalized();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();

			float dotP = DotProduct2D( selfForwardNormal, (targetLocation.GetXY() - selfLocation.GetXY()) );
			float crossP = CrossProduct2D( selfForwardNormal, (targetLocation.GetXY() - selfLocation.GetXY()) );
			if ((dotP >= -0.7f && crossP <= 0) || (dotP <= 0.7f && crossP > 0))
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking Begin Front", 0.016f );
			}
			else
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking Begin Back", 0.016f );
			}
		}
	}
	else if (currentStateName == "Impact Small" ||
		currentStateName == "Impact Mid" ||
		currentStateName == "Impact Big")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.2f );
		}
	}
	else if (currentStateName == "Swiping A Front" ||
		currentStateName == "Swiping A Left")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Swiping B", 0.016f );
		}
		else
		{
			Vec3 targetLocation = targetRef->GetActorWorldPosition();
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY().GetNormalized();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
			float crossP = CrossProduct2D( toTargetNormal, selfForwardNormal );
			float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
			float theta = ACosDegrees( Clamp( dotP, 0.f, 1.f ) );
			float adjustment = MIN( 90.f * deltaSeconds, theta );
			EulerAngles rotator = chara->GetActorWorldOrientation().GetNormalized().ToEulerAngles();
			if (crossP > 0)
			{
				rotator.m_yawDegrees -= adjustment;
			}
			else if (crossP < 0)
			{
				rotator.m_yawDegrees += adjustment;
			}
			chara->SetActorWorldOrientation( Quat( rotator ) );
		}
	}
	else if (currentStateName == "Swiping B")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.016f );
			if (targetRef->IsDead())
			{
				chara->m_targetUID = ActorUID::INVALID;
			}
		}
	}
	else if (currentStateName == "Charging Begin Forward" ||
		currentStateName == "Charging Begin Backward")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			Vec3 targetLocation = targetRef->GetActorWorldPosition();
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
			float crossP = CrossProduct2D( toTargetNormal, selfForwardNormal );
			float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
			if (dotP >= 0.5f)
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Readying", 0.016f );
			}
			else if (crossP > 0)
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Ready Right", 0.016f );
			}
			else if (crossP < 0)
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Ready Left", 0.016f );
			}
		}
	}
	else if (currentStateName == "Charging Ready Left" ||
		currentStateName == "Charging Ready Right")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Readying", 0.016f );
		}
	}
	else if (currentStateName == "Charging Readying")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentLoopCount() >= 2)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Launch", 0.016f );
		}
		else
		{
			Vec3 targetLocation = targetRef->GetActorWorldPosition();
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY().GetNormalized();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
			float crossP = CrossProduct2D( toTargetNormal, selfForwardNormal );
			float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
			float theta = ACosDegrees( Clamp( dotP, 0.f, 1.f ) );
			float adjustment = MIN( 90.f * deltaSeconds, theta );
			EulerAngles rotator = chara->GetActorWorldOrientation().GetNormalized().ToEulerAngles();
			if (crossP > 0)
			{
				rotator.m_yawDegrees -= adjustment;
			}
			else if (crossP < 0)
			{
				rotator.m_yawDegrees += adjustment;
			}
			chara->SetActorWorldOrientation( Quat( rotator ) );
		}
	}
	else if (currentStateName == "Charging Launch")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging", 0.016f );
		}
	}
	else if (currentStateName == "Charging")
	{
		if (targetRef->IsDead())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Stop", 0.016f );
		}
		Vec3 targetLocation = targetRef->GetActorWorldPosition();
		Vec3 selfLocation = chara->GetActorWorldPosition();
		Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
		Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
		float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
		if (dotP <= -0.5f)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Charging Stop", 0.016f );
		}
	}
	else if (currentStateName == "Charging Stop")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.016f );
			if (targetRef->IsDead())
			{
				chara->m_targetUID = ActorUID::INVALID;
			}
		}
	}
	else if (currentStateName == "Walking Begin Front" || currentStateName == "Walking Begin Back")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			Vec3 targetLocation = bullfangoCtrl->m_moveToDest;
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
			//float crossP = CrossProduct2D( toTargetNormal, selfForwardNormal );
			float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
			if (dotP >= 0.7f)
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking", 0.016f );
			}
			else
			{
				m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking Begin Left", 0.016f );
			}
		}
	}
	else if (currentStateName == "Walking Begin Left")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Walking", 0.016f );
		}
	}
	else if (currentStateName == "Walking")
	{
		if (!bullfangoCtrl->m_isMovingTo)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.016f );
		}
		else
		{
			Vec3 targetLocation = bullfangoCtrl->m_moveToDest;
			Vec3 selfLocation = chara->GetActorWorldPosition();
			Vec2 selfForwardNormal = chara->GetActorLocalOrientation().ToEulerAngles().GetForwardDir_XFwd_YLeft_Zup().GetXY().GetNormalized();
			Vec2 toTargetNormal = (targetLocation.GetXY() - selfLocation.GetXY()).GetNormalized();
			float crossP = CrossProduct2D( toTargetNormal, selfForwardNormal );
			float dotP = DotProduct2D( selfForwardNormal, toTargetNormal );
			float theta = ACosDegrees( Clamp( dotP, 0.f, 1.f ) );
			float adjustment = MIN( 90.f * deltaSeconds, theta );
			EulerAngles rotator = chara->GetActorWorldOrientation().GetNormalized().ToEulerAngles();
			if (crossP > 0)
			{
				rotator.m_yawDegrees -= adjustment;
			}
			else if (crossP < 0)
			{
				rotator.m_yawDegrees += adjustment;
			}
			chara->SetActorWorldOrientation( Quat( rotator ) );
		}
	}
	else if (currentStateName == "Lying Down")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationReadyForCrossfade( 2.5f ))
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Sleeping", 2.5f );
		}
	}
	else if (currentStateName == "Sleeping")
	{
		if (!bullfangoCtrl->m_isSleeping)
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Standing Up", 0.3f );
		}
	}
	else if (currentStateName == "Standing Up")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationReadyForCrossfade( 0.3f ))
		{
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.3f );
		}
	}
	else if (currentStateName == "Smelling Up")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			bullfangoCtrl->m_isFindingFood = false;
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.3f );
		}
	}
	else if (currentStateName == "Digging and Eating")
	{
		if (GetStateMachine()->GetOngoingAnimation( 0 ).HasCurrentAnimationEnded())
		{
			bullfangoCtrl->m_isEating = false;
			m_stateMachine->GetOngoingAnimation( 0 ).TransitTo( "Idle", 0.3f );
		}
	}
}
