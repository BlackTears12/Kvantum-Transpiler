#include "c_codegen/c_codegen.hpp"

namespace c::codegen
{
   void CodeGenerator::setModule(string name)
   {
       modules.push_back(new Module(name));
       currentModule()->addDependency(modules[0]);
   }

   void CodeGenerator::functionPrototype(string name, Type *returnt, vector<Variable *> args)
   {
       currentModule()->functions.push_back(new Function(name));
   }

   Function *CodeGenerator::createFunction(string name, c::ast::Type *returnt)
   {
       auto f = currentModule()->getFunction(name);
       if (!f) {
           f = new Function(name, returnt);
           currentModule()->functions.push_back(f);
       }
       f->returnType = returnt;
       setInsertPoint(f->block);
       return f;
   }

   Struct* CodeGenerator::createStruct(string name, vector<Variable*> fields)
   {
       auto tstruct = currentModule()->getStruct(name);
       if (!tstruct) {
           tstruct = new Struct(name, fields);
           currentModule()->structs.push_back(tstruct);
       } else
           tstruct->fields = fields;
       return tstruct;
   }

   Assigment* CodeGenerator::createAssignment(Variable* v,Expression* expr,bool decl)
   {
      auto assig = new Assigment(v,expr,decl);
      blocks.top()->insert(assig);
      return assig;
   }

   VariableDeclaration* CodeGenerator::createDeclaration(Variable* v)
   {
      auto vd = new VariableDeclaration(v);
      blocks.top()->insert(vd);
      return vd;
   }

   Return* CodeGenerator::createReturn(Expression* expr)
   {
      auto r = new Return(expr);
      blocks.top()->insert(r);
      return r;
   }

   FunctionCall* CodeGenerator::createFunctionCall(Function* func,vector<Expression*> args)
   {
      FunctionCall* fcall = new FunctionCall(func,args);
      blocks.top()->insert(fcall);
      return fcall;
   }

   FunctionCall* CodeGenerator::createFunctionCall(string name,vector<Expression*> args) { return createFunctionCall(currentModule()->getFunction(name),args); }

   void CodeGenerator::writeModule(Module* mod)
   {
      std::cout << "writing to " << mod->name << ".c\n";
      std::ofstream os(mod->name+".c");

      std::cout << "#include<stdlib.h>" << std::endl;
      os <<        "#include<stdlib.h>" << std::endl;

      std::cout << "#include<string.h>" << std::endl;
      os <<        "#include<string.h>" << std::endl;

      for (auto& e : mod->structs) {
          std::cout << e->getDefinition();
          os << e->getDefinition();
      }

      for(auto &e : mod->functions){
         std::cout << e->getPrototype() << std::endl;
         os << e->getPrototype() << std::endl;
      }
      std::cout << std::endl;
      os << std::endl;
      for(auto &e : mod->functions){
         std::cout << e->getDefiniton() << std::endl;
         os << e->getDefiniton() << std::endl;
      }
      os.close();
   }

   void CodeGenerator::initStl()
   {
      Module* stl = new Module("stl");
      stl->functions.push_back(new Function("printf"));
      stl->functions.push_back(new Function("scanf"));
      stl->functions.push_back(new Function("malloc"));
      stl->functions.push_back(new Function("memcpy"));
      modules.push_back(stl);
   }
}

#ifdef DEBUG

int main(void)
{
   c::codegen::CodeGenerator gen;
   gen.setModule("main");
   gen.functionPrototype("add");
   gen.createFunction("main",Type::getInt8());
   gen.createAssignment(new c::ast::Variable("asd",Type::getInt32()),new c::ast::Literal("10",Type::getInt32()),true);
   gen.createFunctionCall("add");
   
   gen.createFunction("add",Type::getInt8());
   gen.createDeclaration(new c::ast::Variable("asd",Type::getFloat()));
   gen.writeGenerated();
}

#endif
