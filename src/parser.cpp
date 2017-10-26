#include "parser.h"

namespace pl0 {

int parser::subprogram() {
    while (lexer_.peek(token::CONST)) constant_decl();
    while (lexer_.peek(token::VAR)) variable_decl();
    while (lexer_.peek(token::PROCEDURE)) procedure_decl();
    int proc_begin = asm_.get_next_address();
    asm_.enter(top_->get_variable_count() + 3);
    statement();
    asm_.leave();
    return proc_begin;
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
    procedure *proc = new procedure(procname, top_->get_level());
    top_->define(proc);
    expect(token::SEMICOLON);
    enter_scope();
    proc->set_entry_address(subprogram());
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
    auto br_conseq_end = asm_.branch_if_false();
    statement();
    br_conseq_end->address = asm_.get_next_address();
}

void parser::while_statement() {
    expect(token::WHILE);
    int loop_begin = asm_.get_next_address();
    condition();
    expect(token::DO);
    auto br_loop_end = asm_.branch_if_false();
    statement();
    asm_.branch(loop_begin);
    br_loop_end->address = asm_.get_next_address();
}

void parser::call_statement() {
    expect(token::CALL);
    std::string callee = identifier();
    symbol *symb = top_->resolve(callee);
    if (symb == nullptr) {
        // This procedure has not been declared yet,
        // but it may be declared later. We will cope
        // with this situation in the future version.
        throw general_error("undeclared procedure \"", callee, '"');
    } else if (symb->is_procedure()) {
        procedure *proc = dynamic_cast<procedure*>(symb);
        asm_.call(top_->get_level() - proc->get_level(), proc->get_entry_address());
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
    default:
        assign_statement();
        break;
    }
}

void parser::read_statement() {
    expect(token::READ);
    do {
        variable *var = lvalue();
        asm_.read();
        asm_.store(top_->get_level() - var->get_level(), var->get_index());
    } while (lexer_.match(token::COMMA));
}

void parser::write_statement() {
    expect(token::WRITE);
    do {
        expression();
        asm_.write();
    } while (lexer_.match(token::COMMA));
}

void parser::assign_statement() {
    variable *var = lvalue();
    expect(token::ASSIGN);
    expression();
    asm_.store(top_->get_level() - var->get_level(), var->get_index());
}

// expressions
variable *parser::lvalue() {
    std::string ident = identifier();
    symbol *sym = top_->resolve(ident);
    if (sym == nullptr) {
        throw general_error("undeclared identifier \"", ident, '"');
    } else if (sym->is_variable()) {
        return dynamic_cast<variable*>(sym);
    } else {
        throw general_error("cannot assign value to a non-variable \"", ident, '"');
    }
}

void parser::condition() {
    if (lexer_.match(token::ODD)) {
        expression();
        asm_.operation(token::ODD);
    } else {
        expression();
        token cmp_op = lexer_.next();
        if (!is_compare_operator(cmp_op)) {
            throw general_error("expect a compare operator instead of ", *cmp_op);
        }
        expression();
        asm_.operation(cmp_op);
    }
}

void parser::expression() {
    term();
    while (lexer_.peek(token::MUL) || lexer_.peek(token::DIV)) {
        token op = lexer_.next();
        term();
        asm_.operation(op);
    }
}

void parser::term() {
    factor();
    while (lexer_.peek(token::ADD) || lexer_.peek(token::SUB)) {
        token op = lexer_.next();
        factor();
        asm_.operation(op);
    }
}

void parser::factor() {
    if (lexer_.peek(token::IDENTIFIER)) {
        std::string ident = lexer_.get_literal();
        lexer_.advance();
        symbol *symb = top_->resolve(ident);
        if (symb == nullptr) {
            throw general_error("undeclared identifier \"", ident, '"');
        } else if (symb->is_variable()) {
            variable *var = dynamic_cast<variable*>(symb);
            asm_.load(top_->get_level() - var->get_level(), var->get_index());
        } else if (symb->is_constant()) {
            constant *cons = dynamic_cast<constant*>(symb);
            asm_.load(cons->get_value());
        } else {
            throw general_error("procedure cannot be used in expression");
        }
    } else if (lexer_.peek(token::NUMBER)) {
        std::string num = lexer_.get_literal();
        lexer_.advance();
        asm_.load(std::stoi(num));
    } else if (lexer_.match(token::LPAREN)) {
        expression();
        expect(token::RPAREN);
    } else {
        throw general_error("expect an identifier, a number or a expression instead of ", *lexer_.peek());
    }
}

parser::parser(lexer & lexer) : lexer_(lexer), top_(nullptr) {

}

bytecode parser::program() {
    auto br_main = asm_.branch();
    enter_scope();
    br_main->address = subprogram();
    expect(token::PERIOD);
    expect(token::EOS);
    leave_scope();
    return asm_.get_bytecode();
}

}
