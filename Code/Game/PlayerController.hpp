#pragma once

#include "Engine/Math/Vec3.hpp"
#include "Engine/General/Controller.hpp"

class PlayerController : public Controller
{
	friend class PlayerAnimController;
public:
	PlayerController();
	PlayerController( Character* m_charaRef );
	~PlayerController();

	void InputResponse( float deltaSeconds ) override;
	void Update( float deltaSeconds ) override;

public:
	bool IsSprinting() const override;
	bool IsImpact() const override;
	bool IsTurning() const override;
	bool IsFalling() const override;
	bool IsDying() const override;
	bool IsAttemptingToMove() const override;
	bool IsAttacking() const;
	bool IsRolling() const;
	bool IsBlocking() const;
	bool IsUnblocking() const;
	bool IsMovementHindered() const override;
	bool IsUsingItem() const;
	virtual void ResetAllState() override;
	void ResetUsingItem();
	Vec3 GetMoveDirection() const override;

	void SetIsFalling( bool flag ) override { m_isFalling = flag; }

	bool m_blockedSucceeded = true;

	bool m_movementDisabled = false;
	bool m_turningDisabled = false;

protected:
	float m_isSprinting = 1.f;
	bool m_isAttacking = false;
	bool m_isRolling = false;
	bool m_isBlocking = false;
	bool m_isUnblocking = false;
	bool m_isUsingItem = false;
	bool m_isFalling = false;
	Vec3 m_moveDirection = Vec3::ZERO;

	bool m_unableToDash = false;

private:
	float m_healthPreviousFrame = 0.f;
};
