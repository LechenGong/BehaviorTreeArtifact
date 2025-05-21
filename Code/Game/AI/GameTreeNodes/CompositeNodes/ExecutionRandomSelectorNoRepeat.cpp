#include <algorithm>
#include <random>

#include "Game/AI/GameTreeNodes/CompositeNodes/ExecutionRandomSelectorNoRepeat.h"
#include "Engine/General/Character.hpp"
#include "Engine/General/Controller.hpp"

ExecutionRandomSelectorNoRepeat::ExecutionRandomSelectorNoRepeat( BehaviorTree* bt, int selfIndex, int parentIndex, int location /*= -1 */ )
	: CompositeNode( bt, selfIndex, parentIndex, location )
{
}

void ExecutionRandomSelectorNoRepeat::InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
		+ std::to_string( btContext->chara->GetUID().GetIndex() )
		+ std::to_string( m_index )
		+ std::to_string( (m_parent) ? m_parent->GetIndex() : -1 )
		+ "PrevSucceededChildIndex";

	int prevSucceededChildIndex = btContext->chara->m_controller->m_blackboard.SafeGetValue<int>( blackboardKey );
	TreeNode* temp = nullptr;

	if (prevSucceededChildIndex != -1)
	{
		temp = m_children[prevSucceededChildIndex];
	}

	static std::random_device rd;
	static std::mt19937 rng( rd() );
	std::shuffle( m_children.begin(), m_children.end(), rng );

	prevSucceededChildIndex = -1;

	for (int i = 0; i < m_children.size(); i++)
	{
		if (temp == m_children[i])
		{
			prevSucceededChildIndex = i;
			break;
		}
	}
	btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, prevSucceededChildIndex );
}

NodeStatus ExecutionRandomSelectorNoRepeat::InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext )
{
	int runningChildIndex = 0;
	for (int i = 0; i < m_children.size(); i++)
	{
		if (btContext->chara->m_controller->GetNodeStates()[m_children[i]->GetIndex()] == NodeStatus::RUNNING)
		{
			runningChildIndex = i;
			break;
		}
	}

	std::string blackboardKey = std::to_string( btContext->chara->GetUID().GetSalt() )
		+ std::to_string( btContext->chara->GetUID().GetIndex() )
		+ std::to_string( m_index )
		+ std::to_string( (m_parent) ? m_parent->GetIndex() : -1 )
		+ "PrevSucceededChildIndex";

	int prevSucceededChildIndex = btContext->chara->m_controller->m_blackboard.GetValue<int>( blackboardKey );

	for (int i = runningChildIndex; i < m_children.size(); i++)
	{
		if (i == prevSucceededChildIndex)
			continue;

		TreeNode* child = m_children[i];
		NodeStatus status = child->Execute( btContext );

		if (status == NodeStatus::RUNNING)
		{
			btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, -1 );
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::RUNNING;
		}
		else if (status == NodeStatus::SUCCESS)
		{
			btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, i );
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::SUCCESS;
		}
		else if (status == NodeStatus::BREAKING)
		{
			btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, -1 );
			return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = (btContext->chara->m_controller->GetNodeStates()[GetIndex()] == NodeStatus::INVALID) ? NodeStatus::BREAKING : btContext->chara->m_controller->GetNodeStates()[GetIndex()];
		}
	}
	btContext->chara->m_controller->m_blackboard.SetValue( blackboardKey, -1 );
	return btContext->chara->m_controller->GetNodeStates()[GetIndex()] = NodeStatus::FAILURE;
}

std::string ExecutionRandomSelectorNoRepeat::GetTypeName() const
{
	return "RandomSelector";
}

