#include "parser/parser.hpp"
#include "parser/functiondefparser.hpp"

namespace kvantum::parser
{
    Parser::Parser(deque<string> &includes, Compiler* owner) : includes(includes)
    {
        mod = nullptr;
        lexer = nullptr;
        annotation = nullptr;
        this->owner = owner;
    }

    void Parser::addFunction(FunctionNode* func)
    {
        auto thismod = this->mod;
        auto id = func->getFunctionID();
        if (id.isField()) {
            auto obj = thismod->getObject(id.parentObj);
            obj.addFunction(id.name, func);
        }
        thismod->addFunction(func);
    }

    void Parser::parseArguments(vector<Expression*> &args)
    {
        auto scope = Scope::nextScope(lexer, Token::L_BRACKET, Token::R_BRACKET);
        if (!scope.has_value())
            return;
        lexer = scope.value().get();
        try {
            Token t = lexer->nextToken().as(Token::L_BRACKET);
            t = lexer->lookAhead();

            while (t.type != Token::R_BRACKET) {
                args.push_back(parseExpression().value_or(nullptr));

                t = lexer->lookAhead();
                if (t.type == Token::COMMA)
                    t = lexer->nextToken();
            }
            lexer->nextToken();
        }
        catch (Lexer::UnexpectedEndOfTokens &) {
            panic("Unexpected end of argument list");
        }
    }

    optional<Cast*> Parser::parseCast(Expression* base)
    {
        lexer->nextToken().as(Token::AS);
        auto type = parseType();
        if (!type.has_value())
            return {};
        KVANTUM_VERIFY_RETURN(!type.value()->isVoid(), "cannot cast to Void", {});
        return new Cast(base, *type.value());
    }

    optional<Expression*> Parser::parseBracketedExpression() {}

    optional<Type*> Parser::parseType()
    {
        if(lexer->lookAhead().type == Token::LSQ_BRACKET){
            lexer->nextToken();
            auto type = parseType();
            lexer->nextToken().as(Token::RSQ_BRACKET);
            return &ListType::get(*type.value_or(&Type::get("Void")));
        }

        if(lexer->lookAhead().type == Token::LESS_T){
            lexer->nextToken();
            auto type = parseType();
            lexer->nextToken().as(Token::GREATER_T);
            return &ArrayType::get(*type.value_or(&Type::get("Void")));
        }

        bool isRef = lexer->lookAhead().type == Token::AMPERSAND;
        if(isRef){
            while(lexer->lookAhead().type == Token::AMPERSAND){
                lexer->nextToken();
                Diagnostics::warn("multiple references");
            }
        }

        auto tok = lexer->nextToken().as(Token::IDENTIFIER);
        if (!mod->hasType(tok.value)){
            panic("no type named " + tok.value);
            return {};
        }
        Type* type = &mod->getType(tok.value);

        if(isRef)
            return &ReferenceType::get(*type);
        return type;
    }

    optional<FunctionCall*> Parser::parseFunctionCall(Expression* var)
    {
        KVANTUM_VERIFY_RETURN(var && var->exprtype == ExprType::VARIABLE,"invalid function identifier",{});
        auto* call = new FunctionCall(var->as<Variable*>(), {});
        auto lex = lexer;
        parseArguments(call->arguments);
        lexer = lex;
        return call;
    }

    Variable* Parser::parseVariable(const Token &idToken)
    {
        Variable* var = new Variable(idToken.value);
        if (lexer->lookAhead().type != Token::DOT)
            return var;

        auto field = parseFieldAccess(var);
        if (!field.has_value())
            return var;
        else return field.value();
    }

    optional<FieldAccess*> Parser::parseFieldAccess(Expression* var)
    {
        lexer->nextToken().as(Token::DOT);
        if (!lexer->end() && lexer->lookAhead().type == Token::IDENTIFIER)
            return new FieldAccess(var, parseVariable(lexer->nextToken()));
        panic("no identifier after .");
        return {};
    }

    bool Parser::isBop(const Token &t)
    {
        vector<Token::TokenType> bops{
                Token::PLUS, Token::MINUS, Token::MULTIPLY, Token::DIVIDE,
                Token::LOG_EQUAL, Token::LOG_N_EQUAL, Token::AND, Token::OR,
                Token::GREATER_T, Token::LESS_T};
        return std::find(bops.begin(), bops.end(), t.type) != bops.end();
    }

    bool Parser::isLiteral(const kvantum::Token &t)
    {
        return t.type < 4;
    }

    void* Parser::expressionPanic(const Token &t)
    {
        KVANTUM_VERIFY(isBop(t), t.value + " is not a valid expression");
        else
            panic("missing left hand side of binary operation");
        return nullptr;
    }

