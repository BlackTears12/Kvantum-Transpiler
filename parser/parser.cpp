#include "parser/parser.hpp"
#include "parser/expressionparser.hpp"
#include "parser/functiondefparser.hpp"

namespace kvantum::parser {

Parser::Parser(Module& workMod)
    : workModule(&workMod)
{}

optional<Type*> Parser::parseTypeName()
{
    if (getLexer().lookAhead().type == Token::LSQ_BRACKET) {
        getLexer().nextToken();
        auto type = parseTypeName();
        getLexer().nextToken().as(Token::RSQ_BRACKET);
        return &ListType::get(*type.value_or(&Type::get("Void")));
    }

    if (getLexer().lookAhead().type == Token::LESS_T) {
        getLexer().nextToken();
        auto type = parseTypeName();
        getLexer().nextToken().as(Token::GREATER_T);
        return &ArrayType::get(*type.value_or(&Type::get("Void")));
    }

    bool isRef = getLexer().lookAhead().type == Token::AMPERSAND;
    if (isRef) {
        while (getLexer().lookAhead().type == Token::AMPERSAND) {
            getLexer().nextToken();
            Diagnostics::warn("multiple references");
        }
    }

    auto tok = getLexer().nextToken().as(Token::IDENTIFIER);
    if (!getWorkModule().hasType(tok.value)) {
        panic("no type named " + tok.value);
        return {};
    }
    Type* type = &getWorkModule().getType(tok.value);

    if (isRef)
        return &ReferenceType::get(*type);
    return type;
}

Statement* Parser::parseStatement()
{
    std::queue<Token> errq;
    Statement* valid = nullptr;
    while (!valid && !getLexer().end()) {
        Token token = getLexer().nextToken();
        switch (token.type) {
            CASE(Token::RETURN, valid = parseReturn());
            CASE(Token::LET,
                 valid = parseAssigment(parseVariable(getLexer().nextToken()))->setDeclaration(true));
            CASE(Token::IDENTIFIER, valid = parseAssigment_Fcall(parseVariable(token)));
            CASE(Token::IF, valid = parseIf());
            CASE(Token::WHILE, valid = parseWhile());
            CASE(Token::LC_BRACKET, valid = parseStatementBlock());
            CASE(Token::END_OF_FILE, valid = new StatementBlock());
        default:
            errq.push(token);
            break;
        }
    }
    Token token;
    string err = "";
    while (!errq.empty()) {
        token = errq.front();
        err += token.value;
        errq.pop();
    }
    if (err != "")
        panic(err + " is not a valid statement");
    return valid;
}

StatementBlock* Parser::parseStatementBlock()
{
    StatementBlock* block = new StatementBlock();
    while (!getLexer().end() && getLexer().lookAhead().type != Token::RC_BRACKET) {
        Statement* st = parseStatement();
        if (st)
            block->block.push_back(st);
        else if (getLexer().end())
            break;
    }
    if (!getLexer().end())
        getLexer().nextToken();
    return block;
}

Assigment* Parser::parseAssigment(Variable* var)
{
    Type* assignTy = &Type::get("Void");
    if (getLexer().lookAhead().type == Token::COLON) {
        getLexer().nextToken();
        assignTy = parseTypeName().value_or(&Type::get("Void"));
    }

    Token nexttoken = getLexer().nextToken().as(Token::EQUALS);
    auto expr = parseExpression();
    if (!expr.has_value()) {
        getLexer().skipUntil({Token::SEMI_COLON});
        return nullptr;
    }

    if (!getLexer().consumeIf(Token::SEMI_COLON).has_value())
        panic("no semi colon at the end of assignment");

    var->setType(*assignTy);
    return new Assigment(var, expr.value());
}

Statement* Parser::parseAssigment_Fcall(Variable* var)
{
    if (getLexer().lookAhead().type == Token::L_BRACKET) {
        auto fcall = parseFunctionCall(var);
        if (!getLexer().consumeIf(Token::SEMI_COLON).has_value())
            panic("no semi colon at the end of function call");
        return fcall.value();
    } else if (getLexer().lookAhead().type == Token::EQUALS)
        return parseAssigment(var);
    panic("missing assignment operator or brackets");
    getLexer().skipUntil({Token::SEMI_COLON});
    return nullptr;
}

While* Parser::parseWhile()
{
    auto exp = parseExpression();
    if (!exp.has_value())
        getLexer().skipUntil({Token::L_BRACKET});
    While* wh = new While(exp.value_or(nullptr));
    if (!getLexer().consumeIf(Token::COLON).has_value())
        panic("no colon for while statement");
    wh->block = parseStatement();
    return wh;
}

If_Else* Parser::parseIf()
{
    auto exp = parseExpression();
    if (!exp.has_value())
        getLexer().skipUntil({Token::L_BRACKET});

    If_Else* ife = new If_Else(exp.value_or(nullptr));
    if (!getLexer().consumeIf(Token::COLON).has_value())
        panic("no colon for if statement");

    ife->ifBlock = parseStatement();
    if (!getLexer().end() && getLexer().lookAhead().type == Token::ELSE) {
        getLexer().nextToken().as(Token::ELSE);
        if (!getLexer().end()) {
            getLexer().nextToken().as(Token::COLON);
            ife->elseBlock = parseStatement();
        }
    } else
        ife->elseBlock = nullptr;
    return ife;
}

Return* Parser::parseReturn()
{
    auto exp = parseExpression();
    if (!exp.has_value())
        getLexer().skipUntil({Token::SEMI_COLON});
    if (!getLexer().consumeIf(Token::SEMI_COLON).has_value())
        panic("no semi colon at the end of assignment");
    return new Return(exp.value_or(nullptr));
}

/*
        <statement> := <assignment> | <return> | <s_function_call> | <if_else> | <while> | <statement_block> ;
        <assignment> := let <identifier> := <expression>;
        <return> := ret <expression>;
        <s_function_call> := <function_call>;
        <if_else> := if <expression>: <statement> <else>?
        <else> := else <statement>
        <while> := while <expression>: <statement>
        <statement_block> := { <statement>... }
    */
Variable* Parser::parseVariable(const Token& idToken)
{
    Variable* var = new Variable(idToken.value);
    if (getLexer().lookAhead().type != Token::DOT)
        return var;

    auto field = parseFieldAccess(var);
    if (!field.has_value())
        return var;
    else
        return field.value();
}

optional<FieldAccess*> Parser::parseFieldAccess(Expression* var)
{
    getLexer().nextToken().as(Token::DOT);
    if (!getLexer().end() && getLexer().lookAhead().type == Token::IDENTIFIER)
        return new FieldAccess(var, parseVariable(getLexer().nextToken()));
    panic("no identifier after .");
    return {};
}

optional<FunctionCall*> Parser::parseFunctionCall(Expression* var)
{
    KVANTUM_VERIFY_RETURN(var && var->exprtype == ExprType::VARIABLE,
                          "invalid function identifier",
                          {});
    auto* call = new FunctionCall(var->as<Variable*>(), {});
    // The original code stored and restored a 'lexer' pointer.
    // With getLexer() returning a reference and pushLexer/popLexer
    // likely managing an internal lexer stack/pointer, this saving
    // and restoring of 'lex' might need adjustment depending on
    // the exact implementation of pushLexer/popLexer and getLexer().
    // Assuming pushLexer/popLexer handle the active lexer correctly:
    parseArguments(call->arguments);
    // No explicit lexer restoration needed here if push/pop manage the active one

    return call;
}

void Parser::parseArguments(vector<Expression*>& args)
{
    auto scope = Scope::nextScope(getLexer(), Token::L_BRACKET, Token::R_BRACKET);
    if (!scope.has_value())
        return;
    pushLexer(*scope.value());
    try {
        Token t = getLexer().nextToken().as(Token::L_BRACKET);
        t = getLexer().lookAhead();

        while (t.type != Token::R_BRACKET) {
            args.push_back(parseExpression().value_or(nullptr));

            t = getLexer().lookAhead();
            if (t.type == Token::COMMA)
                t = getLexer().nextToken();
        }
        getLexer().nextToken();
    } catch (Lexer::UnexpectedEndOfTokens&) {
        panic("Unexpected end of argument list");
    }
    popLexer();
}

optional<Expression*> Parser::parseExpression()
{
    ExpressionParser expr(getLexer(), getWorkModule());
    return expr.parseExpression();
}
} // namespace kvantum::parser
