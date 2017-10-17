#include <fstream>
#include <iostream>
#include <cstring>

#include "parser.h"
#include "vm.h"

void print_help() {
    std::cout <<
        "\n"
        "Usage: pl0 [options] filename\n"
        "\n"
        "  --help           : Show this message and exit.\n"
        "  --print-bytecode : Print bytecode after code generation.\n"
        ;
}

void print_bytecode(const pl0::bytecode &code) {
    for (size_t i = 0; i < code.size(); i++) {
        std::cout
            << i << '\t'
            << *(std::get<0>(code[i])) << '\t'
            << std::get<1>(code[i]) << '\t'
            << std::get<2>(code[i]) << '\n';
    }
}

int main(int argc, const char* const argv[]) {
    if (argc < 2 || argv[1] == std::string { "--help" }) {
        print_help();
        return 0;
    }
    int filename_index = 1;
    bool show_bytecode = false;
    // check if there is any options
    if (argv[filename_index] == std::string { "--print-bytecode" }) {
        filename_index++;
        show_bytecode = true;
    }
    if (filename_index == argc) {
        std::cerr << "Expect filename after options.\n";
        print_help();
        return 1;
    }
    // compile and run the code
    try {
        std::ifstream fin(argv[filename_index]);
        if (fin.fail()) {
            std::cerr << "Failed to open file: \"" << argv[1] << "\"\n";
            return 1;
        }
        pl0::lexer lex(fin);
        pl0::parser parser(lex);
        pl0::bytecode code = parser.program();
        if (show_bytecode) {
            print_bytecode(code);
        }
        pl0::execute(code);
    } catch (pl0::general_error error) {
        std::cout << error.what() << std::endl;
    }
    return 0;
}
