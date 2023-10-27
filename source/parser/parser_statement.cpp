#include "parser/parser.hpp"
namespace kvantum::parser
{
    /*
        <statement> := <assignment> | <return> | <s_function_call> | <if_else> | <while> | <statement_block>
        <assignment> := <identifier> := <expression>;
        <return> := ret <expression>;
        <s_function_call> := <function_call>;
        <if_else> := if <expression>: <statement> <else>?
        <else> := else <statement>
        <while> := while <expression>: <statement>
        <statement_block> := { <statement>... }
    */
   Statement* Parser::parseStatement()
   {
      std::queue<Token> errq;
      Statement* valid = nullptr;
      while(!valid && !lexer->end())
      {
         Token token = lexer->nextToken();
         switch (token.type)
         {
         case Token::RETURN:
            valid = parseReturn();
            break;
         case Token::IDENTIFIER:
            valid = parseAssigment_Fcall(parseVariable(token));
            break;
         case Token::IF:
            valid = parseIf();
            break;
         case Token::WHILE:
            valid = parseWhile();
            break;
         case Token::LC_BRACKET:
            valid = parseStatementBlock();
            break;
         case Token::END_OF_FILE:
            valid = new StatementBlock();
            break;
         default:
            errq.push(token);
            break;
         }
      }
      Token token;
      string err = "";
      while(!errq.empty()){
         token = errq.front();
         err += token.value;
         errq.pop();
      }
      if(err != "")
         panic(err+" is not a valid statement");
      return valid;
   }

   StatementBlock* Parser::parseStatementBlock()
   {
      StatementBlock* block = new StatementBlock();
      while(!lexer->end() && lexer->lookAhead().type != Token::RC_BRACKET){
         Statement* st = parseStatement();
         if (st)
             block->block.push_back(st);
         else if (lexer->end())
             break;
      }
      if (!lexer->end())
          lexer->nextToken();
      return block;
   }

   Statement* Parser::parseAssigment_Fcall(Variable* var)
   {
      if(lexer->lookAhead().type == Token::L_BRACKET){
         auto fcall = parseFunctionCall(var);
         if(!lexer->consumeIf(Token::SEMI_COLON).has_value())
            panic("no semi colon at the end of function call");
         return fcall;
      }
      else if(lexer->lookAhead().type == Token::EQUALS)
         return parseAssigment(var);
      panic("missing assignment operator or brackets");
      lexer->skipUntil({Token::SEMI_COLON});
      return nullptr;
   }

   Statement* Parser::parseAssigment(Variable* var){
      Token nexttoken = lexer->nextToken().as(Token::EQUALS);
      auto expr = parseExpression();
      if(!expr.has_value()){
         lexer->skipUntil({Token::SEMI_COLON});
         return nullptr;
      }
      
      if(!lexer->consumeIf(Token::SEMI_COLON).has_value())
         panic("no semi colon at the end of assignment");
         
      //if a val is already allocated return the assignment
      return new Assigment(var,expr.value());
   }

   Statement* Parser::parseReturn()
   {
      auto exp = parseExpression();
      if(!exp.has_value())
         lexer->skipUntil({Token::SEMI_COLON});
      if(!lexer->consumeIf(Token::SEMI_COLON).has_value())
         panic("no semi colon at the end of assignment");
      return new Return(exp.value_or(nullptr));
   }

   Statement* Parser::parseWhile(){
      auto exp = parseExpression();
      if(!exp.has_value())
         lexer->skipUntil({Token::L_BRACKET});
      While* wh = new While(exp.value_or(nullptr));
      if(!lexer->consumeIf(Token::COLON).has_value())
         panic("no colon for while statement");
      wh->block = parseStatement();
      return wh;
   }
  
   Statement* Parser::parseIf(){
      auto exp = parseExpression();
      if(!exp.has_value())
         lexer->skipUntil({Token::L_BRACKET});

      If_Else* ife = new If_Else(exp.value_or(nullptr));
      if(!lexer->consumeIf(Token::COLON).has_value())
         panic("no colon for if statement");

      ife->ifBlock = parseStatement();
      if(!lexer->end() && lexer->lookAhead().type == Token::ELSE){
         lexer->nextToken().as(Token::ELSE);
         if (!lexer->end()) {
            lexer->nextToken().as(Token::COLON);
            ife->elseBlock = parseStatement();
         }
      }
      else ife->elseBlock = nullptr;
      return ife;
   }
}