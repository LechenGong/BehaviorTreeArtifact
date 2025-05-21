#include <algorithm>
#include <random>

#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoFindFood.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/General/Character.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"


ExecutionBullfangoFindFood::ExecutionBullfangoFindFood( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionBullfangoFindFood::ParseDataFromXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void ExecutionBullfangoFindFood::ExportAttributeToXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void ExecutionBullfangoFindFood::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
}

NodeStatus ExecutionBullfangoFindFood::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isFindingFood = true;
	std::vector<Vec3> food( 3 );
	food[0] = Vec3( 13.f, 4.6f, 0.86f );
	food[1] = Vec3( 32.67f, -14.65f, 0.f );
	food[2] = Vec3( 39.21f, 17.07f, 0.04f );
	static std::random_device rd;
	static std::mt19937 rng( rd() );
	std::shuffle( food.begin(), food.end(), rng );
	std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
		+ std::to_string( btContext->chara->GetUID().GetIndex() )
		+ "MoveToDestination";
	btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, food[0] );
	return m_status = NodeStatus::SUCCESS;
}

std::string ExecutionBullfangoFindFood::GetTypeName() const
{
	return "BullfangoFindFood";
}
