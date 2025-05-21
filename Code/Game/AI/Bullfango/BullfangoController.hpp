#pragma once

#include "Engine/General/Controller.hpp"

class BullfangoController : public Controller
{
public:
	BullfangoController() = default;
	BullfangoController( Character* charaRef );
	~BullfangoController();


	virtual void InputResponse( float deltaSeconds );
	virtual void Update( float deltaSeconds );

public:
	bool IsSprinting() const override;
	bool IsImpact() const override;
	bool IsTurning() const override;
	bool IsFalling() const override;
	bool IsDying() const override;
	bool IsAttemptingToMove() const;
	bool IsAttacking() const;
	void ResetAllState();
	bool IsMovementHindered() const;
	Vec3 GetMoveDirection() const;

	void SetIsFalling( bool flag ) override { m_isFalling = flag; }

public:
	bool m_isMovingTo = false;
	bool m_isSleeping = false;
	bool m_isFindingFood = false;
	bool m_isEating = false;
	Vec3 m_moveToDest;

protected:
	bool m_isAttacking = false;

private:
	float m_healthPreviousFrame = 0.f;
};