#include "parser/parser.hpp"

namespace kvantum::parser {

class FunctionDefParser : public Parser
{
public:
    FunctionDefParser(Lexer &lexer, Module &workMod, vector<Annotation *> &annotations = {});
    unique_ptr<FunctionNode> parseFunctionDefinition();

private:
    Variable *parseFunctionIdentifier();
    void parseFormalParams(vector<Variable *> &params);
    void parseFunctional(FunctionNode *);
    void parseConstant(FunctionNode *);
    void parseNormal(FunctionNode *);
    void setTraitList(FunctionNode *node);

    unique_ptr<FunctionNode> node;
};
} // namespace kvantum::parser
