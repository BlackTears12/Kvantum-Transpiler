#pragma once

#include "common/mainheader.hpp"
#include "common/token.hpp"
#include <algorithm>
#include <optional>
#include <queue>
#include <regex>
#include <memory>

using std::queue;
using std::string;
namespace kvantum::lexer
{
    class Lexer
    {
    public:
        explicit Lexer(const std::string &file_name);
        explicit Lexer(queue<Token> &toks);
        Lexer(Lexer &lexer);
        virtual ~Lexer();
        void lex();
        Token nextToken();
        Token lookAhead();
        std::optional<Token> consumeIf(Token::TokenType t);
        bool end();
        void skipUntil(vector<Token::TokenType> t);
    public:
        void printTokens();
        bool err;
        string file;

        class UnexpectedEndOfTokens : std::exception {};
    private:
        void tokenize(const std::string &line);
        void initializeRegexes();
        void setRegex(Token::TokenType t, string reg);
        bool match(const string &string, Token::TokenType &type);

        vector<std::regex *> regexes;
        std::ifstream is;
        int lineIndex;
    protected:
        queue<Token> tokens;
    };
}