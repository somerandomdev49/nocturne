#include "lexer.hpp"
#include <cctype>

namespace noct
{
	namespace
	{
		Token getSpecialToken(Lexer &lexer, const Token &d)
		{
			Token current = d;

			switch(lexer.in.peek())
			{
			case '-':
				lexer.in.get();
				if(lexer.in.peek() == '>')
				{
					current.type = TokenType::opr_arrow;
					current.value += lexer.in.get();
				}
				else if(lexer.in.peek() == '-')
				{
					current.type = TokenType::opr_decnt;
					current.value += lexer.in.get();
				}
				break;
			case '=':
				current.type = lexer.in.get();
				if(lexer.in.peek() == '=')
				{
					current.type = TokenType::opr_equal;
					current.value += lexer.in.get();
				}
				break;
			case '!':
				if(lexer.in.peek() == '=')
				{
					current.type = TokenType::opr_noteq;
					current.value += lexer.in.get();
				}
				break;
			case '+':
				if(lexer.in.peek() == '+')
				{
					current.type = TokenType::opr_incnt;
					current.value += lexer.in.get();
				}
				break;
			case '&':
				if(lexer.in.peek() == '&')
				{
					current.type = TokenType::opr_d_amp;
					current.value += lexer.in.get();
				}
				break;
			case '|':
				if(lexer.in.peek() == '|')
				{
					current.type = TokenType::opr_d_bar;
					current.value += lexer.in.get();
				}
				break;
			case '>':
				if(lexer.in.peek() == '=')
				{
					current.type = TokenType::opr_gteql;
					current.value += lexer.in.get();
				}
				else if(lexer.in.peek() == '>')
				{
					current.type = TokenType::opr_shftr;
					current.value += lexer.in.get();
				}
				break;
			case '<':
				if(lexer.in.peek() == '=')
				{
					current.type = TokenType::opr_gteql;
					current.value += lexer.in.get();
				}
				else if(lexer.in.peek() == '<')
				{
					current.type = TokenType::opr_shftl;
					current.value += lexer.in.get();
				}
				break;
			default:
				// current.value = std::string(1, lexer.in.peek());
				current.type = lexer.in.get();
				break;
			}

			return current;
		}
	} // namespace

	auto TokenIterator::operator++() -> TokenIterator &
	{
		while(std::isspace(lexer.in.peek())) lexer.in.get();

		if(std::isalpha(lexer.in.peek()) || lexer.in.peek() == '_')
		{
			current.value = "";
			while(std::isalnum(lexer.in.peek()) || lexer.in.peek() == '_')
				current.value += lexer.in.get();

			if(current.value == "fn")
				current.type = TokenType::kwd_fn;
			else if(current.value == "if")
				current.type = TokenType::kwd_if;
			else if(current.value == "let")
				current.type = TokenType::kwd_let;
			else if(current.value == "else")
				current.type = TokenType::kwd_else;
			else
				current.type = TokenType::idn;
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
		else
		{
			current.type = lexer.in.peek();
			current.value = std::string(1, lexer.in.peek());
			current = getSpecialToken(lexer, current);
		}

		return *this;
	}

	auto Lexer::begin() -> TokenIterator { return TokenIterator{*this}; }
} // namespace noct

/*
opr_arrow, // ->
opr_equal, // ==
opr_noteq, // !=
opr_incnt, // ++
opr_decnt, // --
opr_d_amp, // &&
opr_d_bar, // ||
opr_lteql, // <=
opr_gteql, // >=
opr_shftr, // >>
opr_shftl, // <<
*/
