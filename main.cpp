#include "common/compiler.hpp"

int main(int argc, char **argv)
{
    string file = argc > 1 ? argv[1] : "main.kv";
    auto compiler = kvantum::Compiler::Instance();
    compiler.compile(file);
    return 0;
}
