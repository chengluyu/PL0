#include <fstream>
#include <iostream>

#include "parsing/parser.h"
#include "vm.h"
#include "ast/pretty-printer.h"
#include "ast/dot-generator.h"

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
        std::cout << i << '\t' << *code[i].op << '\t'
            << code[i].level << '\t' << code[i].address << '\n';
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
        std::cerr << "Error: expect filename after options.\n";
        print_help();
        return 1;
    }
    // compile and run the code
    std::ifstream fin(argv[filename_index]);
    if (fin.fail()) {
        std::cerr << "Error: failed to open file: \"" << argv[1] << "\"\n";
        return 1;
    }
    pl0::lexer lex(fin);
    pl0::parser parser(lex);
    try {
        auto program = parser.program();
        pl0::ast::ast_printer printer{std::cout};
        pl0::ast::dot_generator drawer;
        printer.visit_block(program);
        drawer.generate(program);
        drawer.save_to_file("test.txt");
    } catch (pl0::general_error &error) {
        pl0::location loc = lex.current_location();
        std::cout << "Error(" << loc.to_string() << "): "
                  << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
