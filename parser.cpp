#include "parser.hpp"
#include "fmt.hpp"
#include "log.hpp"

namespace noct
{
	namespace
	{
		std::string expectedErrorMessage(const std::string &msg, const Token &t)
		{
			std::string type;
			if(t.type > ' ')
				type = std::string(1, t.type);
			else
				type = tokenTypeToString((TokenType)t.type);
			return "expected " + msg + ", but got " + type;
		}

	} // namespace

	bool Parser::expect(It &it, char type, const std::string &msg)
	{
		if(it.peek().type != type)
		{
			error(expectedErrorMessage(msg, it.peek()));
			return false;
		}

		return true;
	}

	bool Parser::expectAndGet(It &it, char type, const std::string &msg)
	{
		return expect(it, type, msg) && it.get();
	}

	Ptr<Type> Parser::parseTypeAtomic(It &it)
	{
		auto t = it.get();
		if(t.type == TokenType::idn)
		{
			if(t.value == "i64")
				return makePtr<TypeNumeric>(NumericType::i64);
			if(t.value == "i32")
				return makePtr<TypeNumeric>(NumericType::i32);
			if(t.value == "i16")
				return makePtr<TypeNumeric>(NumericType::i16);
			if(t.value == "i8")
				return makePtr<TypeNumeric>(NumericType::i8);
			if(t.value == "u64")
				return makePtr<TypeNumeric>(NumericType::u64);
			if(t.value == "u32")
				return makePtr<TypeNumeric>(NumericType::u32);
			if(t.value == "u16")
				return makePtr<TypeNumeric>(NumericType::u16);
			if(t.value == "u8")
				return makePtr<TypeNumeric>(NumericType::u8);
			return nullptr;
		}
		error(expectedErrorMessage("a type", t));
		return nullptr;
	}

	Ptr<Type> Parser::parseTypeSuffix(It &it, const Ptr<Type> &base)
	{
		if(it.peek().type == '*')
			return makePtr<TypePointer>(base);
		return base;
	}

	Ptr<Type> Parser::parseType(It &it)
	{
		return parseTypeSuffix(it, parseTypeAtomic(it));
	}

	Ptr<AST> Parser::parseSuffix(It &it, const Ptr<AST> &base)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseAtomic(It &it)
	{
		auto t = it.get();
		if(t.type == TokenType::num)
			return makePtr<ASTInt>(std::stoull(t.value));
		if(t.type == TokenType::idn)
			return makePtr<ASTIdn>(t.value);

		error("expected a number or a variable!");
		return nullptr;
	}

	Ptr<AST> Parser::parsePrefix(It &it)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseInfix(It &it)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseExpr(It &it)
	{
		return parseAtomic(it);
	}

	Ptr<AST> Parser::parseStmt(It &it)
	{
		return parseExpr(it);
	}

	Ptr<AST> Parser::parseBlock(It &it)
	{
		auto b = makePtr<ASTBlock>();
		expect(it, '{', "an opening '{' for a block") &&it.get();

		while(it.peek().type != '}')
			b->nodes.push_back(parseStmt(it));

		expect(it, '}', "a closing '}' for a block") &&it.get();
		return b;
	}

	Ptr<AST> Parser::parseReturn(It &it)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseVariable(It &it)
	{
		auto f = makePtr<ASTVar>();
		expectAndGet(it, TokenType::kwd_let, "the 'let' keyword");

		if(!expect(it, TokenType::idn, "the variable's name"))
			f->decl.name = "<error>";
		else
			f->decl.name = it.get().value;

		expectAndGet(it, ':', "a colon");
		f->decl.type = parseType(it);

		if(it.peek().type == '=')
		{
			it.get();
			f->value = parseExpr(it);
		}

		expectAndGet(it, ';', "a semicolon");

		return f;
	}

	Ptr<AST> Parser::parseFunction(It &it)
	{
		auto f = makePtr<ASTFunc>();
		expectAndGet(it, TokenType::kwd_fn, "the 'fn' keyword");

		if(!expect(it, TokenType::idn, "the function's name"))
			f->decl.name = "<error>";
		else
			f->decl.name = it.get().value;

		// TODO: Arguments

		expectAndGet(it, TokenType::opr_arrow, "a return type arrow");

		f->decl.signature.returnType = parseType(it);

		f->body = parseBlock(it);

		return f;
	}

	Ptr<AST> Parser::parseTopLevel(It &it)
	{
		if(it.peek().type == TokenType::kwd_fn)
			return parseFunction(it);

		if(it.peek().type == TokenType::kwd_let)
			return parseVariable(it);

		error(expectedErrorMessage("Expected a top-level statement.", it.get()));
		return parseTopLevel(it);
	}

	std::vector<Ptr<AST>> Parser::parseProgram(It &it)
	{
		std::vector<Ptr<AST>> prog;

		while(it.peek().type != TokenType::eof) prog.push_back(parseTopLevel(it));

		return prog;
	}
} // namespace noct
