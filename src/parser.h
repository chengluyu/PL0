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
    std::unordered_map<std::string, std::vector<backpatcher>> calls_;

    // scope control
    void enter_scope() {
        top_ = new scope(top_);
    }

    void leave_scope() {
        scope *inner = top_;
        top_ = top_->get_enclosing_scope();
        delete inner;
    }
    
    // helper functions
    void expect(token tk) {
        if (lexer_.peek(tk))
            lexer_.advance();
        else
            throw general_error("expect ", *tk, " instead of ", *lexer_.peek());
    }
    
    std::string identifier() {
        if (lexer_.peek(token::IDENTIFIER)) {
            std::string result = lexer_.get_literal();
            lexer_.advance();
            return result;
        }
        throw general_error("expect an identifier instead of ", *lexer_.peek());
    }
    
    int number() {
        if (lexer_.peek(token::NUMBER)) {
            int num = std::stoi(lexer_.get_literal());
            lexer_.advance();
            return num;
        }
        throw general_error("expect a number instead of ", *lexer_.peek());
    }

    int subprogram();
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
    parser(lexer &lex);
    bytecode program();
};

}

#endif
