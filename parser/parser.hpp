#pragma once

#include <stack>
#include <algorithm>
#include <deque>
#include <map>
#include <thread>
#include <optional>
#include <utility>
#include "common/util.hpp"
#include "lexer/scope.hpp"
#include "common/token.hpp"
#include "common/module.hpp"
#include "symbolstack.hpp"
#include "common/compiler.hpp"
#include "ast/ast.hpp"

using std::deque;
using std::map;
using std::optional;
using std::queue;
using namespace kvantum::lexer;

namespace kvantum::parser
{
    class Parser
    {
    public:
        Parser(Module& workMod);
        virtual ~Parser() = 0;

    protected:
        bool isValidType(const string& type) { return getWorkModule().hasType(type); }
        Type& getType(const string& type) { return getWorkModule().getType(type); }
        optional<Type*> parseTypeName();

        Statement* parseStatement();
        StatementBlock* parseStatementBlock();
        Assigment* parseAssigment(Variable* var);
        Statement* parseAssigment_Fcall(Variable* var);
        While* parseWhile();
        If_Else* parseIf();
        Return* parseReturn();

        /* These expression parsing methods are required by statemenent or function parsers */
        Variable* parseVariable(const Token& idToken);
        optional<FieldAccess*> parseFieldAccess(Expression* var);
        optional<FunctionCall*> parseFunctionCall(Expression* var);
        void parseArguments(vector<Expression*>& args);

        Lexer& getLexer() const { return *lexers.front(); }
        void pushLexer(Lexer& lexer) { lexers.push(&lexer); }
        void popLexer() { lexers.pop(); }

        Module& getWorkModule() const { return *workModule; }

    private:
        optional<Expression*> parseExpression();

        queue<Lexer*> lexers;
        Module* workModule;
    };
}
