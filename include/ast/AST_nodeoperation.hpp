#pragma once
#include <any>
using std::any;
using std::any_cast;

namespace kvantum
{
	class AST_Node;

	/*
		basic class for visitors to inherit from
	*/
	class AST_NodeOperation
	{
	protected:
		AST_Node* current = nullptr;
		any generatedValue;
		enum NodeState{isStatement,isExpression} state = isStatement;
	};
}
