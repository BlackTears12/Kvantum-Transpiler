#pragma once
#include "ast/ast.hpp"

namespace kvantum {
class Compiler;

class Module
{
public:
    Module(const string &n, Compiler *owner);
    ~Module();

    void addFunction(FunctionNode *f);
    void addObjectType(ObjectType *t);
    void addExternalFunctionDependency(string moduleName, string functionName);
    void addExternalObjectDependency(string modusleName, string typen);

    ObjectType &getObject(string name);
    Type &getType(string name);
    FunctionNode *getFunction(const FunctionNode::FunctionIdentifier id);
    vector<FunctionNode *> getFunctionGroup(string name);

    bool hasFunction(const FunctionNode::FunctionIdentifier id);
    bool hasType(string name);
    bool hasInternalFunction(string name);
    bool hasInternalType(string name);

    vector<ObjectType *> getObjectTypes();
    vector<FunctionNode *> getFunctions();
    Compiler *getCompiler();

    string getName();
    FunctionNode *getMainFunction();

private:
    vector<FunctionNode *>::iterator findFunction(const FunctionNode::FunctionIdentifier &e);
    vector<Type *>::iterator findType(string name);

    string name;
    vector<Type *> types;
    vector<FunctionNode *> functions;
    unsigned int externalFunctionIndex;
    unsigned int externalTypeIndex;
    Compiler *owner;
};
} // namespace kvantum
