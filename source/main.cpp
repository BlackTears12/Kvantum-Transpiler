#include "common/compiler.hpp"

int main(int argc,char** argv)
{
   string file;
   if(argc < 2)
      file = "main";
   else file = argv[1];
   kvantum::Compiler compiler;
   compiler.compile(file);
   return 0;
}