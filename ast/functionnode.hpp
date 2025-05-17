#ifndef FUNCTIONNODE_HPP
#define FUNCTIONNODE_HPP

#include "ast.hpp"

namespace kvantum {

struct FunctionNode
{
    struct FunctionIdentifier
    {
        FunctionIdentifier(string nm, vector<Expression *> args = {}, string p = "")
            : parentObj(std::move(p))
            , name(std::move(nm))
            , params(apply(ITER_THROUGH(args),
                           std::function([](Expression *e) -> Type * { return &e->getType(); })))
        {}
        FunctionIdentifier(string nm, vector<Variable *> args, string p = "")
            : parentObj(std::move(p))
            , name(std::move(nm))
            , params(apply(ITER_THROUGH(args),
                           std::function([](Variable *e) -> Type * { return &e->getType(); })))
        {}

        FunctionIdentifier(FunctionCall *fcall);

        bool isField() const { return !parentObj.empty(); }
        Type &getBaseType() const { return parent; }
        void setBaseType(Type &t) { parent = t; }
        string createName() const;
        bool operator==(const FunctionIdentifier &other) const;

        string getArgumentListStr() const
        {
            if (params.empty())
                return "Void";
            string str = params[0]->getName();
            for (int i = 1; i < params.size(); i++)
                str += "," + params[i]->getName();
            return str;
        }

        string parentObj;
        string name;
        vector<Type *> params;
        Type &parent = Type::get("Void");
    };

    FunctionIdentifier getFunctionID() const
    {
        return {name, formalParams, *parent != Type::get("Void") ? parent->getName() : ""};
    }

    enum Trait {
        PUBLIC = 0b00000001,
        CONST = 0b00000010,
        STATIC = 0b00000100,
        VIRTUAL = 0b00001000,
        OVERRIDE = 0b00010000,
        EXPLICIT_TYPE = 0b00100000
    };

    vector<Variable *> formalParams;
    vector<Statement *> ast;

    FunctionNode(string ids,
                 Type &retT = Type::get("Void"),
                 vector<Variable *> formal = {},
                 vector<Statement *> body = {},
                 Type &p = Type::get("Void"),
                 unsigned char tr = 0)
        : name(ids)
        , returnType(&retT)
        , formalParams(std::move(formal))
        , ast(body)
        , parent(&p)
        , traits(tr)
    {}

    ~FunctionNode()
    {
        for (auto &e : formalParams)
            delete e;
        for (auto &e : ast)
            delete e;
    }

    void setTrait(Trait t) { traits |= t; }
    void setTraitList(unsigned char t) { traits = t; }
    bool hasTrait(Trait t) const { return traits & t; }
    unsigned int getTraits() const { return traits; }

    void setName(string nm) { name = std::move(nm); }
    string getName() const { return name; }
    string getID() const { return getFunctionID().createName(); }

    void setReturnType(Type &t) { returnType = &t; }
    Type &getReturnType() const { return *returnType; }

    bool isMethod() const { return *parent != Type::get("Void"); }
    void makeMethod(Type &t) { parent = &t; }
    Type &getParent() const { return *parent; }

    void setAnnotation(Annotation *an) { annotation = an; }
    Annotation *getAnnotation() { return annotation; }
    bool hasAnnotation(Annotation::Type t) { return annotation && annotation->getType() == t; }

    FunctionNode *copy(const string &newId = "")
    {
        auto f = new FunctionNode(name, *returnType, {}, {}, *parent, traits);
        f->formalParams = apply(formalParams.begin(),
                                formalParams.end(),
                                std::function([](Variable *v) { return v->copy(); }));
        f->ast = apply(ast.begin(), ast.end(), std::function([](Statement *v) {
                           return v->copy();
                       }));
        return f;
    }

private:
    unsigned char traits = 0;
    string name;
    Type *parent = &Type::get("Void");
    Type *returnType = &Type::get("Void");
    Annotation *annotation = nullptr;
};

} // namespace kvantum

#endif // FUNCTIONNODE_HPP
