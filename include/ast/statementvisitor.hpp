#pragma once
#include "ast/AST_nodeoperation.hpp"
#include <any>

using std::any;
namespace kvantum
{
/*
	Clases with derive from StatementVisitor can use this macro to
	declare all the statement visitor methods in an ExpressionVisitor compatible way
*/
#define IMPLEMENTS_STATEMENT_VISITOR\
	virtual void visit(StatementBlock*);\
	virtual void visit(Assigment*);\
	virtual void visit(If_Else*);\
	virtual void visit(While*);\
	virtual void visit(For*);\
	virtual void visit(Return*);
/*
	Declares all the statement visitor methods which will be uncompatible with ExpressionVisitor
*/
#define IMPLEMENTS_EXTENDED_STATEMENT_VISITOR\
	IMPLEMENTS_STATEMENT_VISITOR\
	virtual any visit(FunctionCall*);

	class AST_Node;
	class Statement;
	class StatementBlock;
	class FunctionCall;
	class Assigment;
	class If_Else;
	class While;
	class For;
	class Return;

	/*
		class which provides an visitor interface to all STATEMENT_NODEs,
		using the double-dispatch design pattern
	*/
	class StatementVisitor : public virtual AST_NodeOperation
	{
	public:
		virtual void visit(StatementBlock*) = 0;
		virtual void visit(Assigment*) = 0;
		virtual void visit(If_Else*) = 0;
		virtual void visit(While*) = 0;
		virtual void visit(For*) = 0;
		virtual void visit(Return*) = 0;
		virtual any visit(FunctionCall*) = 0;
	protected:
		virtual void visit_statement(Statement* st);
		void insertBeforeThis(Statement* item);
	private:
		StatementBlock* currentBlock = nullptr;
	};
}