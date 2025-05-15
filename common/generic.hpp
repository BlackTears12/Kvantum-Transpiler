#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "common/compiler.hpp"
#include "common/generic.hpp"

namespace kvantum {
class GenericGenerator
{
public:
    FunctionNode *generateFunction(Variable *var, Type *t);
    TypeNode *generateType(std::string typen, Type *t);

private:
    GenericFunctionNode *getFunctionNode(Variable *var);
    GenericObjectNode *getObjectNode(std::string var);

    void visit(Literal *literal);
    void visit(BinaryOperation *bop);
    void visit(Variable *var);
    void visit(DynamicAllocation *all);
    void visit(ArrayExpression *arr);
    void visit(ArrayIndex *arr);
    void visit(Assigment *assig);
    void visit(If_Else *if_else);
    void visit(While *while_loop);
    void visit(For *for_loop);
    void visit(Return *ret);
    void visit(FunctionCall *fcall);
    void visit(StatementBlock *block);

    FunctionNode *thisfunc = nullptr;
    std::unordered_map<std::string, Type *> templatedTypes;
    Module *mod = nullptr; // Assuming 'mod' is a member variable representing the module
    std::unordered_map<std::string, GenericFunctionNode *>
        genericFunctionMap; // Assuming this map is used
    std::unordered_map<std::string, GenericObjectNode *> genericObjectMap; // Assuming this map is used

    void start_visit(AstNode *node)
    {
        node->accept(this);
    } // Helper function for the visitor pattern
};
} // namespace kvantum

#endif // GENERIC_HPP
