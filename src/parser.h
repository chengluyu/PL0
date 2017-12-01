#ifndef PL_ZERO_PARSER_H
#define PL_ZERO_PARSER_H

#include "lexer.h"
#include "scope.h"
#include "assembler.h"

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

    void subprogram();
    // declarations
    void variable_decl();
    void constant_decl();
    void procedure_decl();
    // statements
    void statement_block();
    void if_statement();
    void while_statement();
    void call_statement();
    void statement();
    void read_statement();
    void write_statement();
    void assign_statement();
    void return_statement();
    // expressions
    variable *lvalue();
    void condition();
    void expression();
    void term();
    void factor();
public:
    explicit parser(lexer &lex);
    void program();
};

}

#endif
