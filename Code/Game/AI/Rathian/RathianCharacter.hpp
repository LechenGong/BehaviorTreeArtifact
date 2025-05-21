#pragma once

#include "Game/GameCharacter.hpp"

class RathianCharacter : public GameCharacter
{
public:
	RathianCharacter();
	RathianCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* game );
	RathianCharacter( std::string name, std::vector<MeshT*>meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures, GameRun* game );
	~RathianCharacter();
	void Update( float deltaSeconds ) override;
	void Render() const override;

	virtual std::vector<CollisionInfo>const& GetAllCollisionInfo() override;
	virtual void InitializeCollisionComponents() override;

protected:
};
