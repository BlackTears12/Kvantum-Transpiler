#pragma once
#include "ast/expressionvisitor.hpp"
#include "ast/statementvisitor.hpp"
#include "ast/ast.hpp"

namespace kvantum
{ 
/*
    Classes wich derive from TreeVisitor can use this macro to
    declare all expression and statement visitor methods
*/
#define IMPLEMENTS_TREE_VISITOR IMPLEMENETS_EXPRESSION_VISITOR IMPLEMENTS_STATEMENT_VISITOR
   /*
        class that can visit the entire AST
   */
   class TreeVisitor : public StatementVisitor,public ExpressionVisitor{
   public:
       TreeVisitor() { current = nullptr; }
   };
}