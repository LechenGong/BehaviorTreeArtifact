#include "Game/CharacterFactory.hpp"
#include "Game/PlayerCharacter.hpp"
#include "Game/AI/Bullfango/BullfangoCharacter.hpp"
#include "Game/AI/Rathian/RathianCharacter.hpp"

std::unordered_map<std::string, CharacterFactory::CharacterCreator> CharacterFactory::CharacterCreators;

void CharacterFactory::RegisterCharacterTypes()
{
	CharacterCreators["Paladin"] = []( std::string name, SkeletalMesh* skeletalMesh, GameRun* gameRef )
		{ return new PlayerCharacter( name, skeletalMesh, gameRef ); };

	CharacterCreators["Bullfango"] = []( std::string name, SkeletalMesh* skeletalMesh, GameRun* gameRef )
		{ return new BullfangoCharacter( name, skeletalMesh, gameRef ); };

	CharacterCreators["Rathian"] = []( std::string name, SkeletalMesh* skeletalMesh, GameRun* gameRef )
		{ return new RathianCharacter( name, skeletalMesh, gameRef ); };
}

GameCharacter* CharacterFactory::CreateCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* gameRef )
{
	auto iter = CharacterCreators.find( name );
	if (iter != CharacterCreators.end())
	{
		return iter->second( name, skeletalMesh, gameRef );
	}
	return nullptr;
}
