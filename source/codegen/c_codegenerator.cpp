#include "codegen/c_codegenerator.hpp"

namespace kvantum::codegen
{
    C_Generator::C_Generator() : generator()
    {
        primitiveTypes[PrimitiveType::Integer] = c::ast::Type::getInt32();
        primitiveTypes[PrimitiveType::Float] = c::ast::Type::getDouble();
        primitiveTypes[PrimitiveType::Boolean] = c::ast::Type::getUInt8();
        primitiveTypes[PrimitiveType::Char] = c::ast::Type::getUInt8();
        primitiveTypes[PrimitiveType::Void] = c::ast::Type::getVoid();
    }

    void C_Generator::exec()
    {
        generator.writeGenerated();
    }

    C_Generator::~C_Generator() {}

    void C_Generator::generateFunction(FunctionNode* func)
    {
        std::cout << "generatring code for " << func->getName() << std::endl;
        auto f = generator.createFunction(func->getID(), getCType(func->getReturnType()));
        for (auto &e: func->formalParams) {
            f->formalParams.push_back(new c::ast::Variable(e->id, getCType(e->getType())));
        }
        for (auto &e: func->ast) {
            visit_statement(e);
        }
        generator.popBlock();
    }

    void C_Generator::prototypeFunction(FunctionNode* f)
    {
        generator.functionPrototype(f->getID());
    }

    void C_Generator::generateObject(ObjectType* t)
    {
        std::cout << "generating code for " + t->getName() << std::endl;

        generator.structPrototype(t->getTypeID());
        auto fields = t->getFields();
        generator.createStruct(t->getTypeID(), apply(ITER_THROUGH(fields), std::function([this](std::pair<string, Type*> p) {
            return new c::ast::Variable(p.first, getCType(*p.second));
        })));
    }

    void C_Generator::generate(Module* mod)
    {
        generator.setModule(mod->getName());
        auto fns = mod->getFunctions();
        std::for_each(fns.begin(), fns.end(), [this](kvantum::FunctionNode* f) { this->prototypeFunction(f); });

        for (auto &e: mod->getObjectTypes())
            generateObject(e);

        for (auto &e: mod->getFunctions())
            generateFunction(e);
    }

    any C_Generator::visit(Literal* literal)
    {
        return (c::ast::Expression*) new c::ast::Literal(literal->value, getCType(literal->type));
    }

    any C_Generator::visit(BinaryOperation* bop)
    {
        auto l = visitExpression(bop->lhs);
        auto r = visitExpression(bop->rhs);
        string ops[] = {"+", "-", "*", "/", "==", "!=", "<", "<=", ">", ">=", "&&", "||"};
        return (c::ast::Expression*) new c::ast::BinaryOperation(l, r, ops[bop->op]);
    }

    any C_Generator::visit(Variable* var)
    {
        c::ast::Expression* generated = new c::ast::Variable(var->id, getCType(var->getType()));
        if (var->isField()) {
            auto field = (c::ast::Variable*) generated;
            auto f = visitExpression(var->as<FieldAccess*>()->base);
            generated = new c::ast::FieldAccess(f, field);
        }
        return generated;
    }

    any C_Generator::visit(DynamicAllocation* alloc)
    {
        auto getcoret = [this](Type &t) {
            if (t.isPrimitive())
                return primitiveTypes[t.asPrimitive().type];
            return c::ast::Type::getStruct(generator.getStruct(t.getName()));
        };

        return (c::ast::Expression*) new c::ast::FunctionCall(generator.getFunction("malloc"), {visitExpression(alloc->getSizeExpr())});
    }

    any C_Generator::visit(ArrayExpression* arr)
    {
        return (c::ast::Expression*) new c::ast::ArrayExpression(apply(ITER_THROUGH(arr->initializer), std::function([this](Literal* e) {
            return (c::ast::Literal*) any_cast<c::ast::Expression*>(visit_expression(e));
        })));
    }

    any C_Generator::visit(ArrayIndex* arr)
    {
        auto arrExp = visitExpression(arr->baseArray);
        auto arrInd = visitExpression(arr->index);
        return (c::ast::Expression*) new c::ast::ArrayIndex(arrExp, static_cast<c::ast::Variable*>(arrInd));
    }

    any C_Generator::visit(Cast* cast)
    {
        return (c::ast::ArrayExpression*) nullptr;
    }

    any C_Generator::visit(kvantum::TakeReference*)
    {
        return nullptr;
    }

    void C_Generator::visit(Assigment* assig)
    {
        auto var = visitExpression(assig->variable);
        auto expr = visitExpression(assig->expr);
        generator.createAssignment((c::ast::Variable*) var, expr, assig->isDeclaration());
    }

    void C_Generator::visit(If_Else* if_else)
    {

    }

    void C_Generator::visit(While* while_loop)
    {

    }

    void C_Generator::visit(For* for_loop) {}

    void C_Generator::visit(Return* ret)
    {
        generator.createReturn(visitExpression(ret->expr));
    }

    any C_Generator::visit(FunctionCall* fcall)
    {
        vector<c::ast::Expression*> args;
        for (auto &e: fcall->arguments) {
            args.push_back(visitExpression(e));
        }
        if (state = NodeState::isExpression)
            return (c::ast::Expression*) new c::ast::FunctionCall(generator.getFunction(fcall->fnode->getID()), args);
        else
            return generator.createFunctionCall(generator.getFunction(fcall->fnode->getID()), args);
    }

    void C_Generator::visit(StatementBlock* block)
    {
        generator.pushBlock();
        for (auto &e: block->block) {
            visit_statement(e);
        }
        generator.popBlock();
    }

    c::ast::Type* C_Generator::getCType(Type &t)
    {
        if (t.isPrimitive())
            return primitiveTypes[t.asPrimitive().type];
        if (t.isObject())
            if (t == ObjectType::getObject())
                return c::ast::Type::getPointer(c::ast::Type::getVoid());
            else return c::ast::Type::getPointer(c::ast::Type::getStruct(generator.getStruct(t.getTypeID())));
        if (t.isArray())
            return c::ast::Type::getPointer(getCType(t.asArray().getType()));
        else throw std::invalid_argument("unkown type encountered");
    }
}