#pragma once
#include "value.hpp"
#include "ast/ast.hpp"
#include "ast/treevisitor.hpp"
#include "codegen/codeexecutorinterface.hpp"
#include "parser/symbolstack.hpp"

namespace kvantum::interpreter
{
    class VirtualFunctionInterpreter 
    {
    public:
        Value* interpret(string funcname,vector<Value*> args);
        bool isValidFunction(string name) { return functions.count(name); }
    private:
        static Value* printf(vector<Value*> args);
        static Value* malloc(vector<Value*> args);
        static Value* memcpy(vector<Value*> args);

        const map<string, std::function<Value* (vector<Value*>)>> functions = 
        { 
            {"printf",printf},
            {"malloc",malloc},
            {"memcpy",memcpy}
        };
    };

   using kvantum::parser::SymbolStack;
   class Interpreter : public TreeVisitor,public codegen::CodeExecutorInterface
   {
   IMPLEMENTS_TREE_VISITOR
   public:
      void generate(Module* mod) override;
      void generateFunction(FunctionNode* f) override;
      void prototypeFunction(FunctionNode* f) override;
      void generateObject(ObjectType* t) override;
      void exec() override;
   private:
      Value* eval(Expression* expr);
      Value* interpretFunction(FunctionNode* func,vector<Value*> args);

      Module* mod;
      SymbolStack<Value*> symbols;
      VirtualFunctionInterpreter builtinInterpreter;
      Value* returnVal = nullptr;
   };
}