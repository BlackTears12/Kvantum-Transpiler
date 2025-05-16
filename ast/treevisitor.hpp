#pragma once
#include "ast/ast.hpp"
#include "ast/expressionvisitor.hpp"
#include "ast/statementvisitor.hpp"

namespace kvantum {

/*
    Classes which derive from TreeVisitor can use this macro to
    declare all expression and statement visitor methods
*/
#define IMPLEMENTS_TREE_VISITOR IMPLEMENTS_EXPRESSION_VISITOR IMPLEMENTS_STATEMENT_VISITOR

/*
    Class that can visit the entire AST
*/
class TreeVisitor : public StatementVisitor, public ExpressionVisitor
{
public:
    TreeVisitor() { current = nullptr; }
};

} // namespace kvantum
