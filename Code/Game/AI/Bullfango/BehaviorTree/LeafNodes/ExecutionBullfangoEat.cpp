#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoEat.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"
#include "Engine/General/Character.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"

ExecutionBullfangoEat::ExecutionBullfangoEat( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionBullfangoEat::ParseDataFromXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void ExecutionBullfangoEat::ExportAttributeToXml( [[maybe_unused]] XmlElement* xmlElement )
{
}

void ExecutionBullfangoEat::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
}

NodeStatus ExecutionBullfangoEat::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isEating = true;
	return m_status = NodeStatus::SUCCESS;
}

std::string ExecutionBullfangoEat::GetTypeName() const
{
	return "BullfangoEat";
}
