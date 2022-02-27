#include "parser.hpp"

namespace noct
{
	namespace
	{
		std::string expectedErrorMessage(const std::string &msg, const Token &t)
		{
			std::string type;
			if(t.type > ' ') type = std::string(1, t.type);
			else             type = tokenTypeToString((TokenType)t.type);
			return "expected " + msg + ", but got " + type;
		}
	}

	void Parser::error(const std::string &msg)
	{
		std::cerr << "\033[0;31mError: " << msg << "\033[0;0m" << std::endl;
	}

	bool Parser::expect(It &it, char type, const std::string &msg)
	{
		if(it.peek().type != type)
		{
			error(expectedErrorMessage(msg, it.peek()));
			return false;
		}
	
		return true;
	}
	
	Ptr<Type> Parser::parseTypeAtomic(It &it)
	{
		auto t = it.get();
		if(t.type == TokenType::idn)
		{
			/**/ if(t.value == "i64") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::i64));
			else if(t.value == "i32") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::i32));
			else if(t.value == "i16") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::i16));
			else if(t.value == "i8" ) return Ptr<TypeNumeric>(new TypeNumeric(NumericType::i8 ));
			else if(t.value == "u64") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::u64));
			else if(t.value == "u32") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::u32));
			else if(t.value == "u16") return Ptr<TypeNumeric>(new TypeNumeric(NumericType::u16));
			else if(t.value == "u8" ) return Ptr<TypeNumeric>(new TypeNumeric(NumericType::u8 ));
			else                      return nullptr;
		}
		else error(expectedErrorMessage("a type!", t));
		return nullptr;
	}
	
	Ptr<Type> Parser::parseTypeSuffix(It &it, Ptr<Type> base)
	{
		if(it.peek().type == '*') return Ptr<TypePointer>(new TypePointer(base));
		return base;
	}

	Ptr<Type> Parser::parseType(It &it)
	{
		return parseTypeSuffix(it, parseTypeAtomic(it));
	}

	Ptr<AST> Parser::parseSuffix(It &it, Ptr<AST> base)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseAtomic(It &it)
	{
		auto t = it.get();
		/**/ if(t.type == TokenType::num) return Ptr<ASTInt>(new ASTInt(std::stoull(t.value)));
		else if(t.type == TokenType::idn) return Ptr<ASTIdn>(new ASTIdn(t.value));
		else error("expected a number or a variable!");
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
		auto b = Ptr<ASTBlock>(new ASTBlock());
		expect(it, '{', "an opening '{' for a block") && it.get();

		while(it.peek().type != '}') {
			b->nodes.push_back(parseStmt(it));
		}

		expect(it, '}', "a closing '}' for a block") && it.get();
		return b;
	}

	Ptr<AST> Parser::parseReturn(It &it)
	{
		return nullptr;
	}

	Ptr<AST> Parser::parseVariable(It &it)
	{
		auto f = Ptr<ASTVar>(new ASTVar());
		expect(it, TokenType::kwd_let, "the 'let' keyword") && it.get();

		if(!expect(it, TokenType::idn, "the variable's name"))
			f->decl.name = "<error>";
		else
			f->decl.name = it.get().value;
		
		expect(it, ':', "a colon") && it.get();
		f->decl.type = parseType(it);

		if(it.peek().type == '=')
		{
			it.get();
			f->value = parseExpr(it);
		}

		expect(it, ';', "a semicolon") && it.get();

		return f;
	}

	Ptr<AST> Parser::parseFunction(It &it)
	{
		auto f = Ptr<ASTFunc>(new ASTFunc());
		expect(it, TokenType::kwd_fn, "the 'fn' keyword") && it.get();

		if(!expect(it, TokenType::idn, "the function's name"))
			f->decl.name = "<error>";
		else
			f->decl.name = it.get().value;
		
		// TODO: Arguments

		expect(it, TokenType::opr_arrow, "a return type arrow") && it.get();
		
		f->decl.signature.returnType = parseType(it);

		f->body = parseBlock(it);

		return f;
	}

	Ptr<AST> Parser::parseTopLevel(It &it)
	{
		/**/ if(it.peek().type == TokenType::kwd_fn)  return parseFunction(it);
		else if(it.peek().type == TokenType::kwd_let) return parseVariable(it);
		
		error(expectedErrorMessage("Expected a top-level statement.", it.get()));
		return parseTopLevel(it);
	}

	std::vector<Ptr<AST>> Parser::parseProgram(It &it)
	{
		std::vector<Ptr<AST>> prog;
		
		while(it.peek().type != TokenType::eof)
			prog.push_back(parseTopLevel(it));
		
		return prog;
	}
}
