#pragma once

#include "Engine/BehaviorTree/TreeNodes/LeafNodes/LeafNode.h"

class ExecutionBullfangoFindFood : public LeafNode
{
public:
	ExecutionBullfangoFindFood( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 );
	void ParseDataFromXml( XmlElement* xmlElement ) override;
	void ExportAttributeToXml( XmlElement* xmlElement ) override;

	void InternalSpawn( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	NodeStatus InternalTick( [[maybe_unused]] const BehaviorTree::Context* btContext ) override;
	void InternalTerminate( [[maybe_unused]] const BehaviorTree::Context* btContext ) override {};

	std::string GetTypeName() const override;

protected:
	Vec3 m_dest = Vec3::ZERO;
};