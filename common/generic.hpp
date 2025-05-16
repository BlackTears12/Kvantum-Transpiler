/*#ifndef GENERIC_HPP
#define GENERIC_HPP

#include "ast/treevisitor.hpp"

#include "module.hpp"

namespace kvantum {

class GenericGenerator : public TreeVisitor
{
public:
    IMPLEMENTS_TREE_VISITOR

    FunctionNode *generateFunction(Variable *var, Type *t);
    TypeNode *generateType(std::string typen, Type *t);

private:
    GenericFunctionNode *getFunctionNode(Variable *var);
    GenericObjectNode *getObjectNode(std::string var);

    FunctionNode *thisfunc = nullptr;
    std::unordered_map<std::string, Type *> templatedTypes;
    Module *mod = nullptr; // Assuming 'mod' is a member variable representing the module
    std::unordered_map<std::string, GenericFunctionNode *>
        genericFunctionMap; // Assuming this map is used
    std::unordered_map<std::string, GenericObjectNode *> genericObjectMap; // Assuming this map is used
};

} // namespace kvantum

#endif // GENERIC_HPP
*/
