#include "parser/parser.hpp"

namespace kvantum::parser {

class FunctionDefParser : public Parser
{
public:
    FunctionDefParser(deque<string> &d,
                      Lexer *lexer,
                      Module *m,
                      Compiler *owner,
                      Annotation *annotation = nullptr);
    FunctionNode *parseFunctionDecl();

private:
    Variable *parseFunctionIdentifier();
    void parseFormalParams(vector<Variable *> &params);
    void parseFunctional(FunctionNode *);
    void parseConstant(FunctionNode *);
    void parseNormal(FunctionNode *);
    void setTraitList(FunctionNode *node);

    FunctionNode *node;
};
} // namespace kvantum::parser
