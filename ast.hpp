#pragma once
#include "util.hpp"
#include "types.hpp"

#include <unordered_map>
#include <iostream>
#include <string>
#include <utility>

namespace noct
{
	class TypecheckEnv
	{
	public:
		auto has(const std::string &name) -> bool;
		auto get(const std::string &name) -> Ptr<Type>;
		void set(const std::string &name, Ptr<Type> t);

	private:
		using MapType = std::unordered_map<std::string, Ptr<Type>>;
		auto it_(const std::string &name) -> Result<MapType::iterator>;
		MapType values;
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

	struct ASTImpl;

	struct AST
	{
		Ptr<struct ASTImpl> impl_;

		virtual TypeRes type(TypecheckEnv &env) const noexcept = 0;
		virtual void print(std::ostream &out, int indent) const noexcept = 0;
	};

	struct ASTIdn : AST
	{
		std::string name;

		ASTIdn(std::string name) : name(std::move(name)) { }

		TypeRes type(TypecheckEnv &env) const noexcept override;
		void print(std::ostream &out, int indent) const noexcept override;
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
