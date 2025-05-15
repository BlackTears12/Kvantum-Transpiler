#pragma once
#include "ast/ast.hpp"
#include "common/module.hpp"

namespace kvantum::codegen
{
    /*
    * An interface for codegenerators and interpreter to implement
    */
    class CodeExecutorInterface
    {
    public:
        virtual void generate(Module* mod) = 0;
        virtual void generateFunction(FunctionNode* f) = 0;
        virtual void prototypeFunction(FunctionNode* f) = 0;
        virtual void generateObject(ObjectType* t) = 0;
        virtual void exec() = 0;
    };
}
