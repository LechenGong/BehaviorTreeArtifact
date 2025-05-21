#include "Game/AI/Rathian/RathianAnimController.hpp"
#include "Engine/Animation/AnimationStateMachine.hpp"
#include "Engine/Animation/AnimationState.hpp"

#include "Game/Game.hpp"

RathianAnimController::RathianAnimController()
	: AnimationController()
{
}

RathianAnimController::RathianAnimController( Character* character )
	: AnimationController( character )
{
	GetStateMachine()->InitializeState( "Rathian", GameRun::g_animStates );
}

RathianAnimController::~RathianAnimController()
{
}

void RathianAnimController::Update( float deltaSeconds )
{
	if (!m_stateMachine)
		return;

	m_stateMachine->Update( deltaSeconds );

	//std::string currentStateName = m_stateMachine->GetCurrentState()->GetStateName();
}
