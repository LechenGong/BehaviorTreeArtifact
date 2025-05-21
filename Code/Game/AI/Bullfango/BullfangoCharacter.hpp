#pragma once

#include "Game/GameCharacter.hpp"

class BullfangoCharacter : public GameCharacter
{
public:
	BullfangoCharacter();
	BullfangoCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game );
	BullfangoCharacter( std::string name, std::vector<MeshT*>meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game );
	~BullfangoCharacter();
	void Update( float deltaSeconds ) override;
	void Render() const override;

	virtual std::vector<CollisionInfo>const& GetAllCollisionInfo() override;
	virtual void InitializeCollisionComponents() override;

protected:
	void ComponentCollisionCheck() override;

private:
	bool TestSuicide();
};
