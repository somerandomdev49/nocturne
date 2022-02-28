#pragma once

#include <iostream>
#include <sstream>

namespace noct
{
	struct Indent
	{
		int indent;
		int indentWidth;
		char indentChar;
		Indent(int indent, int indentWidth = 4, char indentChar = ' ')
			: indent(indent), indentWidth(indentWidth), indentChar(indentChar)
		{}
	};

	static inline std::ostream &operator<<(std::ostream &out, const Indent &indent)
	{
		for(int i = 0; i < indent.indentWidth * indent.indent; ++i)
			out.put(indent.indentChar);

		return out;
	}

	template<typename ...Args>
	auto format(const std::string &fmt, Args &&...args) -> std::string
	{
		std::stringstream ss;

		for(std::size_t i = 0; i < fmt.size();)
		{
			if(fmt[i] == '{')
			{
				++i;

				std::size_t k = fmt[i++] - '0';
				std::size_t j = 0;

				std::initializer_list<int> _ =
				{
					(j++, (k == j - 1 ? ((ss << args), 0) : 0))...
				};

				++i;
			}
			else ss << fmt[i++];
		}

		return ss.str();
	}
}
