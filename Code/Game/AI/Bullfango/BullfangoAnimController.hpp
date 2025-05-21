#pragma once

#include "Engine/Animation/AnimationController.hpp"

class BullfangoAnimController : public AnimationController
{
public:
	BullfangoAnimController();
	BullfangoAnimController( Character* character );
	~BullfangoAnimController();

	virtual void Update( float deltaSeconds ) override;

};