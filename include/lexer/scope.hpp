#pragma once

#include "lexer/lexer.hpp"
#include <memory>
#include <optional>
#include <deque>

using std::unique_ptr;
using std::optional;
using std::deque;

namespace kvantum::lexer
{
    /*
        contains a scope defined by the scope delimiter
    */
    class Scope : public Lexer
    {
    public:
        static optional<unique_ptr<Scope>> nextScope(Lexer *lexer, Token::TokenType begin = Token::LC_BRACKET, Token::TokenType end = Token::RC_BRACKET)
        {
            Token t = lexer->nextToken();
            while (t.type != begin) {
                panic("symbol " + t.value + " is not a scope begin operator");
                Token t = lexer->nextToken();
            }

            queue<Token> q;
            q.push(t);
            unsigned int scopeCtr = 1;
            while (scopeCtr > 0) {
                t = lexer->nextToken();
                if (t.type == Token::END_OF_FILE) {
                    panic("expected end of scope operator");
                    return {};
                }

                q.push(t);
                if (t.type == end)
                    scopeCtr--;
                if (t.type == begin)
                    scopeCtr++;
            }
            return std::make_unique<Scope>(q);
        }

        ~Scope() override = default;

        void dropScopeDelimitors()
        {
            deque<Token> inner;
            tokens.pop();
            while (tokens.size() > 1) {
                inner.push_back(tokens.front());
                tokens.pop();
            }
            tokens = std::queue<Token>(inner);
        }

        explicit Scope(queue<Token> &toks) : Lexer(toks) {}
    };
}
