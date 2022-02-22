#include "lexer.hpp"
#include <cctype>

namespace noct
{
	auto TokenIterator::operator++() -> TokenIterator&
	{
		while(std::isspace(lexer.in.peek()))
			lexer.in.get();
		
		/**/ if(std::isalpha(lexer.in.peek()) || lexer.in.peek() == '_')
		{
			current.value = "";
			while(std::isalnum(lexer.in.peek()) || lexer.in.peek() == '_')
				current.value += lexer.in.get();
			
			/**/ if(current.value == "fn"  ) current.type = TokenType::kwd_fn;
			else if(current.value == "if"  ) current.type = TokenType::kwd_if;
			else if(current.value == "else") current.type = TokenType::kwd_else;
			else                             current.type = TokenType::idn;
		}
		else if(std::isdigit(lexer.in.peek()))
		{
			current.type = TokenType::num;
			current.value = "";
			while(std::isdigit(lexer.in.peek()) || lexer.in.peek() == '_')
				current.value += lexer.in.get();
		}
		else if(lexer.in.peek() == EOF)
		{
			current.type = TokenType::eof;
			current.value = "<EOF>";
		}
		else switch(lexer.in.peek())
		{
		case '-':
			current.type = lexer.in.get();
			if(lexer.in.peek() == '>')
			{
				lexer.in.get();
				current.type = TokenType::opr_arrow;
				current.value = "->";
			}
			break;
		default:
			current.value = std::string(1, lexer.in.peek());
			current.type = lexer.in.get();
			break;
		}

		return *this;
	}

	auto Lexer::begin() -> TokenIterator { return TokenIterator{ *this }; }
}
