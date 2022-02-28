#include "ast.hpp"
#include "fmt.hpp"

namespace noct
{
	auto TypecheckEnv::has(const std::string &name) -> bool
	{
		return !it_(name).error;
	}

	auto TypecheckEnv::get(const std::string &name) -> Ptr<Type>
	{
		if(auto r = it_(name); !r.error)
			return r.value->second;
		else
			return nullptr;
	}

	void TypecheckEnv::set(const std::string &name, Ptr<Type> t)
	{
		if(has(name))
			return;
		values[name] = t;
	}

	auto TypecheckEnv::it_(const std::string &name) -> Result<MapType::iterator>
	{
		auto i = values.find(name);
		return {i == values.end(), i};
	}

	TypeRes ASTFunc::type(TypecheckEnv &env) const noexcept
	{
		auto t = body->type(env);
		if(t.error)
			return t;

		return {!decl.signature.returnType->assignable(t.value),
		        decl.signature.returnType};
	}

	void ASTFunc::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << "fn " << decl.name << " -> ";
		decl.signature.returnType->print(out);
		if(body != nullptr)
		{
			out << '\n';
			body->print(out, indent);
		}
	}

	TypeRes ASTBlock::type(TypecheckEnv &env) const noexcept
	{
		if(nodes.size() == 0)
			return TypeRes{false, nullptr};
		return nodes.back()->type(env);
	}

	void ASTBlock::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << "{\n";

		for(const auto &node : nodes)
		{
			node->print(out, indent + 1);
			out << "\n";
		}
		out << Indent(indent) << "}\n";
	}

	TypeRes ASTInt::type(TypecheckEnv &env) const noexcept
	{
		return {false,
		        Ptr<TypeNumeric>(new TypeNumeric(getSuitableIntegerTypeFor(value)))};
	}

	void ASTInt::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << getNumericTypeName(getSuitableIntegerTypeFor(value))
		    << " " << value;
	}

	TypeRes ASTVar::type(TypecheckEnv &env) const noexcept
	{
		if(value != nullptr)
		{
			if(auto v = value->type(env); !v.error && !decl.type->assignable(v.value))
				return {true, nullptr};
		}
		env.set(decl.name, decl.type);
		return {false, decl.type};
	}

	void ASTVar::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << "let " << decl.name << ": ";
		decl.type->print(out);
		if(value != nullptr)
		{
			out << " = ";
			value->print(out, 0);
		}
		out << ";";
	}

	TypeRes ASTIdn::type(TypecheckEnv &env) const noexcept
	{
		if(auto t = env.get(name); t != nullptr)
			return {false, t};
		else
			return {true, nullptr};
	}

	void ASTIdn::print(std::ostream &out, int indent) const noexcept
	{
		out << Indent(indent) << name;
	}
} // namespace noct
