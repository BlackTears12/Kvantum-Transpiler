#ifdef CODEGEN

#include "codegen/functioncodegenerator.hpp"

namespace kvantum::codegen
{
   FunctionCodeGenerator::FunctionCodeGenerator(parser::Parser* p,map<Type*,llvm::Type*> &tm,
      llvm::LLVMContext* cont,llvm::Module* mod,llvm::IRBuilder<>* buildr) : typemap(tm)
   {
      context = cont;
      Currmodule = mod;
      builder = buildr;
      parser = p;
   }
   
   llvm::Function* FunctionCodeGenerator::generate(FunctionNode* fnode)
   {
      this->fnode = fnode;
      vector<llvm::Type*> args;
      getArguments(args);
      llvm::FunctionType* type = llvm::FunctionType::get(getLLVMType(fnode->returnType),args,false);
      llvm::Function* func = llvm::Function::Create(type,llvm::Function::CommonLinkage,fnode->id,*Currmodule);
      llvm::BasicBlock* blck = llvm::BasicBlock::Create(*context,"fn_start",func);
      generatedFunction = func;
      
      for(int i = 0;i < fnode->formalParams.size();i++){
         variables[fnode->formalParams[i]->id] = new AllocationNode(func->getArg(i),getLLVMType(fnode->formalParams[i]->type));
      }
      
      builder->SetInsertPoint(blck);
      for(auto &e : fnode->ast){
         start_visit(e);
      }
      if(fnode->returnType->type == Type::Void)
         builder->CreateRet(nullptr);
      llvm::verifyFunction(*func);
      return func;
   }
   
   void FunctionCodeGenerator::printGenerated()
   {
      Currmodule->print(llvm::errs(),nullptr);
   }

   void FunctionCodeGenerator::visit(Literal* literal)
   {
      generated = llvm::ConstantInt::get(*context,llvm::APInt(std::stoi(literal->value),false));
   }

   void FunctionCodeGenerator::visit(BinaryOperation* bop)
   {
      start_visit(bop->lhs);
      Value* lhs = generated;
      start_visit(bop->rhs);
      Value* rhs = generated;
      if(bop->op == "+")
         generated = builder->CreateFAdd(lhs,rhs,"addtmp");
      else if(bop->op == "-")
         generated = builder->CreateFSub(lhs,rhs,"subtmp");
      else if(bop->op == "*")
         generated = builder->CreateFMul(lhs,rhs,"multmp");
      else if(bop->op == "/")
         generated = builder->CreateFAdd(lhs,rhs,"addtmp");
   }

   void FunctionCodeGenerator::visit(Variable* var)
   {
      auto alloca = variables[var->id];
      if(var->field){
         auto gep = createGEP(var,static_cast<llvm::Instruction*>(alloca->alloc));
         generated = builder->CreateLoad(alloca->type,gep,var->id);
      }
      else generated = builder->CreateLoad(alloca->type,alloca->alloc,var->id);
   }
   
   void FunctionCodeGenerator::visit(DynamicAllocation* alloc)
   {
      llvm::Type* intTy = llvm::Type::getInt32Ty(*context);
      llvm::Constant* allocSize = llvm::ConstantExpr::getSizeOf(getLLVMTypeNode(alloc->node));
      llvm::Instruction* malloc = llvm::CallInst::CreateMalloc(builder->GetInsertBlock(),intTy, getLLVMTypeNode(alloc->node), allocSize, nullptr, nullptr, "");
      generated = malloc;
   }
   
   void FunctionCodeGenerator::visit(Assigment* assig)
   {
      llvm::Value* alloca;
      if(assig->allocation)
      {
         alloca = builder->CreateAlloca(getLLVMType(parser->getType(assig->expr)));
         variables[assig->variable->id] = new AllocationNode(alloca,static_cast<llvm::AllocaInst*>(alloca)->getAllocatedType());
      }
      else
         alloca = getVariable(assig->variable);
      start_visit(assig->expr);
      builder->CreateStore(generated,alloca);
   }
   
   void FunctionCodeGenerator::visit(If_Else* ife)
   {
      start_visit(ife->condition);
      auto Cond = builder->CreateFCmpONE(generated,llvm::ConstantFP::get(*context, llvm::APFloat(0.0)), "ifcond");
      
      llvm::Function* func = builder->GetInsertBlock()->getParent();
      llvm::BasicBlock *Then = llvm::BasicBlock::Create(*context, "then", func);
      llvm::BasicBlock *Else = llvm::BasicBlock::Create(*context, "else");
      llvm::BasicBlock *Merge = llvm::BasicBlock::Create(*context, "ifcont");
      builder->CreateCondBr(Cond,Then,Else);
      
      builder->SetInsertPoint(Then);
      if(ife->elseBlock){
         
      }
      start_visit(ife->ifBlock);
      builder->CreateBr(Merge);
      Then = builder->GetInsertBlock();
   }
   void FunctionCodeGenerator::visit(While* wh){}
   void FunctionCodeGenerator::visit(Return* ret)
   {
      start_visit(ret->expr);
      builder->CreateRet(generated);
   }
   
   void FunctionCodeGenerator::visit(Print* print)
   {
      
   }
   
   void FunctionCodeGenerator::visit(FunctionCall* fcall)
   {
      llvm::Function* func = Currmodule->getFunction(fcall->fnode->id);
      std::vector<Value*> val;
      generated = builder->CreateCall(func,val,"calltmp");
   }
   
   void FunctionCodeGenerator::visit(StatementBlock* block)
   {
      for(auto &e : block->block){
         start_visit(e);
      }
   }
   
   void FunctionCodeGenerator::getArguments(vector<llvm::Type*> &args)
   {
      for(auto &e : fnode->formalParams){
         args.push_back(getLLVMType(e->type));
      }
   }
   
   llvm::Type* FunctionCodeGenerator::getLLVMType(Type* t)
   {
      auto lt = getLLVMTypeNode(t);
      if(t->type != Type::Object)
         return lt;
      return lt->getPointerTo();
   }
   
   llvm::Type* FunctionCodeGenerator::getLLVMTypeNode(Type* t)
   {
      return typemap[t];
   }
   
   llvm::Value* FunctionCodeGenerator::getVariable(string id)
   {
      return variables[id]->alloc;
   }
   
   llvm::Value* FunctionCodeGenerator::getVariable(Variable* var)
   {
      if(var->field)
         return createGEP(var,static_cast<llvm::Instruction*>(getVariable(var->id)));
      return getVariable(var->id);
   }

   vector<llvm::Value*> FunctionCodeGenerator::getGEPIndices(llvm::StructType* t)
   {
      vector<llvm::Value*> ind;
      std::for_each(t->element_begin(),t->element_end(),[this,&ind](llvm::Type* elem){
         ind.push_back(llvm::ConstantInt::get(*context,llvm::APInt(32,ind.size(),true)));
      });
      return ind;
   }
   
   llvm::Value* FunctionCodeGenerator::createGEP(Variable* var,llvm::Instruction* alloc)
   {
      if(var->field->field == nullptr)
         return builder->CreateGEP(getLLVMTypeNode(var->type),alloc,getGEPIndices((llvm::StructType*)getLLVMTypeNode(var->type)),"memberptr");
      else return createGEP(var->field,alloc);
   }
}

#endif