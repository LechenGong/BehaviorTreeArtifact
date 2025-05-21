#include "Game/AI/GameTreeNodeFactory.hpp"

#include "Game/AI/GameTreeNodes/LeafNodes/ExecutionGameWait.hpp"
#include "Game/AI/GameTreeNodes/CompositeNodes/ExecutionRandomSelectorNoRepeat.h"

void GameTreeNodeFactory::RegisterNodeTypes()
{
	NodeCreators["GameWait"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionGameWait( bt, selfIndex, parentIndex, location ); };

	NodeCreators["RandomSelectorNoRepeat"] = []( BehaviorTree* bt, int selfIndex, int parentIndex, int location = -1 )
		{ return new ExecutionRandomSelectorNoRepeat( bt, selfIndex, parentIndex, location ); };
}
