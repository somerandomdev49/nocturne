#pragma once
#include "fmt.hpp"

namespace noct
{
	template<typename... Args>
	void debug(const std::string &fmt, Args &&...args)
	{
		std::cout << "debug: "
		          << noct::format(fmt, std::forward<Args>(args)...)
		          << std::endl;
	}

	template<typename... Args>
	void error(const std::string &fmt, Args &&...args)
	{
		std::cerr << "\033[0;31mError: "
		          << noct::format<Args...>(fmt, std::forward<Args>(args)...)
		          << "\033[0;0m" << std::endl;
	}

	template<typename... Args>
	void panic(const char *funcname, const std::string &fmt, Args &&...args)
	{
		std::cerr << "\033[0;31mPanic in " << funcname << "()! "
		          << noct::format<Args...>(fmt, std::forward<Args>(args)...)
		          << "\033[0;0m" << std::endl;
		std::exit(1);
	}
}
