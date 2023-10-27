#pragma once
#include "ast/ast.hpp"
#include "ast/treevisitor.hpp"
#include "common/module.hpp"
#include "c_codegen/c_codegen.hpp"
#include "codeexecutorinterface.hpp"

namespace kvantum::codegen
{
   class C_Generator : public TreeVisitor,public CodeExecutorInterface
   {
   IMPLEMENTS_TREE_VISITOR
   public:
      C_Generator();
      ~C_Generator();
      void generate(Module* mod) override;

      void generateFunction(FunctionNode* f) override;
      void prototypeFunction(FunctionNode* f) override;
      void generateObject(ObjectType* t) override;
      void exec() override;
   private:
      c::ast::Type* getCType(Type& t);
      c::ast::Expression* visitExpression(Expression* e) { return any_cast<c::ast::Expression*>(TreeVisitor::visit_expression(e)); }

      c::codegen::CodeGenerator generator;
      std::array<c::ast::Type*,PrimitiveType::Void+1> primitiveTypes;
      std::map<string, Struct*> structs;
   };
}