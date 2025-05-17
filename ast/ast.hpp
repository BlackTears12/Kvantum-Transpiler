#pragma once

#include "ast/annotation.hpp"
#include "ast/expressionvisitor.hpp"
#include "ast/statementvisitor.hpp"
#include "common/token.hpp"
#include "common/type.hpp"
#include "common/util.hpp"
#include "lexer/lexer.hpp"
#include <memory>
#include <optional>
#include <string>

using std::make_unique;
using std::optional;
using std::string;
using std::unique_ptr;

/*
   defines operationVisit virtual function override
   its required by codebuilder methods
   MUST BE INSERTED INTO STATEMENT CLASS DEFINITIONS
*/
#define STATEMENT_NODE \
    any operationVisit(StatementVisitor *visitor) override \
    { \
        visitor->visit(this); \
        return KVANTUM_SKIP; \
    }
/*
   defines operationVisit virtual function override
   its required by codebuilder methods
   MUST BE INSERTED INTO EXPRESSION CLASS DEFINITIONS
*/
#define EXPRESSION_NODE \
    any operationVisit(ExpressionVisitor *visitor) override \
    { \
        return visitor->visit(this); \
    }

namespace kvantum {

struct FunctionNode;

class AST_Node
{
public:
    AST_Node() { lineIndex = Diagnostics::getLineIndex(); }
    virtual ~AST_Node() {}

    template<typename T>
    T as()
    {
        return static_cast<T>(this);
    }

    unsigned int lineIndex;
};

enum class ExprType {
    BINARY_OPERATION,
    LITERAL,
    VARIABLE,
    FUNCTION_CALL,
    DYNAMIC_ALLOCATION,
    ARRAY_EXPR,
    ARRAY_INDEX,
    TAKE_REFERENCE,
    CAST
};

class Expression : public AST_Node
{
public:
    Expression(ExprType t) { exprtype = t; }
    virtual ~Expression() {}
    virtual any operationVisit(ExpressionVisitor *visitor) = 0;

    virtual Expression *copy() = 0;
    ExprType exprtype;
    virtual Type &getType() = 0;
};

class BinaryOperation : public Expression
{
public:
    EXPRESSION_NODE
    enum Operator {
        ADD,
        SUBTRACT,
        MULTIPLY,
        DIVIDE,
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_OR_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
        AND,
        OR
    };

    BinaryOperation(Expression *lhs, Expression *rhs, Operator op, bool parenthesized = false)
        : Expression(ExprType::BINARY_OPERATION)
    {
        this->op = op;
        this->lhs = lhs;
        this->rhs = rhs;
        this->parenthesised = parenthesized;
    }

    ~BinaryOperation()
    {
        delete lhs;
        delete rhs;
    }

    Type &getType() override
    {
        if (isBool())
            return PrimitiveType::get(PrimitiveType::Boolean);
        if (lhs != nullptr)
            return rhs->getType();
        return rhs->getType();
    }

    BinaryOperation *copy() override { return new BinaryOperation(lhs->copy(), rhs->copy(), op); }
    constexpr bool isBool() { return op >= EQUAL; }
    constexpr bool isParenthesized() const { return parenthesised; }

    Expression *lhs;
    Expression *rhs;
    Operator op;
    bool parenthesised;
};

class Literal : public Expression
{
public:
    EXPRESSION_NODE
    Literal(string v, Type &t)
        : Expression(ExprType::LITERAL)
        , type(t)
    {
        value = v;
    }

    Type &getType() override { return type; }

    Literal *copy() override { return new Literal(value, type); }

    string value;
    Type &type;
};

class FieldAccess;
class Variable : public Expression
{
public:
    EXPRESSION_NODE

    Variable(string i, Type &t = Type::get("Void"))
        : Expression(ExprType::VARIABLE)
        , type(&t)
    {
        id = i;
    }

    Type &getType() override { return *type; }

    virtual Variable *end() { return this; }
    virtual void setType(Type &t) { type = &t; }

