#pragma once

#include "Engine/Animation/AnimationController.hpp"

class RathianAnimController : public AnimationController
{
public:
	RathianAnimController();
	RathianAnimController( Character* character );
	~RathianAnimController();

	virtual void Update( float deltaSeconds ) override;

};