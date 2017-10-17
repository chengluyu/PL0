#include <fstream>
#include <iostream>

#include "parser.h"
#include "vm.h"

void print_help() {
    std::cout <<
        "\n"
        "Usage: pl0 [OPTIONS]\n"
        "\n"
        " -h --help          : Show this message and exit.\n"
        " -o --output <file> : Specify output file. When absent the output"
                              "will be written to STDOUT.\n"
        " -i --input <file>  : Specify input file, input file may be either"
                              "text source code or pre-compiled byte"
                              "code.\n"
        " -c --compile       : Set to compiler mode. In this mode, pl0 will"
                              "generate byte code only.\n"
        " -r --run           : Set to interpreter mode. In this mode, pl0 will"
                              "compile and run the code.\n"
        ;
}

int main(int argc, const char* const argv[]) {
#ifdef USE_ARGV
	if (argc < 2) {
        std::cout << "Usage: pl0 [source code]" << std::endl;
		return 0;
	}
    const char * const src = argv[1];
#else
    const char * const src = "";
#endif
	std::ifstream fin(src);
	pl0::lexer lex(fin);
	pl0::parser parser(lex);
	pl0::bytecode code = parser.program();
    for (size_t i = 0; i < code.size(); i++) {
        std::cout
            << i << '\t'
            << *(std::get<0>(code[i])) << ' '
            << std::get<1>(code[i]) << '\t'
            << std::get<2>(code[i]) << '\n';
    }
    pl0::execute(code);
    std::cin.get();
	return 0;
}