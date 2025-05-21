#pragma once

#include "Game/GameCharacter.hpp"

class PlayerCharacter : public GameCharacter
{
	friend class PlayerController;
public:
	PlayerCharacter();
	PlayerCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game );
	PlayerCharacter( std::string name, std::vector<MeshT*>meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game );
	~PlayerCharacter();
	void Update( float deltaSeconds ) override;
	void Render() const override;

	virtual std::vector<CollisionInfo>const& GetAllCollisionInfo() override;
	virtual void InitializeCollisionComponents() override;

	virtual void ReceiveDamage( float value, bool blocked = false ) override;

	virtual void RecoverHP( float amount );
	//virtual void ActiveCurrentItem();

	float m_autoRecoverableHP = 0.f;

protected:
	void CameraArmCollisionCheck() override;
	void ComponentCollisionCheck() override;

protected:
	float m_notUsingStaminaTimer = 0.f;
	float m_notTakingDamageTimer = 0.f;
};