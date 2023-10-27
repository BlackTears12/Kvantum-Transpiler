#include "common/generic.hpp"
#include "common/compiler.hpp"

namespace kvantum
{
   FunctionNode* GenericGenerator::generateFunction(Variable* var,Type* t)
   {
      auto node = getFunctionNode(var);
      this->thisfunc = node->getCopy();
      if (var->isField() && var->as<FieldAccess*>()->id == "new") {
          generateType(var->as<FieldAccess*>()->base->getType()->getName(),t);
      }

      templatedTypes.clear();
      templatedTypes.insert({node->node->getGeneric(),t});

      for(auto &e : thisfunc->formalParams){
         start_visit(e);
      }

      for(auto &e : thisfunc->ast){
         start_visit(e);
      }
      thisfunc->getName() += "__GENERIC_" + t->getName();
      mod->addFunction(thisfunc);
      return thisfunc;
   }

   TypeNode* GenericGenerator::generateType(string typen, Type* t)
   {
       auto typenode = getObjectNode(typen);
       auto node = typenode->getCopy();
       for (auto& e : node->fields)
           if (e.second->isGeneric())
               e.second = t;

       for (auto& thisfunc : node->methods){
           this->thisfunc = thisfunc.second;
           for (auto& e : thisfunc.second->formalParams) {
               start_visit(e);
           }

           for (auto& e : thisfunc.second->ast) {
               start_visit(e);
           }
       }

       node->name += "__GENERIC_" + t->getName();
       mod->addObjectType(new ObjectType(node));
       return node;
   }

   GenericFunctionNode* GenericGenerator::getFunctionNode(Variable* var)
   {
      return genericFunctionMap[var->id];
   }

   GenericObjectNode* GenericGenerator::getObjectNode(string var)
   {
       return genericObjectMap[var];
   }

   void GenericGenerator::visit(Literal* literal){}

   void GenericGenerator::visit(BinaryOperation* bop)
   {
      start_visit(bop->lhs);
      start_visit(bop->rhs);
   }

   void GenericGenerator::visit(Variable* var)
   {
      if(!var->isGeneric())
         return;
      GenericVariable* gvar = var->end()->as<GenericVariable*>();

      if(!templatedTypes.count(gvar->generic))
         panic("no generic named " + gvar->generic);
      else 
         gvar->initiate(templatedTypes[gvar->generic]);
   }

   void GenericGenerator::visit(DynamicAllocation* all)
   {

   }

   void GenericGenerator::visit(ArrayExpression* arr)
   {

   }

   void GenericGenerator::visit(ArrayIndex* arr)
   {
   }


   void GenericGenerator::visit(Assigment* assig)
   {
      start_visit(assig->expr);
   }

   void GenericGenerator::visit(If_Else* if_else)
   {

   }

   void GenericGenerator::visit(While* while_loop)
   {

   }

   void GenericGenerator::visit(For* for_loop){}

   void GenericGenerator::visit(Return* ret)
   {
      start_visit(ret->expr);
   }

   void GenericGenerator::visit(FunctionCall* fcall)
   {
      Variable* var = fcall->var->end();
      if(var->isGeneric()){
         GenericVariable* gvar = var->as<GenericVariable*>();
         gvar->initiate(templatedTypes[gvar->generic]);
         generateFunction(fcall->var,templatedTypes[gvar->generic]);
      }
   }

   void GenericGenerator::visit(StatementBlock* block)
   {
      for(auto &i : block->block)
         start_visit(i);
   }
};