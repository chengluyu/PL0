#include "parser.h"
#include <iostream>

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
    int proc_entry_addr = subprogram();
    proc->set_entry_address(proc_entry_addr);
    expect(token::SEMICOLON);
    // backpatch all call instructions
    auto iter = calls_.find(procname);
    if (iter != calls_.end()) {
        for (auto bp : iter->second) {
            bp.set_level(bp.get_level() - top_->get_level() + 1);
            bp.set_address(proc_entry_addr);
        }
        calls_.erase(iter);
    }
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
    auto test_failed = asm_.branch_if_false();
    statement();
    if (lexer_.match(token::ELSE)) {
        auto br_altern_end = asm_.branch();
        test_failed.set_address(asm_.get_next_address());
        statement();
        br_altern_end.set_address(asm_.get_next_address());
    } else {
        test_failed.set_address(asm_.get_next_address());
    }
}

void parser::while_statement() {
    expect(token::WHILE);
    int loop_begin = asm_.get_next_address();
    condition();
    expect(token::DO);
    auto br_loop_end = asm_.branch_if_false();
    statement();
    asm_.branch(loop_begin);
    br_loop_end.set_address(asm_.get_next_address());
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
        if (proc->get_entry_address() == procedure::invalid_address) {
            // the entry address of the callee has not been identified
            auto iter = calls_.find(callee);
            auto calling = asm_.call(top_->get_level());
            if (iter == calls_.end())
                calls_[callee] = { calling };
            else
                iter->second.push_back(calling);
        } else {
            asm_.call(top_->get_level() - proc->get_level(), proc->get_entry_address());
        }
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

void parser::return_statement() {
    expect(token::RETURN);
    asm_.leave();
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
    enter_scope();
    size_t start = static_cast<size_t>(subprogram());
    expect(token::PERIOD);
    expect(token::EOS);
    leave_scope();
    // check if there is any undeclared call instructions
    if (!calls_.empty()) {
        throw general_error("undeclared calls discovered");
    }
    // put main in the front
    bytecode result;
    if (start > 0) {
        const bytecode &code = asm_.get_bytecode();
        size_t offset = code.size() - start;
        for (size_t i = start; i < code.size(); i++) {
            result.push_back(code[i]);
            if (code[i].op == opcode::CAL)
                result.back().address += offset;
            else if (code[i].op == opcode::JMP || code[i].op == opcode::JPC) {
                instruction &ins = result.back();
                if (ins.address > start)
                    ins.address -= start;
                else
                    ins.address += offset;
            }
        }
        for (size_t i = 0; i < start; i++) {
            result.push_back(code[i]);
            if (code[i].op == opcode::CAL || code[i].op == opcode::JMP || code[i].op == opcode::JPC)
                result.back().address += offset;
        }
    } else {
        result = asm_.get_bytecode();
    }
    return result;
}

}