    optional<BinaryOperation*> Parser::parseBop(optional<Expression*> lhs)
    {
        Token t = lexer->nextToken();
        auto rhs = parseExpression();

        if (!lhs.has_value() || !rhs.has_value())
            return {};
        vector<Token::TokenType> bops{
                Token::PLUS, Token::MINUS, Token::MULTIPLY, Token::DIVIDE,
                Token::LOG_EQUAL, Token::LOG_N_EQUAL,
                Token::LESS_T, Token::LESS_OR_EQ_T,
                Token::GREATER_T, Token::GREATER_OR_EQ_T,
                Token::AND, Token::OR};
        BinaryOperation::Operator op = (BinaryOperation::Operator) std::distance(bops.begin(), std::find(ITER_THROUGH(bops), t.type));
        return new BinaryOperation(lhs.value(), rhs.value(), op);
    }

    optional<ArrayExpression*> Parser::parseArrayExpression()
    {
        auto init = parseArrayInitializer(Token::LESS_T, Token::GREATER_T);
        Type* t = &Type::get("Void");
        if (!init.empty())
            t = &init[0]->getType();

        return new ArrayExpression(*t, init);
    }

    Expression* Parser::toPrefixForm(Expression* infix) const {}

    optional<FunctionCall*> Parser::parseListExpression()
    {
        auto init = parseArrayInitializer(Token::LSQ_BRACKET, Token::RSQ_BRACKET);
        Type &t = Type::get("Void");
        if (!init.empty())
            t = init[0]->getType();

        ///if its the first time a list with the specifie type has beed initiated add to the type pool
        if (!mod->hasType(ListType::get(t).getName()))
            mod->addObjectType(&ListType::get(t));

        ///pack the arrayexpression into a [].new call and provide the array body and size as arguments
        return new FunctionCall(new FieldAccess(new Variable("[]", ListType::get(t)), new Variable("new")),
                                {new ArrayExpression(t, init), new Literal(std::to_string(init.size()), Type::get("Int"))}, ListType::get(t).getFunction("new"));
    }

    vector<Literal*> Parser::parseArrayInitializer(Token::TokenType beg, Token::TokenType end)
    {
        auto old_lexer = lexer;
        auto scope = lexer::Scope::nextScope(lexer, beg, end);
        if (!scope.has_value())
            return {};
        vector<Literal*> init;
        lexer = scope.value().get();
        try {
            scope.value()->dropScopeDelimitors();
            while (!lexer->end()) {
                Token t = lexer->lookAhead();
                auto expr = parseExpression();
                if (!lexer->end() && lexer->lookAhead().type == Token::COMMA)
                    lexer->nextToken();
                if (expr.has_value()) {
                    KVANTUM_VERIFY(expr.value()->exprtype == ExprType::LITERAL, "array initialzer can only be literal value");
                    else
                        init.push_back(expr.value()->as<Literal*>());
                }
            }
        }
        catch (Lexer::UnexpectedEndOfTokens &) {
            panic("invalid array initializer");
        }
        lexer = old_lexer;
        return init;
    }

