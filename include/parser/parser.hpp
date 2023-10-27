#pragma once
#include <stack>
#include <algorithm>
#include <deque>
#include <map>
#include <thread>
#include <optional>
#include "common/mainheader.hpp"
#include "lexer/scope.hpp"
#include "common/token.hpp"
#include "common/module.hpp"
#include "symbolstack.hpp"
#include "common/compiler.hpp"
#include "ast/ast.hpp"
using std::deque;
using std::map;
using std::optional;
using namespace kvantum::lexer;

namespace kvantum::parser
{
   class Parser
   {
   public:
      Parser(deque<string> &includes,Compiler* owner);
      ~Parser();
      void Parse();
      
   protected:
      bool isBop(const Token &t);
      bool isValidType(string type) { return mod->hasType(type); }
      Type& getType(string type) { return mod->getType(type); }

      void* expressionPanic(Token &t);
      void parseArguments(vector<Expression*> &args);
      void addFunction(FunctionNode* func);

      void parseExternalDependency();
      Variable* parseVariable(const Token &idToken);
      FunctionCall* parseFunctionCall(Variable* const var);
      
      optional<Type*> parseType();
      optional<ArrayIndex*> parseArrayIndex(Expression* basear);
      optional<FieldAccess*> parseFieldAccess(Expression* var);
      optional<BinaryOperation*> parseBop(optional<Expression*> lhs);
      optional<Expression*> parseExpression();
      optional<FunctionCall*> parseListExpression();
      vector<Literal*> parseArrayInitializer(Token::TokenType beg,Token::TokenType end);
      ///arrayexpressions are packed into a [].new cctor call
      optional<ArrayExpression*> parseArrayExpression();
   protected:
      //Statement parseing
      Statement* parseStatement();
      StatementBlock* parseStatementBlock();
      Statement* parseAssigment_Fcall(Variable* var);
      Statement* parseAssigment(Variable* var);
      Statement* parseWhile();
      Statement* parseIf();
      Statement* parseReturn();

   protected:
      deque<string>& includes;
      Lexer* lexer;
      Module* mod;
      Compiler* owner;
   private:
      void parseFunctionDefinition();
      void parseTypeDefinition();
      void parseFile(Lexer* lexer);
   };
}