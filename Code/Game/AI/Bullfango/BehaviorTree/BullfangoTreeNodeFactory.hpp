#pragma once

#include "Engine/BehaviorTree/TreeNodes/TreeNodeFactory.h"

class BullfangoTreeNodeFactory : public TreeNodeFactory
{
public:
	static void RegisterNodeTypes();
};