#pragma once

#include "Engine/General/Controller.hpp"

class RathianController : public Controller
{
public:
	RathianController() = default;
	RathianController( Character* charaRef );
	~RathianController();


	virtual void InputResponse( float deltaSeconds );
	virtual void Update( float deltaSeconds );

	Character* m_possessedChara = nullptr;

// public:
// 	virtual bool IsSprinting() const = 0;
// 	virtual bool HasMovement() const = 0;
// 	virtual void ResetAllState() = 0;
// 	virtual bool IsMovementHindered() const = 0;
// 	virtual Vec3 GetMoveDirection() const = 0;
// 
// protected:
// 	bool m_isSprinting = false;
// 	Vec3 m_moveDirection = Vec3::ZERO;

};