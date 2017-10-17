#ifndef PL_ZERO_PARSER_H
#define PL_ZERO_PARSER_H

#include "lexer.h"
#include "scope.h"
#include "assembler.h"

namespace pl0 {

class parser {
    lexer &lexer_;
    scope *top_;
	assembler asm_;

	// scope control
	void enter_scope() {
		top_ = new scope(top_);
	}

	void leave_scope() {
		scope *inner = top_;
		top_ = top_->get_enclosing_scope();
		delete inner;
	}

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
    // expressions
	variable *lvalue();
    void condition();
    void expression();
    void term();
    void factor();
public:
    parser(lexer &lex);
    bytecode program();
};

}

#endif