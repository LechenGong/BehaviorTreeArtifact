#include "RathianController.hpp"
#include "Engine/Core/EngineCommon.hpp"

RathianController::RathianController( Character* charaRef )
	: Controller( charaRef )
{
}

RathianController::~RathianController()
{
}

void RathianController::InputResponse( float deltaSeconds )
{
	UNUSED( deltaSeconds );
}

void RathianController::Update( float deltaSeconds )
{
	InputResponse( deltaSeconds );
}
