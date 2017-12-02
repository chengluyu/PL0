#include "parser.h"
#include <iostream>

namespace pl0 {

ast::block * parser::subprogram() {
    auto constants = lexer_.peek(token::CONST) ? constant_decl() : nullptr;
    auto variables = lexer_.peek(token::VAR) ? variable_decl() : nullptr;
    std::vector<ast::procedure_declaration*> sub_methods;
    while (lexer_.peek(token::PROCEDURE))
        sub_methods.push_back(procedure_decl());
    auto body = statement();
    return new ast::block{top_, variables, constants, std::move(sub_methods), body};
}

// declarations
ast::variable_declaration * parser::variable_decl() {
    ast::variable_declaration::list_type vars;
    expect(token::VAR);
    do {
        std::string id = identifier();
        auto sym = new variable(id, top_->get_level(), top_->get_variable_count());
        vars.push_back(sym);
        top_->define(sym);
    } while (lexer_.match(token::COMMA));
    expect(token::SEMICOLON);
    return new ast::variable_declaration{std::move(vars)};
}

ast::constant_declaration * parser::constant_decl() {
    ast::constant_declaration::list_type consts;
    expect(token::CONST);
    do {
        std::string id = identifier();
        expect(token::EQ);
        auto sym = new constant(id, number());
        consts.push_back(sym);
        top_->define(sym);
    } while(lexer_.match(token::COMMA));
    expect(token::SEMICOLON);
    return new ast::constant_declaration{std::move(consts)};
}

ast::procedure_declaration * parser::procedure_decl() {
    expect(token::PROCEDURE);
    std::string name = identifier();
    auto *sym = new procedure(name, top_->get_level());
    top_->define(sym);
    expect(token::SEMICOLON);
    enter_scope();
    auto block = subprogram();
    leave_scope();
    expect(token::SEMICOLON);
    return new ast::procedure_declaration{sym, block};
}

// statements
ast::statement_list * parser::statement_list() {
    ast::statement_list::list_type statements;
    expect(token::BEGIN);
    do {
        statements.push_back(statement());
    } while (lexer_.match(token::SEMICOLON));
    expect(token::END);
    return new ast::statement_list(std::move(statements));
}

ast::if_statement * parser::if_statement() {
    expect(token::IF);
    auto cond = condition();
    expect(token::THEN);
    auto then = statement();
    return new ast::if_statement(cond, then, lexer_.match(token::ELSE) ? statement() : nullptr);
}

ast::while_statement * parser::while_statement() {
    expect(token::WHILE);
    auto cond = condition();
    expect(token::DO);
    return new ast::while_statement(cond, statement());
}

ast::call_statement * parser::call_statement() {
    expect(token::CALL);
    std::string callee = identifier();
    symbol *sym = top_->resolve(callee);
    if (sym == nullptr || sym->is_procedure()) {
        return new ast::call_statement(std::move(callee));
    } else {
        throw general_error("cannot call non-procedure \"", callee, '"');
    }
}

ast::statement * parser::statement() {
    switch (lexer_.peek()) {
    case token::READ: return read_statement();
    case token::WRITE: return write_statement();
    case token::IF: return if_statement();
    case token::BEGIN: return statement_list();
    case token::WHILE: return while_statement();
    case token::CALL: return call_statement();
    case token::RETURN: return return_statement();
    default: return assign_statement();
    }
}

ast::read_statement * parser::read_statement() {
    ast::read_statement::list_type targets_;
    expect(token::READ);
    do {
        targets_.push_back(local_variable());
    } while (lexer_.match(token::COMMA));
    return new ast::read_statement(targets_);
}

ast::write_statement * parser::write_statement() {
    ast::write_statement::list_type expressions_;
    expect(token::WRITE);
    do {
        expressions_.push_back(expression());
    } while (lexer_.match(token::COMMA));
    return new ast::write_statement {expressions_};
}

ast::assign_statement * parser::assign_statement() {
    auto var = local_variable();
    expect(token::ASSIGN);
    return new ast::assign_statement {var, expression()};
}

ast::return_statement * parser::return_statement() {
    expect(token::RETURN);
    return new ast::return_statement {};
}

// expressions
ast::variable_proxy * parser::local_variable() {
    std::string id = identifier();
    symbol *sym = top_->resolve(id);
    if (sym == nullptr) {
        throw general_error("undeclared identifier \"", id, '"');
    } else if (sym->is_variable()) {
        return new ast::variable_proxy(dynamic_cast<variable*>(sym));
    } else {
        throw general_error("cannot assign value to a non-variable \"", id, '"');
    }
}

ast::expression * parser::condition() {
    if (lexer_.match(token::ODD)) {
        return new ast::unary_operation(token::ODD, expression());
    } else {
        auto left = expression();
        token cmp_op = lexer_.next();
        if (!is_compare_operator(cmp_op)) {
            throw general_error("expect a compare operator instead of ", *cmp_op);
        }
        return new ast::binary_operation(cmp_op, left, expression());
    }
}

ast::expression * parser::expression() {
    auto lhs = term();
    while (lexer_.peek(token::MUL) || lexer_.peek(token::DIV)) {
        token op = lexer_.next();
        lhs = new ast::binary_operation(op, lhs, term());
    }
    return lhs;
}

ast::expression * parser::term() {
    auto lhs = factor();
    while (lexer_.peek(token::ADD) || lexer_.peek(token::SUB)) {
        token op = lexer_.next();
        lhs = new ast::binary_operation(op, lhs, factor());
    }
    return lhs;
}

ast::expression * parser::factor() {
    if (lexer_.peek(token::IDENTIFIER)) {
        std::string id = lexer_.get_literal();
        lexer_.advance();
        symbol *sym = top_->resolve(id);
        if (sym == nullptr) {
            throw general_error("undeclared identifier \"", id, '"');
        } else if (sym->is_variable()) {
            auto *var = dynamic_cast<variable*>(sym);
            return new ast::variable_proxy(var);
        } else if (sym->is_constant()) {
            auto *cons = dynamic_cast<constant*>(sym);
            return new ast::literal(cons->get_value());
        } else {
            throw general_error("procedure cannot be used in expression");
        }
    } else if (lexer_.peek(token::NUMBER)) {
        return new ast::literal(number());
    } else if (lexer_.match(token::LPAREN)) {
        auto expr = expression();
        expect(token::RPAREN);
        return expr;
    } else {
        throw general_error("expect an identifier, a number or a expression instead of ", *lexer_.peek());
    }
}

parser::parser(lexer & lexer) : lexer_(lexer), top_(nullptr) { }

ast::block * parser::program() {
    enter_scope();
    auto block = subprogram();
    leave_scope();
    expect(token::PERIOD);
    expect(token::EOS);
    return block;
}

void parser::enter_scope() {
    top_ = new scope(top_);
}

void parser::leave_scope() {
    scope *inner = top_;
    top_ = top_->get_enclosing_scope();
}

void parser::expect(token tk) {
    if (!lexer_.match(tk))
        throw general_error("expect ", *tk, " instead of ", *lexer_.peek());
}

std::string parser::identifier() {
    if (lexer_.peek(token::IDENTIFIER)) {
        std::string result = lexer_.get_literal();
        lexer_.advance();
        return std::move(result);
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
