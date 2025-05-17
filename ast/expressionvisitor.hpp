#pragma once
#include "ast/ast_nodeoperation.hpp"
#include <any>
using std::any;

namespace kvantum {
/*
        Classes witch derive from ExpressionVisitor can use this macro to
        declare all expression visitor methods
*/
#define IMPLEMENTS_EXPRESSION_VISITOR \
    virtual any visit(Literal *) override; \
    virtual any visit(BinaryOperation *) override; \
    virtual any visit(Variable *) override; \
    virtual any visit(DynamicAllocation *) override; \
    virtual any visit(ArrayExpression *) override; \
    virtual any visit(ArrayIndex *) override; \
    virtual any visit(FunctionCall *) override; \
    virtual any visit(TakeReference *) override; \
    virtual any visit(Cast *) override;

class AST_Node;
class Expression;
class Literal;
class BinaryOperation;
class Variable;
class DynamicAllocation;
class ArrayExpression;
class ArrayIndex;
class FunctionCall;
class TakeReference;
class Cast;

class ExpressionVisitor : public virtual AST_NodeOperation
{
public:
    virtual any visit(Literal *) = 0;
    virtual any visit(BinaryOperation *) = 0;
    virtual any visit(Variable *) = 0;
    virtual any visit(DynamicAllocation *) = 0;
    virtual any visit(ArrayExpression *) = 0;
    virtual any visit(ArrayIndex *) = 0;
    virtual any visit(FunctionCall *) = 0;
    virtual any visit(TakeReference *) = 0;
    virtual any visit(Cast *) = 0;

protected:
    virtual any visit_expression(Expression *expr);
};
} // namespace kvantum
