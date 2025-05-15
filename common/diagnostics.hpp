#pragma once
#include <map>
#include <queue>
#include <string>
#include <utility>
using std::map;
using std::queue;
using std::string;

namespace kvantum {
struct Error
{
    Error(string msg, string mod, unsigned int ln)
        : message(std::move(msg))
        , moduleName(std::move(mod))
        , lineIndex(ln)
    {}

    string message;
    string moduleName;
    unsigned int lineIndex;
};

class Diagnostics
{
public:
    enum Verbosity { NONE, ERROR, WARNING, LOG, ABORT };
    static void warn(string msg);
    static void error(string msg);
    static void log(string msg);
    static void setVerbosity(Verbosity v) { verbosity = v; }

    static void setWorkingModule(string modname) { workModule = std::move(modname); }
    static void setLineIndex(unsigned int lnIndex) { lineIndex = lnIndex; }

    static bool hasError() { return !errors.empty(); }
    static void fail();
    static unsigned int getLineIndex() { return lineIndex; }

private:
    static map<string, queue<Error>> errors;
    static unsigned int lineIndex;
    static string workModule;
    static Verbosity verbosity;
};
} // namespace kvantum
