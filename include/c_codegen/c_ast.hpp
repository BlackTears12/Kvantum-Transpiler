#pragma once
#include "c_type.hpp"

#include <iostream>
namespace c::ast
{
   struct Expression
   {
      virtual string getStr() = 0;
      virtual Type* getType() = 0;
   };

   struct Literal : public Expression
   {
      Literal(string v,Type* t)
      {
         value = v;
         type = t;
      }

      string getStr() override
      {
         return value;
      }

      Type* getType()
      {
          return type;
      }

      Type* type;
      string value;
   };

   struct Variable : public Expression
   {
      Variable(string name,Type* t)
      {
         this->name = name;
         type = t;
      }

      string getStr() override
      {
         return name;
      }

      Type* getType()
      {
          return type;
      }

      string name;
      Type* type;
   };

   struct FieldAccess : public Expression
   {
       FieldAccess(Expression* e,Variable* f)
       {
           base = e;
           field = f;
       }

       string getStr() override
       {
           return base->getStr() + (base->getType()->isPtr() ? "->" : ".") + field->getStr();
       }

       Type* getType()
       {
           return field->getType();
       }

       Expression* base;
       Variable* field;
   };

   struct BinaryOperation : public Expression
   {
      BinaryOperation(Expression* r,Expression* l,string op) : operand(op)
      {
         lhs = l;
         rhs = r;
      }

      string getStr() override
      {
         return lhs->getStr() + operand + rhs->getStr();
      }

      Type* getType()
      {
          if (operand == "==" || operand == "!=" || operand == "<" || operand == "<=" || operand == ">=" || operand == ">")
              return Type::getInt8();
          else return lhs->getType();
      }

      string operand;
      Expression* lhs;
      Expression* rhs;
   };

   struct ArrayExpression : public Expression
   {
       ArrayExpression(vector<Literal*> init) : initializer(init){}

       string getStr() override
       {
           string str = "";
           for (auto& e : initializer)
               str += e->getStr() + ",";
           if (!str.empty())
               str.pop_back();

           return "{" + str + "}";
       }

       Type* getType() override
       {
           return Type::getPointer(getSubType());
       }

       Type* getSubType()
       {
           return initializer.empty() ? Type::getVoid() : initializer[0]->getType();
       }

       vector<Literal*> initializer;
   };

   struct ArrayIndex : public Expression
   {
       ArrayIndex(Expression* expr,Variable* ind)
       {
           arrayExpr = expr;
           index = ind;
       }

       string getStr() override
       {
           return arrayExpr->getStr() + "[" + index->getStr() + "]";
       }

       Type* getType() override
       {
           return static_cast<Pointer*>(arrayExpr->getType())->getReferencedType();
       }

       Expression* arrayExpr;
       Variable* index;
   };

   struct Statement
   {
      virtual string getStr() = 0;
   };
   struct Block : public Statement
   {
      vector<Statement*> statements;

      string getStr() override
      {
         string str = "{\n";
         for(auto &e : statements){
            str += e->getStr() + ";\n";
         }
         str += "}\n";
         return str;
      }

      void insert(Statement* s)
      {
         statements.push_back(s);
      };
   };

   struct Assigment : public Statement
   {
      Assigment(Variable* var,Expression* expr,bool decl)
      {
         this->var = var;
         this->expr = expr;
         this->decl = decl;
      }

      string getStr() override {return (decl ? var->getType()->getStr() : string("")) + string(" ") + var->getStr() + " = " + expr->getStr(); }

      Variable* var;
      Expression* expr;
      bool decl;
   };

   struct VariableDeclaration : public Statement
   {
      VariableDeclaration(Variable* v)
      {
         var = v;
      }
      string getStr() override { return var->type->getStr() + string(" ") +var->name; }
      Variable* var;
   };

   struct Return : public Statement
   {
      Return(Expression* e){ expr = e; }
      string getStr() override { return "return " + expr->getStr(); }

      Expression* expr;
   };

   struct IfElse : public Statement
   {
      IfElse(Expression* cond,Statement* ifb,Statement* elseb)
      {
         condition = cond;
         this->ifb = ifb;
         this->elseb = elseb;
      }

      string getStr() override 
      {
         string r = "if(" + condition->getStr()+")" + "\n" + ifb->getStr() 
            + (elseb ? elseb->getStr() : "");
      }

      Expression* condition;
      Statement* ifb;
      Statement* elseb;
   };

   struct Function
   {
      Function(string name,Type* ret = Type::getVoid(),bool variadric = false)
      {
         this->name = name;
         block = new Block();
         returnType = ret;
         this->variadric = variadric;
      }

      string getHeader()
      {
         string str = returnType->getStr() + " " +name + "(";
         for(auto &e : formalParams)
            str += e->type->getStr() + " " + e->name + ",";
         if(!formalParams.empty())
            str.pop_back();
         str += ")";
         return str;
      }

      string getPrototype() { return getHeader() + ";"; }
      string getDefiniton(){ return getHeader() + "\n" + block->getStr(); }

      string name;
      vector<Variable*> formalParams;
      Block* block;
      Type* returnType;
      bool variadric;
   };

   struct Struct 
   {
      Struct(string n,vector<Variable*> f = {}): name(n),fields(f){}
      string getHeader() { return "struct " + name; }
      string getPrototype(){ return getHeader() + ";"; }

      string getDefinition()
      {
         string str = getHeader() + "\n{\n";
         for(auto &e : fields)
            str += e->type->getStr() + " " + e->getStr() + ";\n";
         return str+"};\n";
      }

      string name;
      vector<Variable*> fields;
   };

   struct FunctionCall : public Expression,public Statement
   {
   public:
      FunctionCall(Function* func,vector<Expression*> args = {}) : arguments(args)
      {
         this->func = func;
      }

      string getStr() override
      {
         string str = func->name + "(";
         for(auto &e : arguments)
            str += e->getStr() + ",";
         if(!arguments.empty())
            str.pop_back();
         str += ")";
         return str;
      }

      Type* getType()
      {
          return func->returnType;
      }

      Function* func;
      vector<Expression*> arguments;
   };
}