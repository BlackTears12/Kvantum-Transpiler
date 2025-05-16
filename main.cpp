#include "common/compiler.hpp"

int main(int argc, char **argv)
{
    string file = argc > 1 ? argv[1] : "main.kv";
    kvantum::Compiler compiler;
    compiler.compile(file);
    return 0;
}
