#include "moduleparser.hpp"

namespace kvantum::parser {

ModuleParser::ModuleParser() {}

void ModuleParser::addFunction(FunctionNode* func)
{
    auto thismod = this->mod;
    auto id = func->getFunctionID();
    if (id.isField()) {
        auto obj = thismod->getObject(id.parentObj);
        obj.addFunction(id.name, func);
    }
    thismod->addFunction(func);
}

void ModuleParser::parseFile(Lexer* lexer)
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
            else if (t.type == Token::ANNOTATION) {
                t = lexer->nextToken();
                KVANTUM_VERIFY(Annotation::isValid(t.value), "no valid annotation " + t.value);
                else annotation = Annotation::getAnnotation(t.value);
            }

            else {
                auto st = parseStatement();
                thisfunc->ast.push_back(st);
            }
        }
        thisfunc->setReturnType(Type::get("Void"));
    } catch (Lexer::UnexpectedEndOfTokens&) {
        panic("Unexpected end of file");
    }
}

void ModuleParser::parseFunctionDefinition()
{
    FunctionDefParser fparser(includes, lexer, this->mod, owner, annotation);
    auto func = fparser.parseFunctionDecl();
    annotation = nullptr;
    addFunction(func);
}

void ModuleParser::parseTypeDefinition()
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

    mod->addObjectType(
        new ObjectType(node, parentT.has_value() ? (ObjectType*) &parentT.value() : nullptr));

    ///parse the type body
    while (lexer->lookAhead().type != Token::RC_BRACKET) {
        Token fieldId = lexer->nextToken().as(Token::IDENTIFIER);
        lexer->nextToken().as(Token::COLON);
        if (node->fields.count(fieldId.value))
            panic(id.value + " already has a field named " + fieldId.value);
        else {
            auto templt = parseType();
            KVANTUM_VERIFY(*templt.value_or(&Type::get("Void")) != Type::get("Void"),
                           "field cannot be declared with Void value");
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

void ModuleParser::parseExternalDependency()
{
    lexer->nextToken().as(Token::USE);
    KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
    auto mod = lexer->nextToken().value;
    KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
    lexer->nextToken().as(Token::NAMESPACE_SCOPE);
    KVANTUM_VERIFY_ABANDON(!lexer->end(), "invalid use directive");
    auto item = lexer->nextToken().value;
    KVANTUM_VERIFY(lexer->consumeIf(Token::SEMI_COLON).has_value(),
                   "semi colon missing after use directive");

    auto comp = this->mod->getCompiler();
    if (comp->hasFunction(mod, item))
        this->mod->addExternalFunctionDependency(mod, item);
    else if (comp->hasObject(mod, item))
        this->mod->addExternalObjectDependency(mod, item);
    else
        panic("no function or object named " + item + " in module " + mod);
}

} // namespace kvantum::parser
