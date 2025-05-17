#ifndef MODULEPARSER_HPP
#define MODULEPARSER_HPP

#include "parser.hpp"

namespace kvantum::parser {

class ModuleParser : public Parser
{
public:
    ModuleParser();

    unique_ptr<Module> parse(const string& fileName);

private:
    void parseFunctionDefinition();
    void parseTypeDefinition();
    void parseFile(Lexer* lexer);
    void parseExternalDependency();
    void addFunction(unique_ptr<FunctionNode> func);

    unique_ptr<Module> workModule;
};

} // namespace kvantum::parser

#endif // MODULEPARSER_HPP
