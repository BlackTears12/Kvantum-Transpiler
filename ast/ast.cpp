#include "ast/ast.hpp"
#include "ast/functionnode.hpp"
#include "lexer/scope.hpp"

namespace kvantum {
void StatementVisitor::visit_statement(Statement* statement)
{
    if (statement->sttype == StatementType::BLOCK)
        currentBlock = (StatementBlock*) statement;
    current = statement;
    state = NodeState::isStatement;
    kvantum::Diagnostics::setLineIndex(current->lineIndex);
    statement->operationVisit(this);
}

any ExpressionVisitor::visit_expression(Expression* expr)
{
    current = expr;
    state = NodeState::isExpression;
    kvantum::Diagnostics::setLineIndex(current->lineIndex);
    return expr->operationVisit(this);
}

void StatementVisitor::insertBeforeThis(Statement* item)
{
    KVANTUM_VERIFY_ABANDON(currentBlock != nullptr,
                           "Invalid list initialization: no owning block found");
    for (int i = 0; i < currentBlock->block.size(); i++) {
        if (currentBlock->block[i] == current) {
            currentBlock->block.insert(currentBlock->block.begin() + i, item);
            return;
        }
    }
    panic("Invalid list initialization: owning block doesn't contain current statement");
}

FieldAccess* Variable::asField()
{
    return static_cast<FieldAccess*>(this);
}

array<Annotation, Annotation::ANNOTATION_NUMBER> Annotation::annotations = {
    Annotation(Type::Native)};

Type& FunctionCall::getType()
{
    return fnode->getReturnType();
}

} // namespace kvantum
