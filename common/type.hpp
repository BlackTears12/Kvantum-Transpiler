#pragma once
#include "mainheader.hpp"
#include <algorithm>
#include <array>
#include <map>
#include <memory>
using std::map;
using std::unique_ptr;

namespace kvantum {
class Type;
class Variable;
struct Token;
struct FunctionNode;
struct GenericObjectNode;

struct TypeNode
{
    string name;
    map<string, Type *> fields;
    map<string, FunctionNode *> methods;

    TypeNode(string n)
        : name(n)
        , fields()
        , methods()
    {}
    TypeNode *copy(string newID = "")
    {
        if (newID == "")
            newID = name;
        auto node = new TypeNode(newID);
        std::for_each(ITER_THROUGH(fields), [&node](std::pair<string, Type *> f) {
            node->fields.emplace(f.first, f.second);
        });
        std::for_each(ITER_THROUGH(methods), [&node](std::pair<string, FunctionNode *> f) {
            node->methods.emplace(f.first, f.second);
        });
        return node;
    }
};

class PrimitiveType;
class ObjectType;
class ArrayType;
class ReferenceType;
class Type
{
public:
    enum Trait { Public };

    virtual ~Type() = default;
    virtual string getName() const = 0;
    virtual string getTypeID() const { return getName(); }
    virtual bool isObject() const { return false; }
    virtual bool isPrimitive() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isReference() const { return false; }
    virtual bool isVoid() const { return false; }
    virtual bool equals(Type &other) const = 0;
    virtual bool weakEquals(Type &other) { return equals(other); }
    virtual unsigned int getAllocSize() = 0;

    PrimitiveType &asPrimitive();
    ObjectType &asObject();
    ArrayType &asArray();
    ReferenceType &asReference();

public:
    static void initialize();

    static Type &get(string name);
    static unsigned int getPointerAllocSize() { return 4; }

    friend bool operator==(Type &l, Type &r) { return l.equals(r) || r.equals(l); }
    friend bool operator!=(Type &l, Type &r) { return !(l == r); }
};

class PrimitiveType : public Type
{
public:
    enum TypeBase { Integer, Float, Boolean, Char, Void };
    explicit PrimitiveType(TypeBase b)
        : type(b)
    {}

    string getName() const override;
    bool isPrimitive() const override { return true; }
    bool equals(Type &other) const override
    {
        return other.isPrimitive() && other.asPrimitive().type == type;
    }
    bool isVoid() const override { return type == TypeBase::Void; }
    unsigned int getAllocSize() override { return 4; }

    TypeBase type;

public:
    static void initialize();
    static PrimitiveType &get(TypeBase t) { return *types[t]; }

    static std::array<PrimitiveType *, TypeBase::Void + 1> getTypes()
    {
        std::array<PrimitiveType *, types.size()> arr{};
        for (int i = 0; i < arr.size(); i++)
            arr[i] = types[i].get();
        return arr;
    }

private:
    static std::array<unique_ptr<PrimitiveType>, TypeBase::Void + 1> types;
};

class ObjectType : public Type
{
public:
    explicit ObjectType(TypeNode *node, ObjectType *parent = nullptr)
    {
        this->node = node;
        this->parent = parent;
    }
    ~ObjectType() override { delete node; }

    bool isObject() const override { return true; }
    bool equals(Type &other) const override;
    string getName() const override { return node->name; }
    unsigned int getAllocSize() override;
    TypeNode *getNode() const { return node; }

    bool hasFunction(string name)
    {
        return node->methods.count(name) + (parent ? parent->hasFunction(name) : 0) > 0;
    }
    bool hasFunction(Variable *var);
    bool hasField(Variable *var);

    Type &getFieldType(const string id);
    FunctionNode *getFunction(Variable *var);
    FunctionNode *getFunction(string name) { return node->methods.at(name); }
    void addFunction(string name, FunctionNode *fnode);

    vector<std::pair<string, Type *>> getFields();
    vector<std::pair<string, FunctionNode *>> getMethods();

protected:
    TypeNode *node;
    ObjectType *parent;

public:
    static ObjectType &getObject() { return *object; }
    static void initialize();

private:
    static unique_ptr<ObjectType> object;
};

class ArrayType : public Type
{
public:
    string getName() const override { return "<" + type.getName() + ">"; }
    bool isArray() const override { return true; }
    unsigned int getAllocSize() override { return 4; }
    bool equals(Type &other) const override
    {
        return other.isArray() && other.asArray().getType() == type;
    }
    Type &getType() { return type; }

private:
    ArrayType(Type &t)
        : type(t)
    {}
    ~ArrayType() {}
    Type &type;

public:
    static ArrayType &get(Type &itemT);

private:
    static map<Type *, ArrayType *> initiatedArrays;
};

class ListType : public ObjectType
{
public:
    ListType(Type &t);
    static ListType &get(Type &t);
    string getTypeID() const override { return "[" + type.getName() + "]"; }

private:
    Type &type;
    static map<Type *, ListType *> initiatedLists;
};

class ReferenceType : public Type
{
public:
    static ReferenceType &get(Type &t);
    string getName() const override { return "&" + referenceOf.getName(); }
    unsigned int getAllocSize() override { return Type::getPointerAllocSize(); }
    Type &getReferencedType() const { return referenceOf; };
    bool isReference() const override { return true; }
    bool equals(Type &other) const override
    {
        return other.isReference() && other.asReference().referenceOf == referenceOf;
    }

private:
    explicit ReferenceType(Type &refOf)
        : referenceOf(refOf)
    {}
    Type &referenceOf;
    static map<Type *, ReferenceType *> initatedReferences;
};
} // namespace kvantum
