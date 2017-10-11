#include "parser.h"

namespace pl0 {

void parser::subprogram() {
    while (lexer_.peek(token_type::CONST)) constant_decl();
    while (lexer_.peek(token_type::VAR)) variable_decl();
    while (lexer_.peek(token_type::PROCEDURE)) procedure_decl();
    statement();
}

// declarations
void parser::variable_decl() {
    lexer_.expect(token_type::VAR);
    do {
        std::string ident = lexer_.expect(token_type::IDENTIFIER).second.value();
        top_->define(new variable(ident, top_->get_level(), top_->get_variable_count()));
    } while(lexer_.match(token_type::COMMA));
    lexer_.expect(token_type::SEMICOLON);
}

void parser::constant_decl() {
    lexer_.expect(token_type::VAR);
    do {
        std::string ident = lexer_.expect(token_type::IDENTIFIER).second.value();
        lexer_.expect(token_type::EQ);
        std::string num = lexer_.expect(token_type::NUMBER).second.value();
        top_->define(new constant(ident, std::stoi(num)));
    } while(lexer_.match(token_type::COMMA));
    lexer_.expect(token_type::SEMICOLON);
}

void parser::procedure_decl() {
    lexer_.expect(token_type::PROCEDURE);
    std::string procname = lexer_.expect(token_type::IDENTIFIER).second.value();
    top_->define(new procedure(procname, top_->get_level(), 0));
    lexer_.expect(token_type::SEMICOLON);
	enter_scope();
    subprogram();
    lexer_.expect(token_type::SEMICOLON);
	leave_scope();
}

// statements
void parser::statement_block() {
    lexer_.expect(token_type::BEGIN);
    do {
        statement();
    } while (lexer_.match(token_type::SEMICOLON));
    lexer_.expect(token_type::END);
}

void parser::if_statement() {
    lexer_.expect(token_type::IF);
    condition();
    lexer_.expect(token_type::THEN);
    statement();
}

void parser::while_statement() {
    lexer_.expect(token_type::WHILE);
    condition();
    lexer_.expect(token_type::DO);
    statement();
}

void parser::call_statement() {
    lexer_.expect(token_type::CALL);
    std::string callee = lexer_.expect(token_type::IDENTIFIER).second.value();
    [[maybe_unused]]
    symbol *symb = top_->resolve(callee);
}

void parser::statement() {
    switch (lexer_.peek()) {
    case token_type::READ:
        read_statement();
        break;
    case token_type::WRITE:
        write_statement();
        break;
    case token_type::IF:
        if_statement();
        break;
    case token_type::BEGIN:
        statement_block();
        break;
    case token_type::WHILE:
        while_statement();
        break;
    case token_type::CALL:
        call_statement();
        break;
    default:
        assign_statement();
        break;
    }
}

void parser::read_statement() {
    lexer_.expect(token_type::READ);
    do {
        std::string arg = lexer_.expect(token_type::IDENTIFIER).second.value();
    } while (lexer_.match(token_type::COMMA));
}

void parser::write_statement() {
    lexer_.expect(token_type::READ);
    do {
        expression();
    } while (lexer_.match(token_type::COMMA));
}

void parser::assign_statement() {
    std::string lvalue = lexer_.expect(token_type::IDENTIFIER).second.value();
    lexer_.expect(token_type::ASSIGN);
    expression();
}

// expressions
void parser::condition() {
    if (lexer_.match(token_type::ODD)) {
        expression();
    } else {
        expression();
        token_type cmp_op = lexer_.next().first;
        if (!is_compare_operator(cmp_op)) {
            throw general_error("expect a compare operator instead of ", *cmp_op);
        }
        expression();
    }
}

void parser::expression() {
    term();
    while (lexer_.peek(token_type::MUL) || lexer_.peek(token_type::DIV)) {
        [[maybe_unused]]
        token_type op = lexer_.next().first;
        term();
    }
}

void parser::term() {
    factor();
    while (lexer_.peek(token_type::ADD) || lexer_.peek(token_type::SUB)) {
        [[maybe_unused]]
        token_type op = lexer_.next().first;
        factor();
    }
}

void parser::factor() {
    if (lexer_.peek(token_type::IDENTIFIER)) {
        std::string ident = lexer_.next().second.value();
        symbol *symb = top_->resolve(ident);
        if (symb == nullptr) {
            throw general_error("undeclared identifier \"", ident, '"');
        } else if (symb->is_variable()) {
            [[maybe_unused]]
            variable *varsymb = dynamic_cast<variable*>(symb);
        } else if (symb->is_constant()) {
            [[maybe_unused]]
            constant *consymb = dynamic_cast<constant*>(symb);
        } else {
            throw general_error("procedure cannot be used in expression");
        }
    } else if (lexer_.peek(token_type::NUMBER)) {
        [[maybe_unused]]
        std::string num = lexer_.next().second.value();
    } else if (lexer_.match(token_type::LPAREN)) {
        expression();
		lexer_.expect(token_type::RPAREN);
    } else {
        throw general_error("expect an identifier, a number or a expression instead of ", *lexer_.peek());
    }
}

parser::parser(lexer & lexer) : lexer_(lexer), top_(nullptr) {

}

void parser::program() {
	enter_scope();
	subprogram();
	lexer_.expect(token_type::PERIOD);
	lexer_.expect(token_type::EOS);
	leave_scope();
}

}