#pragma once
#include "ast/ast.hpp"
#include "module.hpp"

namespace kvantum
{
	class Compiler
	{
	public:
		Compiler();
		void compile(string file);
		vector<FunctionNode*> getFunctionGroup(string modname,string funcname);
		ObjectType& getObject(string modname, string objname);

		bool hasFunction(string modname,string funcname);
		bool hasObject(string modname, string objname);

		Module* getModule(string name);
		bool hasModule(string name);
		void addModule(unique_ptr<Module> mod);
	private:
		FunctionNode* createBuiltin(string name);
		vector<unique_ptr<Module>> modules;
	};
}
