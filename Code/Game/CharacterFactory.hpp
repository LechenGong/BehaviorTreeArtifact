#pragma once

#include <unordered_map>
#include <functional>
#include <string>

class GameCharacter;
class SkeletalMesh;
class GameRun;

class CharacterFactory
{
public:
	using CharacterCreator = std::function<GameCharacter* (std::string, SkeletalMesh*, GameRun*)>;

	static std::unordered_map<std::string, CharacterCreator> CharacterCreators;

	static void RegisterCharacterTypes();

	static GameCharacter* CreateCharacter( std::string name, SkeletalMesh* skeletalMesh, GameRun* gameRef );
};
