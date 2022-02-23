#pragma once
#include "util.hpp"
#include "types.hpp"

#include <iostream>

namespace noct
{
	struct TypecheckEnv
	{
		
	};
	
	struct FuncDeclaration
	{
		FuncSignature signature;
		std::string name;
		std::vector<std::string> argNames;
	};

	struct VarDeclaration
	{
		Ptr<Type> type;
		std::string name;
	};

	using TypeRes = Result<Ptr<Type>>;
	
	struct ASTNodeImpl;

	struct AST
	{
		Ptr<struct ASTNodeImpl> impl_;

		virtual TypeRes type(TypecheckEnv &env) const noexcept = 0;
		virtual void print(std::ostream &out, int indent) const noexcept = 0;
	};

	struct ASTVar : AST
	{
		VarDeclaration decl;
		Ptr<AST> value;

		TypeRes type(TypecheckEnv &env) const noexcept override;
		void print(std::ostream &out, int indent) const noexcept override;
	};

	struct ASTFunc : AST
	{
		FuncDeclaration decl;
		Ptr<AST> body;

		TypeRes type(TypecheckEnv &env) const noexcept override;
		void print(std::ostream &out, int indent) const noexcept override;
	};

	struct ASTBlock : AST
	{
		std::vector<Ptr<AST>> nodes;
		
		TypeRes type(TypecheckEnv &env) const noexcept override;
		void print(std::ostream &out, int indent) const noexcept override;
	};

	struct ASTInt : AST
	{
		std::size_t value;

		ASTInt(std::size_t value) : value(value) { }
		
		TypeRes type(TypecheckEnv &env) const noexcept override;
		void print(std::ostream &out, int indent) const noexcept override;
	};
}
