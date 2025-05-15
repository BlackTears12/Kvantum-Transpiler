#include "ast/ast.hpp"
#include "lexer/scope.hpp"

namespace kvantum
{
   void StatementVisitor::visit_statement(Statement* statement)
   {
       if (statement->sttype == StatementType::BLOCK)
           currentBlock = (StatementBlock*)statement;
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
       KVANTUM_VERIFY_ABANDON(currentBlock != nullptr,"Invalid list initialization: no owning block found");
       for (int i = 0; i < currentBlock->block.size(); i++) {
           if (currentBlock->block[i] == current){
               currentBlock->block.insert(currentBlock->block.begin()+i,item);
               return;
           }
       }
       panic("Invalid list initialization: owning block doesn't contain current statement");
   }

   FieldAccess* Variable::asField(){ return static_cast<FieldAccess*>(this); }

   FunctionNode::FunctionIdentifier::FunctionIdentifier(FunctionCall* fcall) : FunctionIdentifier::FunctionIdentifier(fcall->var->id,fcall->arguments)
   {
       if (fcall->var->isField()) {
           auto fa = fcall->var->as<FieldAccess*>();
           name = fa->base->as<Variable*>()->id;
           parentObj = fa->field->id;
       }
   }

   string FunctionNode::FunctionIdentifier::createName() const
   {
       string name = parent != Type::get("Void") ? parent.getName() + "_" : "";
       name += this->name;
       for (auto& e : params)
           name += "_" + e->getName();
       return name;
   }

   bool FunctionNode::FunctionIdentifier::operator==(const FunctionIdentifier& other) const{
       bool eq = this->name == other.name && this->params.size() == other.params.size();
       int i = 0;
       while (eq && i < this->params.size() && this->params[i] == other.params[i]) {
           i++;
       }
       return i == this->params.size() && eq;
   }

   array<Annotation,Annotation::ANNOTATION_NUMBER> Annotation::annotations = {Annotation(Type::Native)};
}