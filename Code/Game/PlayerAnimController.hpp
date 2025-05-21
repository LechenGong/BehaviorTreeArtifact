#pragma once

#include "Engine/Animation/AnimationController.hpp"

class PlayerAnimController : public AnimationController
{
public:
	PlayerAnimController();
	PlayerAnimController( Character* character );
	~PlayerAnimController();

	virtual void Update( float deltaSeconds );

	void TurnBeforeRoll();
};
