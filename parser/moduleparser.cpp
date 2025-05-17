#include "moduleparser.hpp"

#include "functiondefparser.hpp"

namespace kvantum::parser {

ModuleParser::ModuleParser(const string& fileName)
    : Parser(*new Module(fileName))
{
    workModule = unique_ptr<Module>(&getWorkModule());
}

unique_ptr<Module> ModuleParser::parse() {}

void ModuleParser::addFunction(unique_ptr<FunctionNode> func)
{
    auto id = func->getFunctionID();
    if (id.isField()) {
        auto obj = getWorkModule().getObject(id.parentObj);
        obj.addFunction(id.name, std::move(func));
    }
    getWorkModule().addFunction(std::move(func));
}

void ModuleParser::parseFile(Lexer* lexer)
{
    vector<Annotation*> annotations;
    try {
        while (!getLexer().end()) {
            Token t = getLexer().lookAhead();
            if (t.type == Token::FUNCTION)
                parseFunctionDefinition(annotations);
            else if (t.type == Token::TYPE)
                parseTypeDefinition(annotations);
            else if (t.type == Token::USE)
                parseExternalDependency();
            else if (t.type == Token::ANNOTATION) {
                t = getLexer().nextToken();
                KVANTUM_VERIFY(Annotation::isValid(t.value), "no valid annotation " + t.value);
                else
                {
                    annotations.push_back(Annotation::getAnnotation(t.value));
                }
            } else {
                Diagnostics::warn("Unexprected token :" + getLexer().nextToken().value);
            }
        }
    } catch (Lexer::UnexpectedEndOfTokens&) {
        panic("Unexpected end of file");
    }
}

void ModuleParser::parseFunctionDefinition(vector<Annotation*>& annotations)
{
    FunctionDefParser fparser(getLexer(), getWorkModule(), annotations);
    auto func = fparser.parseFunctionDecl();
    addFunction(func);
}

void ModuleParser::parseTypeDefinition(vector<Annotation*>& annotations)
{
    getLexer().nextToken().as(Token::TYPE);
    Token id = getLexer().nextToken().as(Token::IDENTIFIER);
    TypeNode* node = new TypeNode(id.value);

    ///type inheritance
    optional<Type*> parentT = {};
    if (getLexer().lookAhead().type == Token::BACK_ARROW) {
        getLexer().nextToken();
        parentT = parseTypeName();
        if (parentT.has_value() && !parentT.value()->isObject()) {
            panic("Cannot derive from non-object type " + parentT.value()->getName());
            parentT = {};
        }
    }
    getLexer().nextToken().as(Token::LC_BRACKET);

    getWorkModule().addObjectType(
        new ObjectType(node, parentT.has_value() ? (ObjectType*) &parentT.value() : nullptr));

    ///parse the type body
    while (getLexer().lookAhead().type != Token::RC_BRACKET) {
        Token fieldId = getLexer().nextToken().as(Token::IDENTIFIER);
        getLexer().nextToken().as(Token::COLON);
        if (node->fields.count(fieldId.value))
            panic(id.value + " already has a field named " + fieldId.value);
        else {
            auto templt = parseTypeName();
            KVANTUM_VERIFY(*templt.value_or(&Type::get("Void")) != Type::get("Void"),
                           "field cannot be declared with Void value");
            node->fields.emplace(fieldId.value, templt.value_or(&Type::get("Void")));
            if (!templt.has_value())
                getLexer().skipUntil({Token::SEMI_COLON});
        }
        if (!getLexer().consumeIf(Token::SEMI_COLON).has_value())
            panic("no semi colon at the end of field defintion");
    }
    getLexer().nextToken().as(Token::RC_BRACKET);
}

void ModuleParser::parseExternalDependency()
{
    getLexer().nextToken().as(Token::USE);
    KVANTUM_VERIFY_ABANDON(!getLexer().end(), "invalid use directive");
    auto mod = getLexer().nextToken().value;
    KVANTUM_VERIFY_ABANDON(!getLexer().end(), "invalid use directive");
    getLexer().nextToken().as(Token::NAMESPACE_SCOPE);
    KVANTUM_VERIFY_ABANDON(!getLexer().end(), "invalid use directive");
    auto item = getLexer().nextToken().value;
    KVANTUM_VERIFY(getLexer().consumeIf(Token::SEMI_COLON).has_value(),
                   "semi colon missing after use directive");

    if (Compiler::Instance().hasFunction(mod, item))
        getWorkModule().addExternalFunctionDependency(mod, item);
    else if (Compiler::Instance().hasObject(mod, item))
        getWorkModule().addExternalObjectDependency(mod, item);
    else
        panic("no function or object named " + item + " in module " + mod);
}

} // namespace kvantum::parser
