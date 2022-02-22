#pragma once
#include <string>

namespace noct
{
	struct Token
	{
		char type;
		std::string value;
		
		operator bool() const { return true; }
	};

	enum TokenType : char
	{
		eof,
		idn,
		num,
		str,
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
		kwd_fn,
		kwd_if,
		kwd_else,
	};

	constexpr auto tokenTypeToString(TokenType t) -> const char *
	{
		switch(t)
		{
		case TokenType::eof: return "eof";
		case TokenType::idn: return "idn";
		case TokenType::num: return "num";
		case TokenType::str: return "str";
		case TokenType::opr_arrow: return "opr_arrow";
		case TokenType::opr_equal: return "opr_equal";
		case TokenType::opr_noteq: return "opr_noteq";
		case TokenType::opr_incnt: return "opr_incnt";
		case TokenType::opr_decnt: return "opr_decnt";
		case TokenType::opr_d_amp: return "opr_d_amp";
		case TokenType::opr_d_bar: return "opr_d_bar";
		case TokenType::opr_lteql: return "opr_lteql";
		case TokenType::opr_gteql: return "opr_gteql";
		case TokenType::opr_shftr: return "opr_shftr";
		case TokenType::opr_shftl: return "opr_shftl";
		case TokenType::kwd_fn: return "kwd_fn";
		case TokenType::kwd_if: return "kwd_if";
		case TokenType::kwd_else: return "kwd_else";
		default: return "?";
		}
	}
}
