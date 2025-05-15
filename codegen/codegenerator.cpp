#ifdef CODEGEN

#include "codegen/codegenerator.hpp"

namespace kvantum::codegen
{
   CodeGenerator::CodeGenerator(parser::Parser* p)
   {
      parser = p;
      context = new llvm::LLVMContext();
      Currmodule = new llvm::Module("this module",*context);
      builder = new llvm::IRBuilder<>(*context);
   }
   
   void CodeGenerator::generate()
   {
      initializeTypes();
      FunctionCodeGenerator generator(parser,typemap,context,Currmodule,builder);
      std::stack<FunctionNode*> calls(parser->calledFunctions());
      while(!calls.empty()){
         generator.generate(calls.top());
         calls.pop();
      }
      generator.generate(parser->mainf);
      generator.printGenerated();
      
      string targetTriple = llvm::sys::getDefaultTargetTriple();
      llvm::InitializeAllTargetInfos();
      llvm::InitializeAllTargets();
      llvm::InitializeAllTargetMCs();
      llvm::InitializeAllAsmParsers();
      llvm::InitializeAllAsmPrinters();
      
      string error;
      auto target = llvm::TargetRegistry::lookupTarget(targetTriple,error);
      llvm::TargetOptions options;
      auto targetMachine = target->createTargetMachine(targetTriple,"generic","",options,llvm::Reloc::Model());
      Currmodule->setDataLayout(targetMachine->createDataLayout());
      Currmodule->setTargetTriple(targetTriple);
      
      auto filename = "compiled.o";
      std::error_code errorcode;
      llvm::raw_fd_ostream dest(filename,errorcode,llvm::sys::fs::OF_Text);
      llvm::legacy::PassManager pass;
      
      targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile);
      //pass.run(*Currmodule);
      dest.flush();
   }
   
   void CodeGenerator::initializeTypes()
   {
      typemap[Type::get(Type::Integer)] = llvm::Type::getInt32Ty(*context);
      typemap[Type::get(Type::Rational)] = llvm::Type::getFloatTy(*context);
      typemap[Type::get(Type::Boolean)] = llvm::Type::getInt8Ty(*context);
      typemap[Type::get(Type::Void)] = llvm::Type::getVoidTy(*context);
      typemap[Type::get(Type::String)] = llvm::Type::getInt8Ty(*context);
      typemap[Type::get(Type::Object)] = llvm::Type::getInt1PtrTy(*context);
      
      for(auto &e : ObjectType::getTypeDefinitions()){
         constructLLVMType(e);
      }
   }
   
   void CodeGenerator::constructLLVMType(ObjectType* type)
   {
      llvm::StructType* llvmtype = llvm::StructType::create(*context,type->getNode()->name);
      typemap[type] = llvmtype;
      
      std::vector<llvm::Type*> fields;
      fields.reserve(type->getNode()->fields.size());
      for(auto &e : type->getNode()->fields){
         fields.push_back(typemap[e.second]);
      }
      llvmtype->setBody(fields);
   }
}

#endif