#include "preproc/preprocessor.hpp"
#include <cstring>
#include <algorithm>
#include <iterator>

namespace kvantum
{
   template <typename Iterator>
   static bool contains(Iterator begin,Iterator end,typename std::iterator_traits<Iterator>::value_type item){
      while(begin != end && *begin != item){
         begin++;
      }
      return begin != end;
   }

   void Preprocessor::processFile(const string filename){
      includes.push_front(filename);
      std::ifstream is;
      is.open(filename);
      if (is.fail())
          throw std::invalid_argument(std::strerror(errno));
      std::string expr;
      while (std::getline(is, expr))
      {
         string::iterator iter = std::find(expr.begin(),expr.end(),' ');
         if(iter >= expr.end()-1 || string(expr.begin(),iter) != "import")
            continue;
         string file(iter+1,expr.end());
         if(!contains(includes.begin(),includes.end(),file))
            processFile(file);
      }
   }


   deque<string>& Preprocessor::getIncludes(){
      return includes;
   }
}