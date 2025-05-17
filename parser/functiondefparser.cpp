#include "parser/functiondefparser.hpp"

#include "expressionparser.hpp"

namespace kvantum::parser {

FunctionDefParser::FunctionDefParser(Lexer &lexer,
                                     Module &workMod,
                                     vector<Annotation *> &annotations)
    : Parser(workMod)
{
    pushLexer(lexer);
}

Variable *FunctionDefParser::parseFunctionIdentifier()
{
    Variable *var = parseVariable(getLexer().nextToken().as(Token::IDENTIFIER));
    node = std::make_unique<FunctionNode>(var->id);
    if (var->isField()) {
        auto base = var->as<FieldAccess *>()->base->as<Variable *>();
        if (var->as<FieldAccess *>()->field->id == "new")
            node->setTrait(FunctionNode::STATIC);
        KVANTUM_VERIFY(getWorkModule().hasType(base->id), "no type named " + base->id);
        else
        {
            base->setType(getWorkModule().getType(base->id));
            node->makeMethod(getWorkModule().getType(base->id));
        }
    }
    return var;
}

unique_ptr<FunctionNode> FunctionDefParser::parseFunctionDefinition()
{
    getLexer().nextToken().as(Token::FUNCTION);
    Variable *id = parseFunctionIdentifier();

    /* TODO implement functional */
    if (false)
        parseFunctional(node.get());
    else {
        parseFormalParams(node->formalParams);
        if (getLexer().lookAhead().type == Token::DUAL_ARROW)
            parseConstant(node.get());
        else
            parseNormal(node.get());
    }
    return std::move(node);
}

void FunctionDefParser::parseFormalParams(vector<Variable *> &params)
{
    auto scope = Scope::nextScope(getLexer(), Token::L_BRACKET, Token::R_BRACKET);
    if (!scope.has_value())
        return;

    pushLexer(*scope.value());

    try {
        /// parse the func definition
        Token t = getLexer().nextToken().as(Token::L_BRACKET);
        while (!getLexer().end() && getLexer().lookAhead().type != Token::R_BRACKET) {
            Token name = getLexer().nextToken().as(Token::IDENTIFIER);
            getLexer().nextToken().as(Token::COLON);
            auto typeOpt = parseTypeName();
            auto type = typeOpt.value_or(&Type::get("Void"));
            KVANTUM_VERIFY(*type != Type::get("Void"), "parameter cannot have Void type");
            params.push_back(new Variable(name.value, *type));

            if (getLexer().lookAhead().type == Token::COMMA)
                t = getLexer().nextToken();
        }
        if (!getLexer().end())
            getLexer().nextToken();
    }
    /// error while parsing the function
    catch (Lexer::UnexpectedEndOfTokens &) {
        popLexer();
        panic("Unexpected end of function definition");
    }
    popLexer();
}

void FunctionDefParser::parseFunctional(FunctionNode *node) {}

void FunctionDefParser::parseConstant(FunctionNode *node)
{
    getLexer().nextToken().as(Token::DUAL_ARROW);
    ExpressionParser expParser(getLexer(), getWorkModule());
    auto exp = expParser.parseExpression();
    getLexer().skipUntil({Token::SEMI_COLON});
    node->ast.push_back(new Return(exp.value_or(nullptr)));
}

void FunctionDefParser::parseNormal(FunctionNode *node)
{
    setTraitList(node);
    if (getLexer().lookAhead().type == Token::ARROW) {
        getLexer().nextToken();
        auto type = parseTypeName();
        node->setReturnType(*type.value_or(&Type::get("Void")));

        bool err = getLexer().lookAhead().type != Token::LC_BRACKET;
        while (getLexer().lookAhead().type != Token::LC_BRACKET)
            getLexer().nextToken();
        if (err)
            panic("not a valid function signiture");
        node->setTrait(FunctionNode::EXPLICIT_TYPE);
    }

    auto scope = Scope::nextScope(getLexer(), Token::LC_BRACKET, Token::RC_BRACKET);
    if (!scope.has_value())
        return;
    pushLexer(*scope.value());

    getLexer().nextToken().as(Token::LC_BRACKET);
    node->ast = parseStatementBlock()->block;

    popLexer();
}

void FunctionDefParser::setTraitList(FunctionNode *node)
{
    if (getLexer().end() || getLexer().lookAhead().type != Token::LSQ_BRACKET)
        return;
    auto scope = Scope::nextScope(getLexer(), Token::LSQ_BRACKET, Token::RSQ_BRACKET);
    KVANTUM_VERIFY_ABANDON(scope.has_value(), "invalid trait list");

    pushLexer(*scope.value());

    try {
        getLexer().nextToken();
        while (!getLexer().end() && getLexer().lookAhead().type != Token::RSQ_BRACKET) {
            Token next = getLexer().nextToken().as(Token::IDENTIFIER);
            if (next.value == "public")
                node->setTrait(FunctionNode::PUBLIC);
            else if (next.value == "const")
                node->setTrait(FunctionNode::CONST);
            else if (next.value == "static")
                node->setTrait(FunctionNode::STATIC);
            else if (next.value == "virtual")
                node->setTrait(FunctionNode::VIRTUAL);
            else if (next.value == "override")
                node->setTrait(FunctionNode::OVERRIDE);
            else
                panic("unknown trait " + next.value);

            if (!getLexer().end() && getLexer().lookAhead().type == Token::COMMA)
                getLexer().nextToken();
        }
    } catch (Lexer::UnexpectedEndOfTokens &) {
        popLexer();
        panic("Unexpected end of trait list");
    }
    popLexer();
}
} // namespace kvantum::parser
