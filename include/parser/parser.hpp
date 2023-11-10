#pragma once

#include <stack>
#include <algorithm>
#include <deque>
#include <map>
#include <thread>
#include <optional>
#include <utility>
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
        Parser(deque<string> &includes, Compiler* owner);
        ~Parser() = default;
        void Parse();

    protected:
        bool isBop(const Token &t);
        bool isLiteral(const Token &t);
        bool isValidType(string type) { return mod->hasType(std::move(type)); }
        Type &getType(string type) { return mod->getType(std::move(type)); }

        void* expressionPanic(const Token& t);
        void parseArguments(vector<Expression*> &args);
        void addFunction(FunctionNode* func);

        void parseExternalDependency();
        Variable* parseVariable(const Token &idToken);
        optional<FunctionCall*> parseFunctionCall(Expression* var);

        optional<Type*> parseType();
        optional<Cast*> parseCast(Expression* base);
        optional<ArrayIndex*> parseArrayIndex(Expression* basearr);
        optional<FieldAccess*> parseFieldAccess(Expression* var);
        optional<BinaryOperation*> parseBop(optional<Expression*> lhs);
        optional<Expression*> parseExpression();
        optional<FunctionCall*> parseListExpression();
        vector<Literal*> parseArrayInitializer(Token::TokenType beg, Token::TokenType end);
        ///arrayexpressions are packed into a [].new cctor call
        optional<ArrayExpression*> parseArrayExpression();
    protected:
        //Statement parseing
        Statement* parseStatement();
        StatementBlock* parseStatementBlock();
        Assigment* parseAssigment(Variable* var);
        Statement* parseAssigment_Fcall(Variable* var);
        While* parseWhile();
        If_Else* parseIf();
        Return* parseReturn();

    protected:
        deque<string> &includes;
        Lexer* lexer;
        Module* mod;
        Compiler* owner;
        Annotation* annotation;
    private:
        void parseFunctionDefinition();
        void parseTypeDefinition();
        void parseFile(Lexer* lexer);
    };
}