    optional<ArrayIndex*> Parser::parseArrayIndex(Expression* basearr)
    {
        lexer->nextToken().as(Token::LSQ_BRACKET);
        auto ind = parseExpression();
        if (!ind.has_value())
            return {};
        lexer->nextToken().as(Token::RSQ_BRACKET);
        return new ArrayIndex(basearr, ind.value());
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
    optional<Expression*> Parser::parseExpression()
    {
        Token t = lexer->lookAhead();
        optional<Expression*> this_expr = nullptr;

        //if its a primitve type literal
        if (isLiteral(t)) {
            t = lexer->nextToken();
            if (t.type == Token::STRING) {
                ///trim the quotes
                t.value.pop_back();
                t.value.erase(0, 1);
            }
            this_expr = new Literal(t.value, PrimitiveType::get(static_cast<PrimitiveType::TypeBase>(t.type)));
        } else
            switch (t.type) {
                CASE(Token::IDENTIFIER, this_expr = parseVariable(lexer->nextToken()));
                CASE(Token::L_BRACKET, this_expr = parseBracketedExpression());
                CASE(Token::NONE, this_expr = new Literal(lexer->nextToken().value, ObjectType::getObject()));
                CASE(Token::LSQ_BRACKET, this_expr = parseListExpression());
                CASE(Token::LESS_T, this_expr = parseArrayExpression());
                CASE(Token::AMPERSAND, lexer->nextToken();this_expr = new TakeReference(parseExpression().value_or(nullptr)));
                default:
                    expressionPanic(lexer->nextToken());
                    return {};
            }

        while (true) {
            if (isBop(lexer->lookAhead()))
                this_expr = parseBop(this_expr);
            switch (lexer->lookAhead().type) {
                CASE(Token::DOT, this_expr = parseFieldAccess(this_expr.value_or(nullptr)));
                CASE(Token::L_BRACKET, this_expr = parseFunctionCall(this_expr.value_or(nullptr)));
                CASE(Token::LSQ_BRACKET, this_expr = parseArrayIndex(this_expr.value_or(nullptr)););
                CASE(Token::AS, this_expr = parseCast(this_expr.value_or(nullptr)));
                default:
                    return this_expr;
            }
        }
    }

    void Parser::parseFunctionDefinition()
    {
        FunctionDefParser fparser(includes, lexer, this->mod, owner,annotation);
        auto func = fparser.parseFunctionDecl();
        annotation = nullptr;
        addFunction(func);
    }

    void Parser::parseTypeDefinition()
    {
        Module* mod = this->mod;
        lexer->nextToken().as(Token::TYPE);
        Token id = lexer->nextToken().as(Token::IDENTIFIER);
        TypeNode* node = new TypeNode(id.value);

        ///type inheritance
        optional<Type*> parentT = {};
        if (lexer->lookAhead().type == Token::BACK_ARROW) {
            lexer->nextToken();
            parentT = parseType();
            if (parentT.has_value() && !parentT.value()->isObject()) {
                panic("Cannot derive from non-object type " + parentT.value()->getName());
                parentT = {};
            }
        }
        lexer->nextToken().as(Token::LC_BRACKET);

        mod->addObjectType(new ObjectType(node, parentT.has_value() ? (ObjectType*) &parentT.value() : nullptr));

        ///parse the type body
        while (lexer->lookAhead().type != Token::RC_BRACKET) {
            Token fieldId = lexer->nextToken().as(Token::IDENTIFIER);
            lexer->nextToken().as(Token::COLON);
            if (node->fields.count(fieldId.value))
                panic(id.value + " already has a field named " + fieldId.value);
            else {
                auto templt = parseType();
                KVANTUM_VERIFY(*templt.value_or(&Type::get("Void")) != Type::get("Void"), "field cannot be declared with Void value");
                node->fields.emplace(fieldId.value, templt.value_or(&Type::get("Void")));
                if (!templt.has_value())
                    lexer->skipUntil({Token::SEMI_COLON});
            }
            if (!lexer->consumeIf(Token::SEMI_COLON).has_value())
                panic("no semi colon at the end of field defintion");
        }
        lexer->nextToken().as(Token::RC_BRACKET);
        annotation = nullptr;
    }

    void Parser::parseExternalDependency()
    {
        lexer->nextToken().as(Token::USE);
        KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
        auto mod = lexer->nextToken().value;
        KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
        lexer->nextToken().as(Token::NAMESPACE_SCOPE);
        KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
        auto item = lexer->nextToken().value;
        KVANTUM_VERIFY(lexer->consumeIf(Token::SEMI_COLON).has_value(), "semi colon missing after use directive");

        auto comp = this->mod->getCompiler();
        if (comp->hasFunction(mod, item))
            this->mod->addExternalFunctionDependency(mod, item);
        else if (comp->hasObject(mod, item))
            this->mod->addExternalObjectDependency(mod, item);
        else
            panic("no function or object named " + item + " in module " + mod);
    }

    void Parser::parseFile(Lexer* lexer)
    {
        this->lexer = lexer;
        FunctionNode* thisfunc = new FunctionNode(lexer->getModuleName());
        auto currmod = std::make_unique<Module>(lexer->getFileName(), owner);
        this->mod = currmod.get();
        owner->addModule(std::move(currmod));
        mod->addFunction(thisfunc);

        try {
            while (!lexer->end()) {
                Token t = lexer->lookAhead();
                if (t.type == Token::FUNCTION)
                    parseFunctionDefinition();
                else if (t.type == Token::TYPE)
                    parseTypeDefinition();
                else if (t.type == Token::USE)
                    parseExternalDependency();
                else if(t.type == Token::ANNOTATION){
                    t = lexer->nextToken();
                    KVANTUM_VERIFY(Annotation::isValid(t.value),"no valid annotation "+t.value);
                    else annotation = Annotation::getAnnotation(t.value);
                }

                else {
                    auto st = parseStatement();
                    thisfunc->ast.push_back(st);
                }
            }
            thisfunc->setReturnType(Type::get("Void"));
        }
        catch (Lexer::UnexpectedEndOfTokens &) {
            panic("Unexpected end of file");
        }
    }

    void Parser::Parse()
    {
        for (auto& file : includes) {
            auto lex = std::make_unique<lexer::Lexer>(file);
            if (!lex->hasError()) {
                parseFile(lex.get());
            }
        }
    }
}
