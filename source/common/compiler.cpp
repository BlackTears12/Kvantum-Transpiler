#include "common/compiler.hpp"
#include "parser/parser.hpp"
#include "parser/typechecker.hpp"
#include "codegen/c_codegenerator.hpp"
#include "interpreter/interpreter.hpp"

using kvantum::parser::Parser;
using kvantum::parser::TypeChecker;
using kvantum::codegen::C_Generator;
using kvantum::interpreter::Interpreter;

namespace kvantum
{
    bool fileExists(const string &filename)
    {
        std::ifstream is(filename);
        return !is.fail();
    }

    Compiler::Compiler()
    {
        Type::initialize();
    }

    vector<FunctionNode*> Compiler::getFunctionGroup(string modname, string funcname)
    {
        return getModule(modname)->getFunctionGroup(funcname);
    }

    ObjectType &Compiler::getObject(string modname, string objname)
    {
        return getModule(modname)->getObject(objname);
    }

    bool Compiler::hasFunction(string modname, string funcname)
    {
        return hasModule(modname) && getModule(modname)->hasInternalFunction(funcname);
    }

    bool Compiler::hasObject(string modname, string objname)
    {
        return hasModule(modname) && getModule(modname)->hasInternalType(objname);
    }

    Module* Compiler::getModule(string name)
    {
        return std::find_if(ITER_THROUGH(modules), [&name](unique_ptr<Module> &m) {
            return m->getName() == name;
        })->get();
    }

    bool Compiler::hasModule(string name)
    {
        return std::find_if(ITER_THROUGH(modules), [&name](unique_ptr<Module> &m) {
            return m->getName() == name;
        }) < modules.end();
    }

    void Compiler::addModule(unique_ptr<Module> mod)
    {
        modules.push_back(std::move(mod));
        kvantum::Diagnostics::setWorkingModule(modules[modules.size() - 1]->getName());
        kvantum::Diagnostics::setLineIndex(0);
    }

    void Compiler::compile(const string& filename)
    {
        if(!fileExists(filename)){
            std::cerr << "Cannot find " << filename << std::endl;
            return;
        }

        Diagnostics::setVerbosity(Diagnostics::Verbosity::ERROR);
        deque<string> incl;
        incl.push_back(filename);
        Parser parser(incl, this);
        parser.Parse();
        if (kvantum::Diagnostics::hasError()) {
            kvantum::Diagnostics::fail();
            exit(1);
        }

        Diagnostics::log("code parsed");
        TypeChecker tc;
        for (int i = 0; i < modules.size(); i++) {
            kvantum::Diagnostics::setWorkingModule(modules[i]->getName());
            kvantum::Diagnostics::setLineIndex(0);
            tc.checkModule(modules[i].get());
        }
        if (kvantum::Diagnostics::hasError()) {
            kvantum::Diagnostics::fail();
            exit(1);
        }

        Diagnostics::log("analysis success");
        codegen::CodeExecutorInterface* ce = new kvantum::interpreter::Interpreter();
        for (int i = 0; i < modules.size(); i++) {
            ce->generate(modules[i].get());
        }
        ce->exec();
        //std::cout << "code generated" << std::endl;
        //system((string("gcc ")+modules[1]->getName() + ".c -o "+ modules[1]->getName()).c_str());
    }

    void kvantum::Diagnostics::warn(string msg)
    {
        if (verbosity >= Verbosity::WARNING)
            errors[workModule].emplace(msg, workModule, lineIndex);
    }

    void kvantum::Diagnostics::error(string msg)
    {
        if (verbosity == Verbosity::ABORT)
            throw std::invalid_argument(msg + " -> at " + workModule + ":" + std::to_string(lineIndex));
        if (verbosity >= Verbosity::ERROR)
            errors[workModule].emplace(msg, workModule, lineIndex);
    }

    void kvantum::Diagnostics::log(string msg)
    {
        if (verbosity >= Verbosity::LOG)
            std::cout << msg << std::endl;
    }

    void kvantum::Diagnostics::fail()
    {
        for (auto &e: errors) {
            std::ifstream is;
            is.open(e.first);
            string s = "";
            unsigned int lineIdx = 0;
            while (!e.second.empty()) {
                ///if already stepped over the line reset the file
                if (lineIdx > e.second.front().lineIndex) {
                    lineIdx = 0;
                    is.seekg(0, std::ios::beg);
                }

                ///step until the specified lineidx
                while (lineIdx != e.second.front().lineIndex) {
                    std::getline(is, s);
                    lineIdx++;
                }
                std::cout << e.second.front().message + " at line " + std::to_string(e.second.front().lineIndex) + " file: " + e.first << "\n";
                std::cout << s << "\n\n";
                e.second.pop();
            }
            is.close();
        }
    }

    map<string, queue<Error>> kvantum::Diagnostics::errors = {};
    unsigned int kvantum::Diagnostics::lineIndex = 0;
    string kvantum::Diagnostics::workModule = "";
    Diagnostics::Verbosity Diagnostics::verbosity = Diagnostics::Verbosity::WARNING;
}