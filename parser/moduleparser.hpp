#ifndef MODULEPARSER_HPP
#define MODULEPARSER_HPP

#include "parser.hpp"

namespace kvantum::parser {

class ModuleParser : public Parser
{
public:
    ModuleParser(const string& fileName);

    unique_ptr<Module> parse();

private:
    void parseFunctionDefinition(vector<Annotation*>& annotations);
    void parseTypeDefinition(vector<Annotation*>& annotations);
    void parseFile(Lexer* lexer);
    void parseExternalDependency();
    void addFunction(unique_ptr<FunctionNode> func);

    unique_ptr<Module> workModule;
};

} // namespace kvantum::parser

#endif // MODULEPARSER_HPP
