#include "parser/functiondefparser.hpp"

namespace kvantum::parser
{
   FunctionDefParser::FunctionDefParser(deque<string> &d,Lexer* lexer,Module* m,Compiler* owner,Annotation* an) : Parser(d,owner)
   {
      this->node = nullptr;
      this->lexer = lexer;
      mod = m;
   }
   
   Variable* FunctionDefParser::parseFunctionIdentifier()
   {
      Variable* var = parseVariable(lexer->nextToken().as(Token::IDENTIFIER));
      node = new FunctionNode(var->id);
      if (var->isField()) {
          auto base = var->as<FieldAccess*>()->base->as<Variable*>();
          if (var->as<FieldAccess*>()->field->id == "new")
              node->setTrait(FunctionNode::STATIC);
          KVANTUM_VERIFY(mod->hasType(base->id),"no type named "+base->id);
          else {
            base->setType(mod->getType(base->id));
            node->makeMethod(mod->getType(base->id));
          }
      }
      return var;
   }

   FunctionNode* FunctionDefParser::parseFunctionDecl()
   {
      lexer->nextToken().as(Token::FUNCTION);
      Variable* id = parseFunctionIdentifier();
      
      if(false)
         parseFunctional(node);
      else{
         parseFormalParams(node->formalParams);
         if(lexer->lookAhead().type == Token::DUAL_ARROW)
            parseConstant(node);
         else parseNormal(node);
      }
      return node;
   }

   void FunctionDefParser::parseFormalParams(vector<Variable*> &params)
   {
       Lexer* Origlexer = this->lexer;
       auto scope = Scope::nextScope(Origlexer, Token::L_BRACKET, Token::R_BRACKET);
       if (!scope.has_value())
           return;

      this->lexer = scope.value().get();

      try {
          ///parse the func definition
          Token t = lexer->nextToken().as(Token::L_BRACKET);
          while (!lexer->end() && lexer->lookAhead().type != Token::R_BRACKET)
          {
              Token name = lexer->nextToken().as(Token::IDENTIFIER);
              lexer->nextToken().as(Token::COLON);
              auto typeOpt = parseType();
              auto type = typeOpt.value_or(&Type::get("Void"));
              KVANTUM_VERIFY(*type != Type::get("Void"),"parameter cannot have Void type");
              params.push_back(new Variable(name.value,*type));

              if (lexer->lookAhead().type == Token::COMMA)
                  t = lexer->nextToken();
          }
          if (!lexer->end())
              lexer->nextToken();
      }
      ///error while parsing the function
      catch (Lexer::UnexpectedEndOfTokens&) {
          panic("Unexpected end of function definition");
      }
      this->lexer = Origlexer;
   }

   void FunctionDefParser::parseFunctional(FunctionNode* node)
   {

   }

   void FunctionDefParser::parseConstant(FunctionNode* node)
   {
      lexer->nextToken().as(Token::DUAL_ARROW);
      auto exp = parseExpression();
      lexer->skipUntil({Token::SEMI_COLON});
      node->ast.push_back(new Return(exp.value_or(nullptr)));
   }

   void FunctionDefParser::parseNormal(FunctionNode* node)
   {
      setTraitList(node);
      if (lexer->lookAhead().type == Token::ARROW) {
          lexer->nextToken();
          auto type = parseType();
          node->setReturnType(*type.value_or(&Type::get("Void")));

          bool err = lexer->lookAhead().type != Token::LC_BRACKET;
          while (lexer->lookAhead().type != Token::LC_BRACKET)
              lexer->nextToken();
          if (err)
              panic("not a valid function signiture");
          node->setTrait(FunctionNode::EXPLICIT_TYPE);
      }

      Lexer* Origlexer = this->lexer;
      auto scope = Scope::nextScope(Origlexer, Token::LC_BRACKET, Token::RC_BRACKET);
      if (!scope.has_value())
          return;
      this->lexer = scope.value().get();

      lexer->nextToken().as(Token::LC_BRACKET);
      node->ast = parseStatementBlock()->block;

      this->lexer = Origlexer;
   }

   void FunctionDefParser::setTraitList(FunctionNode* node) 
   {
       if (lexer->end() || lexer->lookAhead().type != Token::LSQ_BRACKET)
           return;
       auto scope = Scope::nextScope(lexer,Token::LSQ_BRACKET,Token::RSQ_BRACKET);
       KVANTUM_VERIFY_ABANDON(scope.has_value(),"invalid trait list");

       auto lex = scope.value().get();

       try {
           lex->nextToken();
           while (!lex->end() && lex->lookAhead().type != Token::RSQ_BRACKET) {
               Token next = lex->nextToken().as(Token::IDENTIFIER);
               if (next.value == "public")
                   node->setTrait(FunctionNode::PUBLIC);
               else if (next.value == "const")
                   node->setTrait(FunctionNode::CONST);
               else if (next.value == "static")
                   node->setTrait(FunctionNode::STATIC);
               else if (next.value == "virtual")
                   node->setTrait(FunctionNode::VIRTUAL);
               else if (next.value == "override")
                   node->setTrait(FunctionNode::OVERRIDE);
               else panic("unknown trait " + next.value);

               if (!lex->end() && lex->lookAhead().type == Token::COMMA)
                   lex->nextToken();
           }
       }
       catch (Lexer::UnexpectedEndOfTokens&) {
           panic("Unexpected end of trait list");
       }
   }
}