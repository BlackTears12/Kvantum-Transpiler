#ifndef EXPRESSIONPARSER_HPP
#define EXPRESSIONPARSER_HPP

#include "parser.hpp"

namespace kvantum::parser {

class ExpressionParser : public Parser
{
public:
    ExpressionParser(Lexer& lexer, Module& workMod);

    optional<Expression*> parseExpression();

private:
    bool isBop(const Token& t);
    bool isLiteral(const Token& t);

    void* expressionPanic(const Token& t);

    optional<Cast*> parseCast(Expression* base);
    optional<ArrayIndex*> parseArrayIndex(Expression* basearr);
    optional<BinaryOperation*> parseBop(optional<Expression*> lhs);
    optional<FunctionCall*> parseListExpression();
    vector<Literal*> parseArrayInitializer(Token::TokenType beg, Token::TokenType end);
    ///arrayexpressions are packed into a [].new cctor call
    optional<ArrayExpression*> parseArrayExpression();

    Expression* toPrefixForm(Expression* infix) const;
};

} // namespace kvantum::parser

#endif // EXPRESSIONPARSER_HPP
