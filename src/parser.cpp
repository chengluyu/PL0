#include "parser.h"
#include <iostream>

namespace pl0 {

void parser::subprogram() {
    while (lexer_.peek(token::CONST)) constant_decl();
    while (lexer_.peek(token::VAR)) variable_decl();
    while (lexer_.peek(token::PROCEDURE)) procedure_decl();
    statement();
}

// declarations
void parser::variable_decl() {
    expect(token::VAR);
    do {
        std::string ident = identifier();
        top_->define(new variable(ident, top_->get_level(), top_->get_variable_count()));
    } while(lexer_.match(token::COMMA));
    expect(token::SEMICOLON);
}

void parser::constant_decl() {
    expect(token::CONST);
    do {
        std::string ident = identifier();
        expect(token::EQ);
        int num = number();
        top_->define(new constant(ident, num));
    } while(lexer_.match(token::COMMA));
    expect(token::SEMICOLON);
}

void parser::procedure_decl() {
    expect(token::PROCEDURE);
    std::string procname = identifier();
    auto *proc = new procedure(procname, top_->get_level());
    top_->define(proc);
    expect(token::SEMICOLON);
    enter_scope();
    subprogram();
    expect(token::SEMICOLON);
    leave_scope();
}

// statements
void parser::statement_block() {
    expect(token::BEGIN);
    do {
        statement();
    } while (lexer_.match(token::SEMICOLON));
    expect(token::END);
}

void parser::if_statement() {
    expect(token::IF);
    condition();
    expect(token::THEN);
    statement();
    if (lexer_.match(token::ELSE)) {
        statement();
    }
}

void parser::while_statement() {
    expect(token::WHILE);
    condition();
    expect(token::DO);
    statement();
}

void parser::call_statement() {
    expect(token::CALL);
    std::string callee = identifier();
    symbol *sym = top_->resolve(callee);
    if (sym == nullptr) {
        // This procedure has not been declared yet,
        // but it may be declared later. We will cope
        // with this situation in the future version.
        throw general_error("undeclared procedure \"", callee, '"');
    } else if (sym->is_procedure()) {

    } else {
        throw general_error("cannot call non-procedure \"", callee, '"');
    }
}

void parser::statement() {
    switch (lexer_.peek()) {
    case token::READ:
        read_statement();
        break;
    case token::WRITE:
        write_statement();
        break;
    case token::IF:
        if_statement();
        break;
    case token::BEGIN:
        statement_block();
        break;
    case token::WHILE:
        while_statement();
        break;
    case token::CALL:
        call_statement();
        break;
    case token::RETURN:
        return_statement();
        break;
    default:
        assign_statement();
        break;
    }
}

void parser::read_statement() {
    expect(token::READ);
    do {
        variable *var = lvalue();
    } while (lexer_.match(token::COMMA));
}

void parser::write_statement() {
    expect(token::WRITE);
    do {
        expression();
    } while (lexer_.match(token::COMMA));
}

void parser::assign_statement() {
    variable *var = lvalue();
    expect(token::ASSIGN);
    expression();
}

void parser::return_statement() {
    expect(token::RETURN);
}

// expressions
variable *parser::lvalue() {
    std::string id = identifier();
    symbol *sym = top_->resolve(id);
    if (sym == nullptr) {
        throw general_error("undeclared identifier \"", id, '"');
    } else if (sym->is_variable()) {
        return dynamic_cast<variable*>(sym);
    } else {
        throw general_error("cannot assign value to a non-variable \"", id, '"');
    }
}

void parser::condition() {
    if (lexer_.match(token::ODD)) {
        expression();
    } else {
        expression();
        token cmp_op = lexer_.next();
        if (!is_compare_operator(cmp_op)) {
            throw general_error("expect a compare operator instead of ", *cmp_op);
        }
        expression();
    }
}

void parser::expression() {
    term();
    while (lexer_.peek(token::MUL) || lexer_.peek(token::DIV)) {
        token op = lexer_.next();
        term();
    }
}

void parser::term() {
    factor();
    while (lexer_.peek(token::ADD) || lexer_.peek(token::SUB)) {
        token op = lexer_.next();
        factor();
    }
}

void parser::factor() {
    if (lexer_.peek(token::IDENTIFIER)) {
        std::string id = lexer_.get_literal();
        lexer_.advance();
        symbol *sym = top_->resolve(id);
        if (sym == nullptr) {
            throw general_error("undeclared identifier \"", id, '"');
        } else if (sym->is_variable()) {
            auto *var = dynamic_cast<variable*>(sym);
        } else if (sym->is_constant()) {
            auto *cons = dynamic_cast<constant*>(sym);
        } else {
            throw general_error("procedure cannot be used in expression");
        }
    } else if (lexer_.peek(token::NUMBER)) {
        std::string num = lexer_.get_literal();
        lexer_.advance();
    } else if (lexer_.match(token::LPAREN)) {
        expression();
        expect(token::RPAREN);
    } else {
        throw general_error("expect an identifier, a number or a expression instead of ", *lexer_.peek());
    }
}

parser::parser(lexer & lexer) : lexer_(lexer), top_(nullptr) { }

void parser::program() {
    enter_scope();
    subprogram();
    expect(token::PERIOD);
    expect(token::EOS);
    leave_scope();
}

void parser::enter_scope() {
    top_ = new scope(top_);
}

void parser::leave_scope() {
    scope *inner = top_;
    top_ = top_->get_enclosing_scope();
    delete inner;
}

void parser::expect(token tk) {
    if (lexer_.peek(tk))
        lexer_.advance();
    else
        throw general_error("expect ", *tk, " instead of ", *lexer_.peek());
}

std::string parser::identifier() {
    if (lexer_.peek(token::IDENTIFIER)) {
        std::string result = lexer_.get_literal();
        lexer_.advance();
        return result;
    }
    throw general_error("expect an identifier instead of ", *lexer_.peek());
}

int parser::number() {
    if (lexer_.peek(token::NUMBER)) {
        int num = std::stoi(lexer_.get_literal());
        lexer_.advance();
        return num;
    }
    throw general_error("expect a number instead of ", *lexer_.peek());
}

}
