#include "common/module.hpp"
#include "common/compiler.hpp"
#include <algorithm>

namespace kvantum {
Module::Module(const string &n)
    : name(n)
{
    for (auto t : PrimitiveType::getTypes()) {
        types.push_back(t);
    }
    types.push_back(&ObjectType::getObject());
    externalFunctionIndex = 0;
    externalTypeIndex = 0;
}

Module::~Module()
{
    auto objectTypes = getObjectTypes();
    for (auto &e : objectTypes) {
        delete e;
    }
}

void Module::addFunction(unique_ptr<FunctionNode> f)
{
    functions.push_back(std::move(f));
}

bool Module::hasInternalFunction(string name)
{
    for (int i = externalFunctionIndex; i < functions.size(); i++) {
        if (functions[i]->getName() == name)
            return true;
    }
    return false;
}

bool Module::hasInternalType(string name)
{
    auto iter = findType(name);
    return iter < types.end() && iter >= types.begin() + externalTypeIndex;
}

void Module::addExternalFunctionDependency(string moduleName, string funcname)
{
    auto funcs = Compiler::Instance().getFunctionGroup(moduleName, funcname);
    for (auto &func : funcs) {
        KVANTUM_VERIFY(func->hasTrait(FunctionNode::PUBLIC),
                       "cannot use function " + funcname + " because its private for module "
                           + getName());
        else
        {
            functions.insert(functions.begin(), func);
            externalFunctionIndex++;
        }
    }
}

void Module::addExternalObjectDependency(string moduleName, string typen)
{
    types.insert(types.begin(), &Compiler::Instance().getObject(moduleName, typen));
    externalTypeIndex++;
    if ((*types.begin())->isObject()) {
        for (auto &e : (*types.begin())->asObject().getMethods()) {
            if (e.second->hasTrait(FunctionNode::PUBLIC))
                addExternalFunctionDependency(moduleName, e.second->getName());
        }
    }
}

ObjectType &Module::getObject(string name)
{
    return (ObjectType &) getType(name);
}

Type &Module::getType(string name)
{
    return **findType(name);
}

FunctionNode *Module::getFunction(const FunctionNode::FunctionIdentifier id)
{
    return *findFunction(id);
}

vector<FunctionNode *> Module::getFunctionGroup(string name)
{
    vector<FunctionNode *> funcs;
    for (auto &e : functions) {
        if (e->getName() == name)
            funcs.push_back(e);
    }
    if (funcs.empty())
        panic("no function named " + name + " in module " + this->getName());
    return funcs;
}

bool Module::hasFunction(const FunctionNode::FunctionIdentifier id)
{
    return findFunction(id) != functions.end();
}

bool Module::hasType(string name)
{
    return findType(name) < types.end();
}

vector<FunctionNode *> Module::getFunctions()
{
    std::vector<FunctionNode *> funcs;
    std::for_each(functions.begin() + externalFunctionIndex,
                  functions.end(),
                  [&funcs](FunctionNode *f) { funcs.push_back(f); });
    return funcs;
}

string Module::getName()
{
    return name;
}

FunctionNode *Module::getMainFunction()
{
    return functions[externalFunctionIndex];
}

vector<ObjectType *> Module::getObjectTypes()
{
    return apply((vector<Type *>::iterator) types.begin() + PrimitiveType::Void + 1
                     + externalTypeIndex,
                 types.end(),
                 std::function(
                     [](Type *t) -> ObjectType * { return dynamic_cast<ObjectType *>(t); }));
}

vector<FunctionNode *>::iterator Module::findFunction(const FunctionNode::FunctionIdentifier &e)
{
    ///if its a primive fcall find the function by name
    return std::find_if(ITER_THROUGH(functions), [&e](auto &f) { return e == f->getFunctionID(); });
}

vector<Type *>::iterator Module::findType(string name)
{
    return std::find_if(ITER_THROUGH(types),
                        std::function([&name](auto &t) { return t->getName() == name; }));
}

void Module::addObjectType(unique_ptr<ObjectType> t)
{
    auto ptr = t.get();
    t.release();
    types.push_back(ptr);
}
} // namespace kvantum
