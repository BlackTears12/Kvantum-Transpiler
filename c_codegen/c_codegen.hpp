#pragma once

#include "c_ast.hpp"
#include <stack>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace c::ast;
using std::stack;

namespace c::codegen
{
   struct Module
   {
      Module(string n) : name(n){}

      Function* getFunction(string name)
      {
         auto f = std::find_if(functions.begin(),functions.end(),[&name](Function* f){ return f->name == name; });
         if(f < functions.end())
            return *f;
         for(auto &e : dependecies){
            auto func = e->getFunction(name);
            if(func)
               return func;
         }
         return nullptr;
      }

      Struct* getStruct(string name)
      {
         auto f = std::find_if(structs.begin(),structs.end(),[&name](Struct* f){ return f->name == name; });
         if(f < structs.end())
            return *f;
         for(auto &e : dependecies){
            auto strct = e->getStruct(name);
            if(strct)
               return strct;
         }
         return nullptr;
      }

      void addDependency(Module* m)
      {
         dependecies.push_back(m);
      }

      string name;
      vector<Function*> functions;
      vector<Struct*> structs;
      vector<Module*> dependecies;
   };

   class CodeGenerator
   {
   public:
      CodeGenerator() { initStl(); }
      void setModule(string name);
      void setInsertPoint(Block* f){ pushBlock(f); }

      void pushBlock(Block* b = nullptr){ blocks.push(b ? b : new Block()); }
      void popBlock(){ blocks.pop(); }
      Assigment* createAssignment(Variable* v,Expression* expr,bool decl = false);
      Function* createFunction(string name,c::ast::Type* returnt);
      Struct* createStruct(string name,vector<Variable*> fields);
      VariableDeclaration* createDeclaration(Variable* v);
      Return* createReturn(Expression* expr);

      FunctionCall* createFunctionCall(Function* func,vector<Expression*> args);
      FunctionCall* createFunctionCall(string name,vector<Expression*> args);
      void functionPrototype(string name,c::ast::Type* returnt = c::ast::Type::getVoid(),vector<Variable*> args = {});
      void structPrototype(string name) { currentModule()->structs.push_back(new Struct(name)); }
      void setDependencies(vector<string> depends){/*todo*/}

      void writeGenerated(){ writeModule(currentModule()); } 
      Function* getFunction(string name)
      { 
         auto f = currentModule()->getFunction(name);
         if(!f)
            throw std::invalid_argument("no function named "+name);
         return f; 
      }
      Struct* getStruct(string name) { return currentModule()->getStruct(name); }
   private:
      void writeModule(Module* mod);
      Module* currentModule(){ return modules[modules.size()-1]; }
      void initStl();

      vector<Module*> modules;
      stack<Block*> blocks;
   };
}