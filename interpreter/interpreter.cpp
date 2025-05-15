#include "interpreter/interpreter.hpp"

namespace kvantum::interpreter
{
    Value* VirtualFunctionInterpreter::interpret(string funcname, vector<Value*> args)
    {
        if (!functions.count(funcname))
            throw std::invalid_argument("no function named " + funcname);
        return functions.at(funcname)(args);
    }

    Value* VirtualFunctionInterpreter::printf(vector<Value*> args)
    {
        std::cout << args[0]->asStr()->value;
        return (Value*) new VoidValue();
    }

    Value* VirtualFunctionInterpreter::malloc(vector<Value*> args)
    {
        return (Value*) new VoidValue();
    }

    Value* VirtualFunctionInterpreter::memcpy(vector<Value*> args)
    {
        return (Value*) new VoidValue();
    }

    void Interpreter::generate(Module* mod)
    {
        this->mod = mod;
    }

    void Interpreter::generateFunction(FunctionNode* f) {}
    void Interpreter::prototypeFunction(FunctionNode* f) {}
    void Interpreter::generateObject(ObjectType* t) {}

    void Interpreter::exec()
    {
        interpretFunction(mod->getMainFunction(), {});
    }

    Value* Interpreter::interpretFunction(FunctionNode* node, vector<Value*> args)
    {
        vector<pair<string, Value*>> values;
        values.resize(args.size());
        for (int i = 0; i < args.size(); i++) {
            values[i] = pair(node->formalParams[i]->id, args[i]);
        }
        symbols.pushSegment(values);

        int i = 0;
        returnVal = nullptr;
        while (i < node->ast.size() && returnVal == nullptr) {
            visit_statement(node->ast[i++]);
        }
        symbols.popSegment();
        return (Value*) returnVal ? returnVal : new VoidValue();
    }

    any Interpreter::visit(Literal* literal)
    {
        PrimitiveType &t = literal->type.asPrimitive();
        Value* value = new VoidValue();
        switch (t.type) {
            case PrimitiveType::Integer:
                value = new IntValue(std::stoi(literal->value));
                break;
            case PrimitiveType::Float:
                value = new RatValue(std::stod(literal->value));
                break;
            case PrimitiveType::Char:
                value = new StrValue(literal->value);
                break;
            case PrimitiveType::Boolean:
                value = new BoolValue(literal->value == "True");
                break;
        }
        return (Value*) value;
    }

    any Interpreter::visit(BinaryOperation* bop)
    {
        Value* l = eval(bop->lhs);
        Value* r = eval(bop->rhs);
        Value* value = new VoidValue();
        switch (bop->op) {
            CASE(BinaryOperation::ADD, value = l->add(r));
            CASE(BinaryOperation::SUBTRACT, value = l->sub(r));
            CASE(BinaryOperation::MULTIPLY, value = l->mul(r));
            CASE(BinaryOperation::DIVIDE, value = l->div(r));
            CASE(BinaryOperation::EQUAL, value = l->sub(r));
            CASE(BinaryOperation::NOT_EQUAL, value = l->sub(r));
            CASE(BinaryOperation::LESS, value = l->sub(r));
            CASE(BinaryOperation::LESS_OR_EQUAL, value = l->sub(r));
            CASE(BinaryOperation::GREATER, value = l->sub(r));
            CASE(BinaryOperation::GREATER_OR_EQUAL, value = l->sub(r));
            CASE(BinaryOperation::AND, value = l->sub(r));
            CASE(BinaryOperation::OR, value = l->sub(r));
        }
        return (Value*) value;
    }

    any Interpreter::visit(Variable* var)
    {
        if (!var->isField())
            return (Value*) symbols.get(var->id);
    }

    any Interpreter::visit(DynamicAllocation* alloc)
    {
        Value* arg = eval(alloc->sizeExpr);
        return builtinInterpreter.interpret("malloc", {arg});
    }

    any Interpreter::visit(ArrayExpression* arr)
    {
        vector<Value*>* values = new vector<Value*>();
        for (auto &e: arr->initializer) {
            values->push_back(eval(e));
        }
        return (Value*) new ArrayValue(values);
    }

    any Interpreter::visit(FunctionCall* fcall)
    {
        auto evalArgs = [this](vector<Expression*> arguments) {
            return apply(ITER_THROUGH(arguments), std::function([this](Expression* e) {
                return eval(e);
            }));
        };

        if (builtinInterpreter.isValidFunction(fcall->fnode->getName())) {
            return builtinInterpreter.interpret(fcall->fnode->getName(), evalArgs(fcall->arguments));
        }
        return interpretFunction(fcall->fnode, evalArgs(fcall->arguments));
    }

    any Interpreter::visit(ArrayIndex* ind)
    {
        auto base = eval(ind->baseArray);
        auto index = eval(ind->index);
        return base->asArray()->index(index->asInt()->value);
    }

    any Interpreter::visit(TakeReference* ref)
    {
        return eval(ref->baseExpr);
    }

    any Interpreter::visit(Cast* cast)
    {
        auto val = eval(cast->expr);
        return new VoidValue();
    }

    void Interpreter::visit(Assigment* assig)
    {
        if (assig->isDeclaration())
            symbols.push({assig->variable->id, eval(assig->expr)});
        symbols.getNode(assig->variable->id).second = eval(assig->expr);
    }

    void Interpreter::visit(If_Else* if_else)
    {
        if (eval(if_else->condition)->asBool()->value)
            visit_statement(if_else->ifBlock);
        else if (if_else->elseBlock)
            visit_statement(if_else->elseBlock);
    }

    void Interpreter::visit(While* while_loop)
    {
        while (returnVal == nullptr && eval(while_loop->condition)) {
            visit_statement(while_loop->block);
        }
    }

    void Interpreter::visit(Return* ret)
    {
        returnVal = eval(ret->expr);
    }

    void Interpreter::visit(StatementBlock* block)
    {
        symbols.pushSegment({});
        int i = 0;
        while (i < block->block.size() && returnVal == nullptr) {
            visit_statement(block->block[i++]);
        }
        symbols.popSegment();
    }
    void Interpreter::visit(For* f) {}


    Value* Interpreter::eval(Expression* expr)
    {
        return any_cast<Value*>(visit_expression(expr));
    }
}