#include "ast.hpp"
#include "fmt.hpp"

namespace noct
{
	TypeRes ASTFunc::type(TypecheckEnv &env) const noexcept
	{
		auto t = body->type(env);
		if(t.error) return t;

		return TypeRes { !decl.signature.returnType->assignable(t.value), decl.signature.returnType };
	}

	void ASTFunc::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << "fn " << decl.name << " -> ";
		decl.signature.returnType->print(out);
		out << "\n";
		body->print(out, indent + 1);
	}


	TypeRes ASTBlock::type(TypecheckEnv &env) const noexcept
	{
		if(nodes.size() == 0) return TypeRes { false, nullptr };
		return nodes.back()->type(env);
	}

	void ASTBlock::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << "{\n";
		for(const auto &node : nodes)
			node->print(out, indent + 1);
		out << Indent(indent) << "}\n";
	}


	TypeRes ASTInt::type(TypecheckEnv &env) const noexcept
	{
		return TypeRes { false, Ptr<TypeNumeric>(new TypeNumeric(getSmallestIntegerTypeFor(value))) };
	}

	void ASTInt::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << getNumericTypeName(getSmallestIntegerTypeFor(value))
		                      << " " << value << "\n";
	}
}
