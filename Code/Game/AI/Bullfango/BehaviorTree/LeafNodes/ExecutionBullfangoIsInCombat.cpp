#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoStay.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"

ExecutionBullfangoStay::ExecutionBullfangoStay( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionBullfangoStay::ParseDataFromXml( XmlElement* xmlElement )
{
}

void ExecutionBullfangoStay::ExportAttributeToXml( XmlElement* xmlElement )
{
}

void ExecutionBullfangoStay::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
}

NodeStatus ExecutionBullfangoStay::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	return m_status = NodeStatus::SUCCESS;
}

std::string ExecutionBullfangoStay::GetTypeName() const
{
	return "BullfangoStay";
}
