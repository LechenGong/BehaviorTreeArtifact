#pragma once

#include "Engine/BehaviorTree/TreeNodes/TreeNodeFactory.h"

class GameTreeNodeFactory : public TreeNodeFactory
{
public:
	static void RegisterNodeTypes();
};