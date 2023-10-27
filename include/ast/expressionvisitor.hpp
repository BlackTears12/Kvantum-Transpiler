#pragma once
#include "ast/AST_nodeoperation.hpp"
#include <any>
using std::any;

namespace kvantum
{
/*
	Classes witch derive from ExpressionVisitor can use this macro to 
	declare all expression visitor methods
*/
#define IMPLEMENETS_EXPRESSION_VISITOR \
	virtual any visit(Literal*); \
	virtual any visit(BinaryOperation*); \
	virtual any visit(Variable*); \
	virtual any visit(DynamicAllocation*); \
	virtual any visit(ArrayExpression*); \
	virtual any visit(ArrayIndex*); \
	virtual any visit(FunctionCall*); \
	virtual any visit(Cast*);

	class AST_Node;
	class Expression;
	class Literal;
	class BinaryOperation;
	class Variable;
	class DynamicAllocation;
	class ArrayExpression;
	class ArrayIndex;
	class FunctionCall;
	class Cast;

	class ExpressionVisitor : public virtual AST_NodeOperation
	{
	public:
		virtual any visit(Literal*) = 0;
		virtual any visit(BinaryOperation*) = 0;
		virtual any visit(Variable*) = 0;
		virtual any visit(DynamicAllocation*) = 0;
		virtual any visit(ArrayExpression*) = 0;
		virtual any visit(ArrayIndex*) = 0;
		virtual any visit(FunctionCall*) = 0;
		virtual any visit(Cast*) = 0;
	protected:
		virtual any visit_expression(Expression* expr);
	};
}