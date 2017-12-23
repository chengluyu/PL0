#include <fstream>
#include <iostream>

#include "parsing/parser.h"
#include "vm.h"
#include "ast/ast.h"
#include "ast/pretty-printer.h"
#include "ast/dot-generator.h"
#include "bytecode/compiler.h"
#include "argparser.h"


void print_bytecode(const pl0::bytecode &code) {
    for (size_t i = 0; i < code.size(); i++) {
        std::cout << i << '\t' << *code[i].op << '\t'
            << code[i].level << '\t' << code[i].address << '\n';
    }
}

struct options {
    bool show_bytecode = false;
    bool show_tokens = false;
    bool compile_only = false;
    bool show_ast = false;
    std::string output_graph_file = "";
    std::string input_file = "";
};

options parse_args(int argc, const char *argv[]) {
    try {
        options option;
        std::vector<std::string> rest;
        pl0::argument_parser<options> parser{"Yet Another PL/0 Interpreter"};
        parser.flags({"--show-bytecode", "-s"}, "Print bytecode after code generation", &options::show_bytecode);
        parser.flags({"--show-tokens", "-l"}, "Print all tokens.", &options::show_tokens);
        parser.flags({"--compile-only", "-c"}, "If specified, bytecode will not be executed.", &options::compile_only);
        parser.flags({"--show-ast", "-t"}, "Print abstract syntax tree.", &options::show_ast);
        parser.store<std::initializer_list<const char *>>(
                {"--plot-tree", "-t"},
                "If specified, the GraphViz representation of abstract syntax tree will be output to file.",
                &options::output_graph_file);
        parser.parse(argc, argv, option, rest);

        if (rest.empty())
            parser.show_help();

        option.input_file = rest[0];
        return option;
    } catch (pl0::basic_error &error) {
        std::cout << "Error: " << error.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

[[noreturn]] void print_tokens(pl0::lexer &lex) {
    while (true) {
        auto token = lex.peek();
        std::cout << lex.current_location().to_string() << '\t' << *token << '\t' << lex.get_literal() << '\n';
        if (token == pl0::token::EOS)
            break;
        lex.advance();
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, const char* argv[]) {
    options option = parse_args(argc, argv);

    std::ifstream fin(option.input_file);
    if (fin.fail()) {
        std::cerr << "Error: failed to open file: \"" << option.input_file << "\"\n";
        return 1;
    }

    pl0::lexer lex(fin);

    if (option.show_tokens)
        print_tokens(lex);

    pl0::parser parser(lex);
    pl0::ast::block *program = nullptr;

    try {
        program = parser.program();
    } catch (pl0::general_error &error) {
        pl0::location loc = lex.current_location();
        std::cout << "Error(" << loc.to_string() << "): " << error.what() << '\n';
        return EXIT_FAILURE;
    }


    pl0::code::compiler compiler{};
    compiler.generate(program);

    if (!option.output_graph_file.empty()) {
        pl0::ast::dot_generator plotter;
        plotter.generate(program);
        plotter.save_to_file(option.output_graph_file.c_str());
    }

    if (option.show_ast) {
        pl0::ast::ast_printer printer{std::cout};
        printer.visit_block(program);
    }

    if (option.show_bytecode)
        print_bytecode(compiler.code());

    if (!option.compile_only)
        pl0::execute(compiler.code());

    return EXIT_SUCCESS;
}
