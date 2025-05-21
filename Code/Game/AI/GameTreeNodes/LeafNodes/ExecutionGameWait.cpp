#include "Game/AI/GameTreeNodes/LeafNodes/ExecutionGameWait.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"

#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"
#include "Game/GameCharacter.hpp"
#include "Game/Game.hpp"

ExecutionGameWait::ExecutionGameWait( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionGameWait::ParseDataFromXml( XmlElement* xmlElement )
{
	m_duration = ParseXmlAttribute( *xmlElement, "Duration", 0.f );
}

void ExecutionGameWait::ExportAttributeToXml( XmlElement* xmlElement )
{
	xmlElement->SetAttribute( "Duration", m_duration );
}

void ExecutionGameWait::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	btContext->chara->m_controller->GetTimeSpentOnNode() = 0.f;
}

NodeStatus ExecutionGameWait::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	if (btContext->chara->m_controller->GetTimeSpentOnNode() >= m_duration)
		return NodeStatus::SUCCESS;

	btContext->chara->m_controller->GetTimeSpentOnNode() += static_cast<GameCharacter*>(btContext->chara)->m_gameRef->GetDeltaSeconds();
	return m_status = NodeStatus::RUNNING;
}

std::string ExecutionGameWait::GetTypeName() const
{
	return "GameWait";
}
