#include "parser/typechecker.hpp"

namespace kvantum::parser {
TypeChecker::TypeChecker()
{
    mod = nullptr;
}

void TypeChecker::checkModule(Module *mod)
{
    Diagnostics::log("module " + mod->getName() + " is being checked\n");
    this->mod = mod;
    auto objt = mod->getObjectTypes();
    symbols.pushSegment(apply(ITER_THROUGH(objt),
                              function<pair<string, Type *>(ObjectType *)>(
                                  [this](ObjectType *t) -> pair<string, Type *> {
                                      return {t->getName(), (Type *) t};
                                  })));

    for (auto &e : mod->getFunctions())
        checkFunction(e);
    for (auto &e : mod->getObjectTypes()) {
        checkObject(e);
    }
}

void TypeChecker::checkFunction(FunctionNode *node)
{
    functionCheckStack.push(node);
    if (checkedFunctions.count(node) > 0 || node->hasAnnotation(Annotation::Native)) {
        functionCheckStack.pop();
        return;
    }
    checkedFunctions.insert(node);
    Diagnostics::log("func " + node->getName() + " is being checked\n");
    symbols.pushSegment({});
    for (auto &e : node->formalParams) {
        symbols.push({e->id, &e->getType()});
    }

    for (int i = 0; i < node->ast.size(); i++) {
        visit_statement(node->ast[i]);
    }
    symbols.popSegment();
    functionCheckStack.pop();
}

void TypeChecker::checkObject(ObjectType *type)
{
    Diagnostics::log("type " + type->getName() + " is being checked\n");
    auto node = type->getNode();
    symbols.pushSegment(apply(node->fields.begin(),
                              node->fields.end(),
                              function<pair<string, Type *>(pair<string, Type *>)>(
                                  [](pair<string, Type *> p) { return p; })));
    for (auto &e : node->methods)
        checkFunction(e.second);
}

any TypeChecker::visit(Literal *literal)
{
    return &literal->type;
}

any TypeChecker::visit(BinaryOperation *bop)
{
    auto &l = visitExpression(bop->lhs);
    auto &r = visitExpression(bop->rhs);
    if (bop->isBool())
        return &PrimitiveType::get(PrimitiveType::Boolean);
    KVANTUM_VERIFY(l == r, "binary operand types mismatch: " + l.getName() + " " + r.getName());
    return &l;
}

any TypeChecker::visit(Variable *var)
{
    return &handleVariable(var, false);
}

any TypeChecker::visit(DynamicAllocation *alloc)
{
    return &alloc->node;
}

any TypeChecker::visit(ArrayExpression *arr)
{
    auto &type = arr->getType().asArray().getType();
    for (auto &e : arr->initializer) {
        KVANTUM_VERIFY(visitExpression(e) == type,
                       e->getType().getName() + " does not equal array type "
                           + arr->getType().asArray().getType().getName());
    }
    return (Type *) &arr->type;
}

any TypeChecker::visit(ArrayIndex *arr)
{
    auto &bt = visitExpression(arr->baseArray);
    auto &it = visitExpression(arr->index);
    KVANTUM_VERIFY_ERROR(bt.isArray(), "cannot index non array type");
    KVANTUM_VERIFY_ERROR(it == PrimitiveType::get(PrimitiveType::Integer),
                         "cannot index with non integer type " + it.getName());
    return &bt.asArray().getType();
}

any TypeChecker::visit(TakeReference *ref)
{
    auto &from = visitExpression(ref->baseExpr);
    KVANTUM_VERIFY_ERROR(ref->baseExpr->exprtype == ExprType::VARIABLE,
                         "cannot take reference from non-variable");
    return &ref->getType();
}

any TypeChecker::visit(Cast *cast)
{
    visitExpression(cast->expr);
    return &cast->getType();
}

void TypeChecker::visit(Assigment *assig)
{
    handleVariable(assig->variable, true);
    auto &value = visitExpression(assig->expr);

    KVANTUM_VERIFY(assig->variable->isField() || assig->isDeclaration()
                       || symbols.isDeclared(assig->variable->id),
                   "variable not declared " + assig->variable->id);
    KVANTUM_VERIFY(assig->variable->isField() || !assig->isDeclaration()
                       || !symbols.isDeclaredLocal(assig->variable->id),
                   "redeclaration of local variable " + assig->variable->id);
    KVANTUM_VERIFY(assig->variable->getType().isVoid() || assig->variable->getType() == value,
                   "expression type " + value.getName() + " does not equal specified type "
                       + assig->variable->getType().getName());
    setVariableType(assig->variable, value);
}

void TypeChecker::visit(If_Else *if_else)
{
    visit_expression(if_else->condition);
    visit_statement(if_else->ifBlock);
    if (if_else->elseBlock)
        visit_statement(if_else->elseBlock);
}

void TypeChecker::visit(While *while_loop) {}
void TypeChecker::visit(For *for_loop) {}

void TypeChecker::visit(Return *ret)
{
    auto &type = getCurrentFunc()->getReturnType();
    auto &value = visitExpression(ret->expr);
    KVANTUM_VERIFY_ABANDON(value != KVANTUM_TYPE_ERROR, "cannot return void");

    /// if its a recursive call skip
    if (ret->expr->exprtype == ExprType::FUNCTION_CALL
        && static_cast<FunctionCall *>(ret->expr)->fnode == this->getCurrentFunc())
        return;

    KVANTUM_VERIFY(value != Type::get("Void"), "cannot return void");

    else KVANTUM_VERIFY((type == Type::get("Void")
                         && !getCurrentFunc()->hasTrait(FunctionNode::EXPLICIT_TYPE))
                            || value == type,
                        type.getName() + " is not same as " + value.getName());

    else getCurrentFunc()->setReturnType(value);
}

any TypeChecker::visit(FunctionCall *fcall)
{
    for (auto &e : fcall->arguments)
        visitExpression(e);

    FunctionNode::FunctionIdentifier identifier(fcall);
    if (identifier.isField() && identifier.getBaseType() == Type::get("Void")) {
        KVANTUM_VERIFY_ERROR(symbols.isDeclared(identifier.parentObj),
                             "no object declared " + identifier.parentObj);
        identifier.setBaseType(*symbols.get(identifier.parentObj));
    }

    KVANTUM_VERIFY_ERROR(mod->hasFunction(identifier),
                         "no function named " + identifier.name
                             + " with arguments: " + identifier.getArgumentListStr());
    fcall->setNode(mod->getFunction(identifier));
    checkFunction(fcall->fnode);

    /// if its a non static method push self as first argument
    if (!fcall->fnode->hasTrait(FunctionNode::STATIC) && fcall->var->isField())
        fcall->arguments.insert(fcall->arguments.begin(),
                                new Variable("self",
                                             fcall->var->as<FieldAccess *>()->base->getType()));

    /// verify the arguments size an type order
    KVANTUM_VERIFY(fcall->fnode->formalParams.size() == fcall->arguments.size(),
                   "function " + fcall->fnode->getName() + " expects "
                       + std::to_string(fcall->fnode->formalParams.size()) + " arguments, but "
                       + std::to_string(fcall->arguments.size()) + " were provided");

    for (int i = 0; i < std::min(fcall->fnode->formalParams.size(), fcall->arguments.size()); i++) {
        auto &e = fcall->arguments[i];
        auto &param = fcall->fnode->formalParams[i];
        KVANTUM_VERIFY(e->getType() == param->getType(),
                       e->getType().getName() + " does not equal expected "
                           + param->getType().getName());
    }
    return &fcall->fnode->getReturnType();
}

void TypeChecker::visit(StatementBlock *block)
{
    for (int i = 0; i < block->block.size(); i++)
        visit_statement(block->block[i]);
}

Type &TypeChecker::getType(Expression *expr)
{
    return visitExpression(expr);
}

Type &TypeChecker::getVariableType(Variable *var)
{
    if (symbols.isDeclared(var->id))
        return *symbols.get(var->id);
    panic(var->id + " not declared");
    return KVANTUM_TYPE_ERROR;
}

void TypeChecker::setVariableType(Variable *var, Type &t)
{
    symbols.push({var->id, &t});
    var->setType(t);
}

Type &TypeChecker::handleVariable(Variable *var, bool assig)
{
    auto value = &Type::get("Void");
    if (var->isField()) {
        auto f = var->as<FieldAccess *>();
        visit_expression(f->base);

        KVANTUM_VERIFY_RETURN(f->base->getType().isObject(),
                              "cannot access a field of a non-object "
                                  + f->base->getType().getName(),
                              KVANTUM_TYPE_ERROR);
        f->field->setType(f->base->getType().asObject().getFieldType(f->field->id));
    }
    if (assig)
        return KVANTUM_TYPE_ERROR;
    if (var->getType() == PrimitiveType::get(PrimitiveType::Void)) {
        KVANTUM_VERIFY_RETURN(symbols.isDeclared(var->id),
                              "variable not declared " + var->id,
                              KVANTUM_TYPE_ERROR);
        var->setType(getVariableType(var));
    }
    value = &var->getType();
    return *value;
}

Type &TypeChecker::visitExpression(Expression *e)
{
    auto value = visit_expression(e);
    if (!value.has_value() || value.type() == typeid(nullptr))
        return KVANTUM_TYPE_ERROR;
    return *any_cast<Type *>(value);
}
} // namespace kvantum::parser
