#include "Game/AI/Bullfango/BehaviorTree/BullfangoTreeNodeFactory.hpp"

#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoMoveTo.hpp"
#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoStay.hpp"
#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoSleep.hpp"
#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoFindFood.hpp"
#include "Game/AI/Bullfango/BehaviorTree/LeafNodes/ExecutionBullfangoEat.hpp"

void BullfangoTreeNodeFactory::RegisterNodeTypes()
{
	NodeCreators["BullfangoMoveTo"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionBullfangoMoveTo( bt, selfIndex, parentIndex, location ); };

	NodeCreators["BullfangoStay"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionBullfangoStay( bt, selfIndex, parentIndex, location ); };

	NodeCreators["BullfangoSleep"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionBullfangoSleep( bt, selfIndex, parentIndex, location ); };

	NodeCreators["BullfangoFindFood"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionBullfangoFindFood( bt, selfIndex, parentIndex, location ); };

	NodeCreators["BullfangoEat"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionBullfangoEat( bt, selfIndex, parentIndex, location ); };
}
