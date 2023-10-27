#include "lexer/lexer.hpp"
#include "common/compiler.hpp"
namespace kvantum::lexer
{
   std::vector<string> std_string_split(string str)
   {
      std::vector<string> ret;
      for(int i = 0; i < str.size(); i++) {
         string s = "";
         auto instr = false;
         while(i < str.size() && (instr || !isspace(str[i]))) {
            s += str[i];
            if(str[i] == '\'')
               instr = !instr;
            i++;
         }
         //str[i] is whitespace
         if(s != "")
            ret.push_back(s);
      }
      return ret;
   }

   Lexer::Lexer(string file_name)
   {
      regexes.resize(Token::END_OF_FILE);
      file = file_name;
      initializeRegexes();
      lineIndex = 1;
      err = false;
      is.open(file_name);
      if(is.fail()){
         err = true;
         panic("cannot open file " + file_name);
      }
   }

   Lexer::Lexer(queue<Token> &toks) : tokens(std::move(toks)),err(false),lineIndex(0){}
   Lexer::Lexer(Lexer& lexer) : Lexer(lexer.tokens){}
   
   Lexer::~Lexer()
   {
      is.close();
   }
   
   void Lexer::lex()
   {
      string line;
      while (!is.eof()) {
         std::getline(is,line);
         auto parts = std_string_split(line);
         for(auto &e : parts) {
            tokenize(e);
         }
         lineIndex++;
      }
      tokens.push({Token::END_OF_FILE,"",file});
   }
   
   void Lexer::printTokens()
   {
      Token tok;
      queue<Token> toks(tokens);
      while(toks.size() > 0) {
         tok = toks.front();
         toks.pop();
         Diagnostics::log(tok.typeToString() + " " + tok.value);
      }
   }
   
   bool Lexer::end()
   {
      return tokens.empty() || tokens.front().type == Token::END_OF_FILE;
   }

   void Lexer::skipUntil(vector<Token::TokenType> t)
   {
      auto contains = [t](Token::TokenType tok){ return std::find(t.begin(),t.end(),tok) != t.end(); };
      while(!contains(nextToken().type));
   }
   
   Token Lexer::nextToken()
   {
      Token t = lookAhead();
      tokens.pop();
      return t;
   }
   
   Token Lexer::lookAhead()
   {
      if(tokens.empty())
        throw UnexpectedEndOfTokens();
      auto t = tokens.front();
      kvantum::Diagnostics::setLineIndex(t.lineIndex);
      kvantum::Diagnostics::setWorkingModule(t.filename);
      return t;
   }

   std::optional<Token> Lexer::consumeIf(Token::TokenType t)
   {
      auto tok = lookAhead();
      if(tok.type == t)
         return nextToken();
      return {};
   }
   
   void Lexer::initializeRegexes()
   {
      setRegex(Token::RATIONAL,"[0-9]+\\.[0-9]*");
      setRegex(Token::INTEGER,R"([1-9]+[0-9]*|0)");
      setRegex(Token::BOOLEAN,"(True|False)");
      setRegex(Token::STRING, R"(\".*\")");
      setRegex(Token::NONE,"None");
      setRegex(Token::EQUALS,"=");
      setRegex(Token::PLUS,"\\+");
      setRegex(Token::MINUS,"-");
      setRegex(Token::MULTIPLY,"\\*");
      setRegex(Token::DIVIDE,"/");
      setRegex(Token::LESS_T,"<");
      setRegex(Token::LESS_OR_EQ_T, "<=");
      setRegex(Token::GREATER_T,">");
      setRegex(Token::GREATER_OR_EQ_T, ">=");
      setRegex(Token::LOG_EQUAL,"==");
      setRegex(Token::LOG_N_EQUAL,"!=");
      setRegex(Token::AND,"and");
      setRegex(Token::OR,"or");
      setRegex(Token::NOT,"not");
      setRegex(Token::L_BRACKET,"\\(");
      setRegex(Token::R_BRACKET,"\\)");
      setRegex(Token::LC_BRACKET,"\\{");
      setRegex(Token::RC_BRACKET,"\\}");
      setRegex(Token::LSQ_BRACKET,"\\[");
      setRegex(Token::RSQ_BRACKET,"\\]");
      setRegex(Token::DOT,"\\.");
      setRegex(Token::COLON,":");
      setRegex(Token::COMMA,",");
      setRegex(Token::SEMI_COLON,";");
      setRegex(Token::WHILE,"while");
      setRegex(Token::FOR,"for");
      setRegex(Token::IF,"if");
      setRegex(Token::ELSE,"else");
      setRegex(Token::FUNCTION,"fn");
      setRegex(Token::TYPE,"type");
      setRegex(Token::RETURN,"ret");
      setRegex(Token::ARROW,"->");
      setRegex(Token::BACK_ARROW, "<-");
      setRegex(Token::DUAL_ARROW, "=>");
      setRegex(Token::NAMESPACE_SCOPE, "::");
      setRegex(Token::USE, "use");
      setRegex(Token::IDENTIFIER,"[a-zA-Z_][a-zA-Z0-9_]*");
   }
   
   void Lexer::setRegex(Token::TokenType token,string reg)
   {
      regexes[token] = new std::regex("^"+reg+"$");
   }
   
   bool Lexer::match(string str,Token::TokenType &type)
   {
      std::smatch match;
      for(int i = 0; i < regexes.size(); i++) {
         if(std::regex_match(str,match,*regexes[i])) {
            type = static_cast<Token::TokenType>(i);
            return true;
         }
      }
      return false;
   }
   
   int Lexer::tokenizePart(string line)
   {
       if (line == "")
           return 0;
      Token::TokenType type;
      if(match(line,type)){
         tokens.push({type,line,file,lineIndex});
         return (int)line.size();
      }
      else{
         return tokenizePart(line.substr(0,line.size()-1));
      }
   }
   
   void Lexer::tokenize(string line)
   {
      int idx = 0;
      while(idx < line.size()){
         int incr = tokenizePart(line.substr(idx));
         if(!incr){
            panic("could not tokenize " + line.substr(incr));
            err = true;
            return;
         }
         idx += incr;
      }
   }
   }