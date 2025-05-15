#ifdef CODEGEN

#include "llvm-14/llvm/ADT/APFloat.h"
#include "llvm-14/llvm/ADT/STLExtras.h"
#include "llvm-14/llvm/IR/BasicBlock.h"
#include "llvm-14/llvm/IR/Constants.h"
#include "llvm-14/llvm/IR/DerivedTypes.h"
#include "llvm-14/llvm/IR/Function.h"
#include "llvm-14/llvm/IR/IRBuilder.h"
#include "llvm-14/llvm/IR/LLVMContext.h"
#include "llvm-14/llvm/IR/Module.h"
#include "llvm-14/llvm/IR/Type.h"
#include "llvm-14/llvm/IR/Verifier.h"
#include "common/mainheader.hpp"
#include "common/ast.hpp"
#include "common/ast_nodeoperation.hpp"
#include "parser/parser.hpp"
#include <map>

using namespace kvantum;
using Value = llvm::Value;
namespace kvantum::codegen
{
   struct AllocationNode
   {
      llvm::Value* alloc;
      llvm::Type* type;
      
      AllocationNode(llvm::Value* a,llvm::Type* t){ alloc = a; type = t; }
   };
   
   class FunctionCodeGenerator : AST_NodeOperation
   {
   public:
      FunctionCodeGenerator(parser::Parser* p,map<Type*,llvm::Type*> &tm,llvm::LLVMContext* cont,llvm::Module* mod,llvm::IRBuilder<>* buildr);
      llvm::Function* generate(FunctionNode* fnode);
      void printGenerated();
   private:
      FunctionNode* fnode;
      llvm::Function* generatedFunction;
      parser::Parser* parser;
      std::map<string,AllocationNode*> variables;
      std::map<Type*,llvm::Type*> &typemap;
      llvm::LLVMContext* context;
      llvm::Module* Currmodule;
      llvm::IRBuilder<>* builder;

      virtual void visit(Literal* literal);
      virtual void visit(BinaryOperation* bop);
      virtual void visit(Variable* var);
      virtual void visit(DynamicAllocation* alloc);
      virtual void visit(Assigment* assig);
      virtual void visit(If_Else* if_else);
      virtual void visit(While* while_loop);
      virtual void visit(Return* ret);
      virtual void visit(Print* print);
      virtual void visit(FunctionCall* fcall);
      virtual void visit(StatementBlock* block);
      
      llvm::Type* getLLVMType(Type* t);
      llvm::Type* getLLVMTypeNode(Type* t);
      llvm::Value* getVariable(string value);
      llvm::Value* getVariable(Variable* var);
      void getArguments(vector<llvm::Type*> &args);
      vector<llvm::Value*> getGEPIndices(llvm::StructType* t);
      llvm::Value* createGEP(Variable* var,llvm::Instruction* alloc);

      Value* generated;
   };
}

#endif