    virtual bool isField() { return false; }
    FieldAccess *asField();
    Variable *copy() override { return new Variable(id, *type); }

    string id;
    Type *type;
};

class FieldAccess : public Variable
{
public:
    FieldAccess(Expression *b, Variable *f)
        : Variable(*f)
    {
        base = b;
        field = f;
    }

    ~FieldAccess()
    {
        delete base;
        delete field;
    }

    Type &getType() override { return field->getType(); }
    bool isField() override { return true; }
    Variable *end() override { return field->end(); }

    FieldAccess *copy() override { return new FieldAccess(base->copy(), field->copy()); }
    Expression *base;
    Variable *field;
};

class DynamicAllocation : public Expression
{
public:
    EXPRESSION_NODE
    DynamicAllocation(Type &n)
        : Expression(ExprType::DYNAMIC_ALLOCATION)
        , node(n)
    {
        sizeExpr = new Literal(std::to_string(node.getAllocSize()), Type::get("Int"));
    }

    Type &getType() override { return ReferenceType::get(node); }

    DynamicAllocation *copy() override { return new DynamicAllocation(node); }
    virtual Expression *getSizeExpr() { return sizeExpr; }

    Expression *sizeExpr;
    Type &node;
};

class ArrayExpression : public Expression
{
public:
    EXPRESSION_NODE
    ArrayExpression(Type &t, vector<Literal *> init)
        : Expression(ExprType::ARRAY_EXPR)
        , initializer(init)
        , type(ArrayType::get(t))
    {}

    Type &getType() override { return type; }
    ArrayExpression *copy() override
    {
        return new ArrayExpression(type,
                                   apply(initializer.begin(),
                                         initializer.end(),
                                         std::function([](Literal *l) { return l->copy(); })));
    }

    vector<Literal *> initializer;
    ArrayType &type;
};

class ArrayAllocation : public DynamicAllocation
{
public:
    ArrayAllocation(Type &itemT, Variable *sizeV)
        : DynamicAllocation(itemT)
        , itemType(itemT)
    {
        itemType = itemT;
        sizeVar = sizeV;
        sizeExpr = new BinaryOperation(sizeExpr, sizeVar, BinaryOperation::MULTIPLY);
    }

    Type &getType() override { return node; }

    ArrayAllocation *copy() override { return new ArrayAllocation(itemType, sizeVar->copy()); }
    Expression *getSizeExpr() override { return sizeExpr; }

    Type &itemType;
    Variable *sizeVar;
};

class ArrayIndex : public Expression
{
public:
    EXPRESSION_NODE
    ArrayIndex(Expression *arr, Expression *ind)
        : Expression(ExprType::ARRAY_INDEX)
    {
        baseArray = arr;
        index = ind;
    }

    Type &getType() override { return baseArray->getType().asArray().getType(); }
    ArrayIndex *copy() override { return new ArrayIndex(baseArray->copy(), index->copy()); }

    Expression *baseArray;
    Expression *index;
};

class TakeReference : public Expression
{
public:
    EXPRESSION_NODE
    TakeReference(Expression *expr)
        : Expression(ExprType::TAKE_REFERENCE)
        , baseExpr(expr)
    {}
    Type &getType() override { return ReferenceType::get(baseExpr->getType()); }
    TakeReference *copy() override { return new TakeReference(baseExpr->copy()); }

    Expression *baseExpr;
};

class Cast : public Expression
{
public:
    EXPRESSION_NODE
    Cast(Expression *expr, Type &to)
        : Expression(ExprType::CAST)
        , castTo(to)
    {
        this->expr = expr;
    }
    Type &getType() override { return castTo; }
    Cast *copy() override { return new Cast(expr->copy(), castTo); }

