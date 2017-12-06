#ifndef PL_ZERO_PARSER_H
#define PL_ZERO_PARSER_H

#include "lexer.h"
#include "scope.h"
#include "../bytecode/assembler.h"
#include "../ast/ast.h"

namespace pl0 {

class parser {
    lexer &lexer_;
    scope *top_;

    // scope control
    void enter_scope();
    void leave_scope();

    // lexical helper functions
    void expect(token tk);
    std::string identifier();
    int number();

    ast::block * subprogram();
    // declarations
    ast::variable_declaration * variable_decl();
    ast::constant_declaration * constant_decl();
    ast::procedure_declaration * procedure_decl();
    // statements
    ast::statement_list * statement_list();
    ast::if_statement * if_statement();
    ast::while_statement * while_statement();
    ast::call_statement * call_statement();
    ast::statement * statement();
    ast::read_statement * read_statement();
    ast::write_statement * write_statement();
    ast::assign_statement * assign_statement();
    ast::return_statement * return_statement();
    // expressions
    ast::variable_proxy * local_variable();
    ast::expression * condition();
    ast::expression * expression();
    ast::expression * term();
    ast::expression * factor();
public:
    explicit parser(lexer &lex);
    ast::block * program();
};

}

#endif
