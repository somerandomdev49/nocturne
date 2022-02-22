#include "util.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "codegen.hpp"

auto main(int argc, char *argv[]) -> int
{
	if(argc < 3) return 1;

	std::ifstream inp(argv[1]);
	if(!inp) return 1;

	auto l = noct::Lexer { inp };
	auto bl = noct::BufferedIterable<noct::Token, noct::Lexer>(l);
	auto it = bl.begin();
	
	// do
	// {
	// 	if(it.peek().type > ' ')
	// 		std::cout << "token: '" << it.peek().type << "'" << std::endl;
	// 	else
	// 		std::cout << "token: " << noct::tokenTypeToString((noct::TokenType)it.peek().type)
	// 		          << " '" << it.peek().value << "'" << std::endl;
	// 	it.get();
	// }
	// while(it.peek().type != noct::TokenType::eof);

	auto parser = noct::Parser();
	auto program = parser.parseProgram(it);

	noct::TypecheckEnv typecheckEnv;
	for(const auto &node : program)
	{
		node->print(std::cout, 0);
		if(node->type(typecheckEnv).error) {
			std::cout << "Type error!" << std::endl;
			break;
		}
	}

	noct::Generator gen(argv[1]);
	
	if(auto f = dynamic_cast<noct::ASTFunc*>(program[0].get()); f != nullptr)
		gen.generateFunction(f);
	else
	{
		return 2;
	}

	gen.set(noct::GeneratorOpt::ShouldOutputObject, noct::GeneratorBool::Yes);
	gen.set(noct::GeneratorOpt::OutputObjectFile, std::string(argv[2]));
	gen.set(noct::GeneratorOpt::ShouldOutputIR, noct::GeneratorBool::Yes);
	gen.set(noct::GeneratorOpt::OutputIRFile, std::string(argv[2]) + ".ll");
	gen.output();

	std::cout.flush();
	inp.close();

	return 0;
}
