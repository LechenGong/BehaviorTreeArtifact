#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoSleep.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"

#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"
#include "Game/GameCharacter.hpp"
#include "Game/Game.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"

ExecutionBullfangoSleep::ExecutionBullfangoSleep( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionBullfangoSleep::ParseDataFromXml( XmlElement* xmlElement )
{
	m_duration = ParseXmlAttribute( *xmlElement, "Duration", 0.f );
}

void ExecutionBullfangoSleep::ExportAttributeToXml( XmlElement* xmlElement )
{
	xmlElement->SetAttribute( "Duration", m_duration );
}

void ExecutionBullfangoSleep::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	btContext->chara->m_controller->GetTimeSpentOnNode() = 0.f;
}

NodeStatus ExecutionBullfangoSleep::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	if (btContext->chara->m_controller->GetTimeSpentOnNode() >= m_duration)
	{
		static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isSleeping = false;
		return NodeStatus::SUCCESS;
	}

	btContext->chara->m_controller->GetTimeSpentOnNode() += static_cast<GameCharacter*>(btContext->chara)->m_gameRef->GetDeltaSeconds();
	static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isSleeping = true;
	return m_status = NodeStatus::RUNNING;
}

std::string ExecutionBullfangoSleep::GetTypeName() const
{
	return "BullfangoSleep";
}