    Expression *expr;
    Type &castTo;
};

enum class StatementType { ASSIGMENT, RETURN, FUNCTION_CALL, IF_ELSE, WHILE, FOR, BLOCK };

class Statement : public AST_Node
{
public:
    Statement(StatementType t) { sttype = t; }
    virtual ~Statement() {}
    virtual any operationVisit(StatementVisitor *visitor) = 0;
    virtual Statement *copy() = 0;

    StatementType sttype;
};

class StatementBlock : public Statement
{
public:
    STATEMENT_NODE
    StatementBlock()
        : Statement(StatementType::BLOCK)
    {}
    ~StatementBlock()
    {
        for (auto &e : block)
            delete e;
    }
    StatementBlock *copy() override
    {
        StatementBlock *b = new StatementBlock();
        b->block = apply(block.begin(), block.end(), std::function([](Statement *s) {
                             return s->copy();
                         }));
        return b;
    }

    vector<Statement *> block;
};

class Assigment : public Statement
{
public:
    STATEMENT_NODE
    Assigment(Variable *var, Expression *expr, bool decl = false)
        : Statement(StatementType::ASSIGMENT)
    {
        variable = var;
        this->expr = expr;
        declaration = decl;
    }

    ~Assigment() override
    {
        delete variable;
        delete expr;
    }
    Assigment *copy() override
    {
        return new Assigment(variable->copy(), expr->copy(), declaration);
    }
    bool isDeclaration() const { return declaration; }
    Assigment *setDeclaration(bool decl)
    {
        this->declaration = decl;
        return this;
    }

    Variable *variable;
    Expression *expr;

private:
    bool declaration;
};

class Return : public Statement
{
public:
    STATEMENT_NODE
    Return(Expression *e)
        : Statement(StatementType::RETURN)
    {
        expr = e;
    }
    ~Return() override { delete expr; }
    Return *copy() override { return new Return(expr->copy()); }

    Expression *expr;
};

class If_Else : public Statement
{
public:
    STATEMENT_NODE
    If_Else(Expression *cond, Statement *ifb = nullptr, Statement *elseb = nullptr)
        : Statement(StatementType::IF_ELSE)
    {
        condition = cond;
        ifBlock = ifb;
        elseBlock = elseb;
    }

    ~If_Else() override
    {
        delete condition;
        delete ifBlock;
        delete elseBlock;
    }
    If_Else *copy() override
    {
        return new If_Else(condition->copy(), ifBlock->copy(), elseBlock->copy());
    }

    Expression *condition;
    Statement *ifBlock;
    Statement *elseBlock;
};

class While : public Statement
{
public:
    STATEMENT_NODE
    explicit While(Expression *cond, Statement *b = nullptr)
        : Statement(StatementType::WHILE)
    {
        condition = cond;
        block = b;
    }
    ~While() override
    {
        delete condition;
        delete block;
    }
    While *copy() override { return new While(condition->copy(), block->copy()); }

    Expression *condition;
    Statement *block;
};

class For : public Statement
{
public:
    STATEMENT_NODE

    explicit For(Statement *b = nullptr)
        : Statement(StatementType::FOR)
    {
        block = b;
    }

    Statement *block;
};

class FunctionCall : public Expression, public Statement
{
public:
    STATEMENT_NODE
    EXPRESSION_NODE
    FunctionCall(Variable *name, vector<Expression *> arguments = {}, FunctionNode *node = nullptr)
        : Expression(ExprType::FUNCTION_CALL)
        , Statement(StatementType::FUNCTION_CALL)
    {
        var = name;
        this->arguments = arguments;
        fnode = node;
    }

    ~FunctionCall() override
    {
        delete var;
        for (auto &e : arguments)
            delete e;
    }

    void setNode(FunctionNode *node) { fnode = node; }
    Type &getType() override;
    FunctionCall *copy() override
    {
        return new FunctionCall(var->copy(),
                                apply(arguments.begin(),
                                      arguments.end(),
                                      std::function([](Expression *e) { return e->copy(); })));
    }
    FunctionNode *fnode;
    Variable *var;
    vector<Expression *> arguments;
};
} // namespace kvantum
