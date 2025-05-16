#pragma once

#include "common/token.hpp"
#include "common/util.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <queue>
#include <regex>

using std::queue;
using std::string;
namespace kvantum::lexer {

class Lexer
{
public:
    class UnexpectedEndOfTokens : std::exception
    {};

    Lexer(Lexer &lexer);
    explicit Lexer(const std::string &file_name);
    explicit Lexer(queue<Token> &toks);
    virtual ~Lexer();

    Token nextToken();
    Token lookAhead();
    std::optional<Token> consumeIf(Token::TokenType t);
    void skipUntil(vector<Token::TokenType> t);
    bool end();

    void printTokens();
    constexpr bool hasError() const { return err; }
    string getFileName() const { return file; }
    string getModuleName() const { return file.substr(0, file.find('.')); }

private:
    void initializeRegexes();
    void setRegex(Token::TokenType t, string reg);

    void lex();
    void tokenize(const std::string &line);
    bool match(const string &string, Token::TokenType &type);

    vector<std::regex *> regexes;
    string file;
    std::ifstream is;
    int lineIndex;
    bool err;

protected:
    queue<Token> tokens;
};
} // namespace kvantum::lexer
