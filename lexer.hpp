#pragma once
#include "token.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

namespace noct
{
	struct Lexer;
	
	struct TokenIterator
	{
		Lexer &lexer;
		Token current;
		
		TokenIterator &operator++();
		const Token &operator*() { return current; }
		bool operator==(const TokenIterator &other) const /* not implemented */;
	};
	
	struct Lexer
	{
		std::istream &in;
		
		auto begin() -> TokenIterator;
		auto end() -> TokenIterator;
	};
}
