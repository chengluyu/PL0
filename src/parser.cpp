#include "parser.h"

namespace pl0 {

int parser::subprogram() {
    while (lexer_.peek(token_type::CONST)) constant_decl();
    while (lexer_.peek(token_type::VAR)) variable_decl();
    while (lexer_.peek(token_type::PROCEDURE)) procedure_decl();
    int proc_begin = asm_.get_next_address();
	asm_.enter(top_->get_variable_count() + 3);
    statement();
	asm_.leave();
    return proc_begin;
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
    lexer_.expect(token_type::CONST);
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
    procedure *proc = new procedure(procname, top_->get_level());
    top_->define(proc);
    lexer_.expect(token_type::SEMICOLON);
	enter_scope();
    proc->set_entry_address(subprogram());
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
    auto branch_to_conseq_end = asm_.branch_if_false();
    statement();
    branch_to_conseq_end.set_address(asm_.get_next_address());
}

void parser::while_statement() {
    lexer_.expect(token_type::WHILE);
    int loop_begin = asm_.get_next_address();
    condition();
    lexer_.expect(token_type::DO);
    auto branch_to_loop_end = asm_.branch_if_false();
    statement();
    asm_.branch(loop_begin);
    branch_to_loop_end.set_address(asm_.get_next_address());
}

void parser::call_statement() {
    lexer_.expect(token_type::CALL);
    std::string callee = lexer_.expect(token_type::IDENTIFIER).second.value();
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
		variable *var = lvalue();
		asm_.read();
		asm_.store(top_->get_level() - var->get_level(), var->get_index());
    } while (lexer_.match(token_type::COMMA));
}

void parser::write_statement() {
    lexer_.expect(token_type::WRITE);
    do {
        expression();
		asm_.write();
    } while (lexer_.match(token_type::COMMA));
}

void parser::assign_statement() {
	variable *var = lvalue();
    lexer_.expect(token_type::ASSIGN);
    expression();
	asm_.store(top_->get_level() - var->get_level(), var->get_index());
}

// expressions
variable *parser::lvalue() {
	std::string ident = lexer_.expect(token_type::IDENTIFIER).second.value();
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
    if (lexer_.match(token_type::ODD)) {
        expression();
		asm_.operation(token_type::ODD);
    } else {
        expression();
        token_type cmp_op = lexer_.next().first;
        if (!is_compare_operator(cmp_op)) {
            throw general_error("expect a compare operator instead of ", *cmp_op);
        }
		expression();
		asm_.operation(cmp_op);
    }
}

void parser::expression() {
    term();
    while (lexer_.peek(token_type::MUL) || lexer_.peek(token_type::DIV)) {
        token_type op = lexer_.next().first;
        term();
		asm_.operation(op);
    }
}

void parser::term() {
    factor();
    while (lexer_.peek(token_type::ADD) || lexer_.peek(token_type::SUB)) {
        token_type op = lexer_.next().first;
        factor();
		asm_.operation(op);
    }
}

void parser::factor() {
    if (lexer_.peek(token_type::IDENTIFIER)) {
        std::string ident = lexer_.next().second.value();
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
    } else if (lexer_.peek(token_type::NUMBER)) {
        std::string num = lexer_.next().second.value();
		asm_.load(std::stoi(num));
    } else if (lexer_.match(token_type::LPAREN)) {
        expression();
		lexer_.expect(token_type::RPAREN);
    } else {
        throw general_error("expect an identifier, a number or a expression instead of ", *lexer_.peek());
    }
}

parser::parser(lexer & lexer) : lexer_(lexer), top_(nullptr) {

}

bytecode parser::program() {
    auto jump_to_main = asm_.branch();
	enter_scope();
	jump_to_main.set_address(subprogram());
	lexer_.expect(token_type::PERIOD);
	lexer_.expect(token_type::EOS);
	leave_scope();
    return asm_.get_bytecode();
}

}