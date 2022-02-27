#pragma once
#include "types.hpp"
#include "ast.hpp"

#include <string>
#include <vector>

namespace noct
{
	enum class GeneratorOpt
	{
		ShouldOutputAssembly,
		OutputAssemblyFile,
		ShouldOutputObject,
		OutputObjectFile,
		ShouldOutputIR,
		OutputIRFile,
	};

	enum class GeneratorBool
	{
		No = 0, Yes = 1
	};

	struct Generator
	{
		struct GeneratorImpl *impl;
		
		Generator(const std::string &moduleName);
		~Generator();

		void generateFunction(ASTFunc *func);
		void generate(AST *node);
		void output();
		
		void set(GeneratorOpt opt, GeneratorBool value);
		void set(GeneratorOpt opt, const std::string &value);
		void set(GeneratorOpt opt, int value);
	};
}
