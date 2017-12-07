#include "code-generator.h"

namespace pl0::code {

void generator::visit_variable_declaration(ast::variable_declaration *node) { }

void generator::visit_constant_declaration(ast::constant_declaration *node) { }

void generator::visit_procedure_declaration(ast::procedure_declaration *node) {
    entry_points_[node->symbol()] = assembler_.get_next_address();
    visit_block(node->main_block());
}

void generator::visit_block(ast::block *node) {
    top_scope_ = node->belonging_scope();
    assembler_.enter(top_scope_->get_variable_count() + 3);
    visit(node->body());
    assembler_.leave();
    for (auto method : node->sub_procedures())
        visit_procedure_declaration(method);
    top_scope_ = top_scope_->get_enclosing_scope();
}

void generator::visit_unary_operation(ast::unary_operation *node) {
    visit(node->expr());
    assembler_.operation(node->op());
}

void generator::visit_binary_operation(ast::binary_operation *node) {
    visit(node->left());
    visit(node->right());
    assembler_.operation(node->op());
}

void generator::visit_literal(ast::literal *node) {
    assembler_.load(node->value());
}

void generator::visit_variable_proxy(ast::variable_proxy *node) {
    visit_rvalue(node);
}

void generator::visit_lvalue(ast::variable_proxy *node) {
    auto sym = node->target();
    if (sym->is_variable()) {
        auto var = dynamic_cast<variable *>(sym);
        assembler_.store(top_scope_->get_level() - var->get_level(), var->get_index());
    } else if (sym->is_constant())
        throw general_error("constant " + sym->get_name() + " is not assignable");
    else
        throw general_error("procedure " + sym->get_name() + " is not assignable");
}

void generator::visit_rvalue(ast::variable_proxy *node) {
    auto sym = node->target();
    if (sym->is_variable()) {
        auto var = dynamic_cast<variable *>(sym);
        assembler_.load(top_scope_->get_level() - var->get_level(), var->get_index());
    } else if (sym->is_constant()) {
        auto var = dynamic_cast<constant *>(sym);
        assembler_.load(var->get_value());
    } else
        throw general_error(sym->get_name() + " is a procedure so that cannot be used in expression");
}

void generator::visit_assign_statement(ast::assign_statement *node) {
    visit(node->expr());
    visit_lvalue(node->target());
}

void generator::visit_call_statement(ast::call_statement *node) {
    auto sym = top_scope_->resolve(node->callee());
    if (sym == nullptr) throw general_error("no procedure named \"" + node->callee() + "\" to be called");
    if (!sym->is_procedure()) throw general_error(node->callee() + " is not a procedure");
    auto method = dynamic_cast<procedure *>(sym);
    patch_list_[method].push_back(assembler_.call(top_scope_->get_level()));
}

void generator::visit_write_statement(ast::write_statement *node) {
    for (auto expr : node->expressions()) {
        visit(expr);
        assembler_.write();
    }
}

void generator::visit_while_statement(ast::while_statement *node) {
    auto beginning = assembler_.get_next_address();
    visit(node->cond());
    auto goto_end = assembler_.branch_if_false();
    visit(node->body());
    assembler_.branch(beginning);
    goto_end.set_address(assembler_.get_next_address());
}

void generator::visit_return_statement(ast::return_statement *node) {
    assembler_.leave();
}

void generator::visit_read_statement(ast::read_statement *node) {
    for (auto var : node->targets()) {
        assembler_.read();
        visit_lvalue(var);
    }
}

void generator::visit_if_statement(ast::if_statement *node) {
    visit(node->condition());
    if (node->has_else_statement()) {
        auto goto_else = assembler_.branch_if_false();
        visit(node->then_statement());
        auto goto_end = assembler_.branch();
        goto_else.set_address(assembler_.get_next_address());
        visit(node->else_statement());
        goto_end.set_address(assembler_.get_next_address());
    } else {
        auto goto_end = assembler_.branch_if_false();
        visit(node->then_statement());
        goto_end.set_address(assembler_.get_next_address());
    }
}

void generator::visit_statement_list(ast::statement_list *node) {
    for (auto stmt : node->statements())
        visit(stmt);
}

void generator::generate(ast::block *program) {
    visit_block(program);
    for (auto kv : patch_list_) {
        for (auto patch : kv.second) {
            patch.set_level(patch.get_level() - kv.first->get_level());
            if (entry_points_.find(kv.first) == entry_points_.end())
                throw general_error("unexpected error");
            patch.set_address(entry_points_[kv.first]);
        }
    }
}

}