#pragma once
#include "mainheader.hpp"
#include <string>

namespace kvantum {
struct Token
{
    enum TokenType {
        INTEGER,
        RATIONAL,
        BOOLEAN,
        STRING,
        NONE,
        EQUALS,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        LESS_T,
        LESS_OR_EQ_T,
        GREATER_T,
        GREATER_OR_EQ_T,
        LOG_EQUAL,
        LOG_N_EQUAL,
        AND,
        OR,
        NOT,
        L_BRACKET,
        R_BRACKET,
        LC_BRACKET,
        RC_BRACKET,
        LSQ_BRACKET,
        RSQ_BRACKET,
        DOT,
        COLON,
        COMMA,
        SEMI_COLON,
        AS,
        LET,
        ANNOTATION,
        WHILE,
        FOR,
        IF,
        ELSE,
        FUNCTION,
        TYPE,
        RETURN,
        ARROW,
        BACK_ARROW,
        DUAL_ARROW,
        AMPERSAND,
        NAMESPACE_SCOPE,
        USE,
        EXTERN,
        IDENTIFIER,
        END_OF_FILE,
        FUNCTION_CALL
    };

    Token(TokenType t, std::string str, std::string file = "", int lineIndex = 0)
    {
        type = t;
        value = str;
        this->lineIndex = lineIndex;
        filename = file;
    }
    Token()
        : lineIndex(0)
    {
        type = END_OF_FILE;
    }
    ~Token() {}

    std::string typeToString()
    {
        std::string str[] = {"INTEGER",
                             "RATIONAL",
                             "BOOLEAN",
                             "STRING",
                             "NONE",
                             "EQUALS",
                             "PLUS",
                             "MINUS",
                             "MULTIPLY",
                             "DIVIDE",
                             "LESS_THAN",
                             "LESS_OR_EQUAL_THAN",
                             "GREATER_THAN",
                             "GREATER_OR_EQUAL_THAN",
                             "LOGICAL EQUAL",
                             "LOGICAL NOT EQUAL",
                             "AND",
                             "OR",
                             "NOT",
                             "LEFT_BRACKET",
                             "RIGHT_BRACKET",
                             "LEFT_CURLY_BRACKET",
                             "RIGHT_CURLY_BRACKET",
                             "LEFT_SQUARE_BRACKET",
                             "RIGHT_SQUARE_BRACKET",
                             "DOT",
                             "COLON",
                             "COMMA",
                             "SEMI_COLON",
                             "AS",
                             "LET",
                             "ANNOTATION"
                             "WHILE",
                             "FOR",
                             "IF",
                             "ELSE",
                             "FUNCTION",
                             "TYPE",
                             "RETURN",
                             "ARROW",
                             "BACK_ARROW",
                             "DUAL_ARROW",
                             "AMPERSAND",
                             "NAMESPACE_SCOPE",
                             "USE",
                             "EXTERN",
                             "IDENTIFIER",
                             "EOF",
                             "FUNCTION_CALL"};
        return str[type];
    }

    Token as(TokenType t)
    {
        if (type != t)
            panic(typeToString() + " does not equal expected " + Token(t, "").typeToString());
        return *this;
    }

    TokenType type;
    std::string value;
    std::string filename;
    int lineIndex;
};
} // namespace kvantum
