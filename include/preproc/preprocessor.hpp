#include "common/mainheader.hpp"
#include <iostream>
#include <fstream>
#include <deque>
using std::deque;
using std::vector;

namespace kvantum
{
   class Preprocessor{
   public:
      Preprocessor(const string filename){
         processFile(filename);
      }
      void processFile(const string filename);
      deque<string>& getIncludes();
   private:
      deque<string> includes;
   };
}