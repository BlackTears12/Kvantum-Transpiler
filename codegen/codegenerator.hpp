#ifdef CODEGEN
#pragma once
#include <optional>
#include "common/mainheader.hpp"
#include "codegen/functioncodegenerator.hpp"
#include "parser/parser.hpp" 
#include "llvm-14/llvm/MC/TargetRegistry.h"
#include "llvm-14/llvm/Support/FileSystem.h"
#include "llvm-14/llvm/Support/TargetSelect.h"
#include "llvm-14/llvm/Support/raw_ostream.h"
#include "llvm-14/llvm/Target/TargetMachine.h"
#include "llvm-14/llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Endian.h"


namespace kvantum::codegen
{
   class CodeGenerator
   {
   public:
      CodeGenerator(parser::Parser* parser);
      void generate();
   private:
      parser::Parser* parser;
      map<Type*,llvm::Type*> typemap;
      void constructLLVMType(ObjectType* type);
      void initializeTypes();
      
      llvm::LLVMContext* context;
      llvm::Module* Currmodule;
      llvm::IRBuilder<>* builder;
   };
}


#endif