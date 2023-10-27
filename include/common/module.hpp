#pragma once
#include "ast/ast.hpp"

namespace kvantum
{
    class Compiler;
    class Module
    {
    public:
        Module(string n,Compiler* owner);
        ~Module();

        void addFunction(FunctionNode* f) { functions.push_back(f); }
        void addObjectType(ObjectType* t);
        void addExternalFunctionDependency(string moduleName, string functionName);
        void addExternalObjectDependency(string moduleName, string typen);

        ObjectType& getObject(string name) { return (ObjectType&)getType(name); }
        Type& getType(string name) { return **findType(name); }
        FunctionNode* getFunction(const FunctionNode::FunctionIdentifier id) { return *findFunction(id); }
        vector<FunctionNode*> getFunctionGroup(string name);

        bool hasFunction(const FunctionNode::FunctionIdentifier id) { return findFunction(id) != functions.end(); }
        bool hasType(string name) { return findType(name) < types.end(); }
        bool hasInternalFunction(string name);
        bool hasInternalType(string name);

        vector<ObjectType*> getObjectTypes();
        vector<FunctionNode*> getFunctions();
        Compiler* getCompiler() { return owner; }

        string getName() { return name; }
        FunctionNode* getMainFunction() { return functions[externalFunctionIndex]; }
    private:
        vector<FunctionNode*>::iterator findFunction(const FunctionNode::FunctionIdentifier& e);
        vector<Type*>::iterator findType(string name) { return std::find_if(ITER_THROUGH(types), std::function([&name](Type* t) { return t->getName() == name; })); }

        string name;
        vector<Type*> types;
        vector<FunctionNode*> functions;
        unsigned int externalFunctionIndex;
        unsigned int externalTypeIndex;
        Compiler* owner;
    };
}
