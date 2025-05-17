#include "expressionparser.hpp"

namespace kvantum::parser {

ExpressionParser::ExpressionParser(Lexer &lexer, Module &workMod)
    : Parser(workMod)
{
    pushLexer(lexer);
}

/*
    expression := <variable> | <fcall> | <literal> | <array> | <field_access> |<bop> | <arr_index> | <take_reference> | <cast>
    variable := <identifier> | <field_access>
    fcall := <identifier>(<expression>,...)
    literal := <integer> | <float> | <string> | <boolean>
    field_access := <identifier>.<identifier>
    arr_index := <expression>[<expression>]
    take_reference := &<expression>
    cast := <expression> as <typename>
    bop := <expression> <binary_operator> <expression>
    binary_operator := + | - | * | - | == | != | and | or
*/
optional<Expression *> ExpressionParser::parseExpression()
{
    Token t = getLexer().lookAhead();
    optional<Expression *> this_expr = nullptr;

    //if its a primitve type literal
    if (isLiteral(t)) {
        t = getLexer().nextToken();
        if (t.type == Token::STRING) {
            ///trim the quotes
            t.value.pop_back();
            t.value.erase(0, 1);
        }
        this_expr = new Literal(t.value,
                                PrimitiveType::get(static_cast<PrimitiveType::TypeBase>(t.type)));
    } else
        switch (t.type) {
            CASE(Token::IDENTIFIER, this_expr = parseVariable(getLexer().nextToken()));
            CASE(Token::NONE,
                 this_expr = new Literal(getLexer().nextToken().value, ObjectType::getObject()));
            CASE(Token::LSQ_BRACKET, this_expr = parseListExpression());
            CASE(Token::LESS_T, this_expr = parseArrayExpression());
            CASE(Token::AMPERSAND, getLexer().nextToken();
                 this_expr = new TakeReference(parseExpression().value_or(nullptr)));
        default:
            expressionPanic(getLexer().nextToken());
            return {};
        }

    while (true) {
        if (isBop(getLexer().lookAhead()))
            this_expr = parseBop(this_expr);
        switch (getLexer().lookAhead().type) {
            CASE(Token::DOT, this_expr = parseFieldAccess(this_expr.value_or(nullptr)));
            CASE(Token::L_BRACKET, this_expr = parseFunctionCall(this_expr.value_or(nullptr)));
            CASE(Token::LSQ_BRACKET, this_expr = parseArrayIndex(this_expr.value_or(nullptr)););
            CASE(Token::AS, this_expr = parseCast(this_expr.value_or(nullptr)));
        default:
            return this_expr;
        }
    }
}

bool ExpressionParser::isBop(const Token &t)
{
    vector<Token::TokenType> bops{Token::PLUS,
                                  Token::MINUS,
                                  Token::MULTIPLY,
                                  Token::DIVIDE,
                                  Token::LOG_EQUAL,
                                  Token::LOG_N_EQUAL,
                                  Token::AND,
                                  Token::OR,
                                  Token::GREATER_T,
                                  Token::LESS_T};
    return std::find(bops.begin(), bops.end(), t.type) != bops.end();
}

bool ExpressionParser::isLiteral(const kvantum::Token &t)
{
    return t.type < 4;
}

void *ExpressionParser::expressionPanic(const Token &t)
{
    KVANTUM_VERIFY(isBop(t), t.value + " is not a valid expression");
    else panic("missing left hand side of binary operation");
    return nullptr;
}

optional<Cast *> ExpressionParser::parseCast(Expression *base)
{
    getLexer().nextToken().as(Token::AS);
    auto type = parseTypeName();
    if (!type.has_value())
        return {};
    KVANTUM_VERIFY_RETURN(!type.value()->isVoid(), "cannot cast to Void", {});
    return new Cast(base, *type.value());
}

optional<ArrayIndex *> ExpressionParser::parseArrayIndex(Expression *basearr)
{
    getLexer().nextToken().as(Token::LSQ_BRACKET); // Assuming lexer is a member pointer
    auto ind = parseExpression();
    if (!ind.has_value())
        return {};
    getLexer().nextToken().as(Token::RSQ_BRACKET); // Assuming lexer is a member pointer
    return new ArrayIndex(basearr, ind.value());
}

optional<BinaryOperation *> ExpressionParser::parseBop(optional<Expression *> lhs)
{
    Token t = getLexer().nextToken();
    auto rhs = parseExpression();

    if (!lhs.has_value() || !rhs.has_value())
        return {};
    vector<Token::TokenType> bops{Token::PLUS,
                                  Token::MINUS,
                                  Token::MULTIPLY,
                                  Token::DIVIDE,
                                  Token::LOG_EQUAL,
                                  Token::LOG_N_EQUAL,
                                  Token::LESS_T,
                                  Token::LESS_OR_EQ_T,
                                  Token::GREATER_T,
                                  Token::GREATER_OR_EQ_T,
                                  Token::AND,
                                  Token::OR};
    BinaryOperation::Operator op = (BinaryOperation::Operator)
        std::distance(bops.begin(), std::find(ITER_THROUGH(bops), t.type));
    return new BinaryOperation(lhs.value(), rhs.value(), op);
}

optional<FunctionCall *> ExpressionParser::parseListExpression()
{
    auto scope = Scope::nextScope(getLexer(), Token::LSQ_BRACKET, Token::RSQ_BRACKET);
    if (!scope.has_value())
        return {};
    pushLexer(*scope.value());
    auto init = parseArrayInitializer(
        Token::LSQ_BRACKET,
        Token::RSQ_BRACKET); // This call was inside the pushLexer block, moved outside based on typical scope handling. Revert if original logic is intended.
    Type &t = Type::get("Void");
    if (!init.empty())
        t = init[0]->getType();

    ///if its the first time a list with the specifie type has beed initiated add to the type pool
    if (!getWorkModule().hasType(ListType::get(t).getName()))
        getWorkModule().addObjectType(&ListType::get(t));

    popLexer(); // Pop after processing the scope

    ///pack the arrayexpression into a [].new call and provide the array body and size as arguments
    return new FunctionCall(new FieldAccess(new Variable("[]", ListType::get(t)),
                                            new Variable("new")),
                            {new ArrayExpression(t, init),
                             new Literal(std::to_string(init.size()), Type::get("Int"))},
                            ListType::get(t).getFunction("new"));
}

vector<Literal *> ExpressionParser::parseArrayInitializer(Token::TokenType beg, Token::TokenType end)
{
    auto scope = Scope::nextScope(getLexer(), beg, end);
    if (!scope.has_value())
        return {};
    pushLexer(*scope.value());
    vector<Literal *> init;
    try {
        scope.value()->dropScopeDelimitors();
        while (!getLexer().end()) {
            Token t = getLexer().lookAhead();
            auto expr = parseExpression();
            if (!getLexer().end() && getLexer().lookAhead().type == Token::COMMA)
                getLexer().nextToken();
            if (expr.has_value()) {
                KVANTUM_VERIFY(expr.value()->exprtype == ExprType::LITERAL,
                               "array initialzer can only be literal value");
                else init.push_back(expr.value()->as<Literal *>());
            }
        }
    } catch (Lexer::UnexpectedEndOfTokens &) {
        popLexer();
        panic("invalid array initializer");
    }
    popLexer();
    return init;
}

optional<ArrayExpression *> ExpressionParser::parseArrayExpression()
{
    auto init = parseArrayInitializer(Token::LESS_T, Token::GREATER_T);
    Type *t = &Type::get("Void");
    if (!init.empty())
        t = &init[0]->getType();

    return new ArrayExpression(*t, init);
}

Expression *ExpressionParser::toPrefixForm(Expression *infix) const {}

} // namespace kvantum::parser
