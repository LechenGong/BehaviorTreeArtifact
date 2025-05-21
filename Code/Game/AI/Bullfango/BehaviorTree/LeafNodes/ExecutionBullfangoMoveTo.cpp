#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoMoveTo.hpp"
#include "Engine/BehaviorTree/BehaviorTree.h"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/General/Character.hpp"
#include "Game/AI/Bullfango/BullfangoController.hpp"

ExecutionBullfangoMoveTo::ExecutionBullfangoMoveTo( BehaviorTree* bt, int selfIndex, int parentIndex, int location )
	: LeafNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionBullfangoMoveTo::ParseDataFromXml( XmlElement* xmlElement )
{
	m_dest = ParseXmlAttribute( *xmlElement, "Destination", Vec3{ -99999.f, -99999.f, -99999.f } );
	m_detectRange = ParseXmlAttribute( *xmlElement, "DetectRange", 0.625f );
}

void ExecutionBullfangoMoveTo::ExportAttributeToXml( [[maybe_unused]] XmlElement* xmlElement )
{
	//xmlElement->SetAttribute( "Destination", m_dest );
}

void ExecutionBullfangoMoveTo::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	if (m_dest != Vec3{ -99999.f, -99999.f, -99999.f })
	{
		std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
			+ std::to_string( btContext->chara->GetUID().GetIndex() )
			+ "MoveToDestination";

		btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, m_dest );
	}
}

NodeStatus ExecutionBullfangoMoveTo::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
		+ std::to_string( btContext->chara->GetUID().GetIndex() )
		+ "MoveToDestination";

	Vec3 actorLoc = btContext->chara->GetActorWorldPosition();

	Vec3 dest = btContext->chara->m_controller->m_blackboard.GetValueWithDefault<Vec3>( blackboardKey, Vec3::ZERO );

	float currentDist = GetDistance2D( dest.GetXY(), actorLoc.GetXY() );
	if (currentDist <= m_detectRange * m_detectRange)
	{
		static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isMovingTo = false;
		return m_status = NodeStatus::SUCCESS;
	}
	static_cast<BullfangoController*>(btContext->chara->m_controller)->m_isMovingTo = true;
	static_cast<BullfangoController*>(btContext->chara->m_controller)->m_moveToDest = dest;
	return m_status = NodeStatus::RUNNING;
}

void ExecutionBullfangoMoveTo::InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
		+ std::to_string( btContext->chara->GetUID().GetIndex() )
		+ std::to_string( m_index )
		+ std::to_string( (m_parent) ? m_parent->GetIndex() : -1 )
		+ "MoveToDestination";

	btContext->chara->m_controller->m_blackboard.RemoveValue( blackboardKey );
}

std::string ExecutionBullfangoMoveTo::GetTypeName() const
{
	return "BullfangoMoveTo";
}
