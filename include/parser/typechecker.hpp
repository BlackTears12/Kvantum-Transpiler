#pragma once
#include <set>
#include <stack>
#include "ast/ast.hpp"
#include "ast/treevisitor.hpp"
#include "common/compiler.hpp"
#include "symbolstack.hpp"
#define KVANTUM_TYPE_ERROR Type::get("Void")

using std::set;
using std::stack;

namespace kvantum::parser
{
   class TypeChecker : public TreeVisitor
   {
   IMPLEMENTS_TREE_VISITOR
   public:
      TypeChecker();
      void checkModule(Module* mod);
      void checkFunction(FunctionNode* node);
      void checkObject(ObjectType* type);

      Type& handleVariable(Variable* var, bool assig);
   private:
      Type& getType(Expression* expr);
      Type& getVariableType(Variable* var);
      void setVariableType(Variable* var,Type& t);
      Type& visitExpression(Expression* e);
      FunctionNode* getCurrentFunc() const { return functionCheckStack.top(); }

      Module* mod;
      set<FunctionNode*> checkedFunctions;
      SymbolStack<Type&>* symbols;
      stack<FunctionNode*> functionCheckStack;
   };
}