#pragma once
#include "util.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "ast.hpp"

namespace noct
{
	struct Parser
	{
		using It = BufferedIterator<Token, TokenIterator>;

		bool expect(It &it, char type, const std::string &msg = "");
		bool expectAndGet(It &it, char type, const std::string &msg = "");

		Ptr<Type> parseTypeAtomic(It &it);
		Ptr<Type> parseTypeSuffix(It &it, const Ptr<Type> &b);
		Ptr<Type> parseType(It &it);
		
		Ptr<AST> parseSuffix(It &it, const Ptr<AST> &b);
		Ptr<AST> parseAtomic(It &it);
		Ptr<AST> parsePrefix(It &it);
		Ptr<AST> parseInfix(It &it);
		Ptr<AST> parseExpr(It &it);
		Ptr<AST> parseStmt(It &it);
		Ptr<AST> parseBlock(It &it);
		Ptr<AST> parseReturn(It &it);
		Ptr<AST> parseFunction(It &it);
		Ptr<AST> parseVariable(It &it);
		Ptr<AST> parseTopLevel(It &it);
		std::vector<Ptr<AST>> parseProgram(It &it);
	};
}
