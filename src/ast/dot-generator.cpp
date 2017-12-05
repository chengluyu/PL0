//
// Created by chengluyu on 2017/12/4.
//

#include <fstream>

#include "dot-generator.h"

namespace pl0::ast {

void dot_generator::visit_constant_declaration(constant_declaration *node) {
    auto prefix = get_name(node);
    label(prefix, "const declaration");
    for (auto sym : node->constants()) {
        auto sym_name = prefix + sym->get_name();
        link(prefix, sym_name);
        label(sym_name, sym->get_name());
    }
}

void dot_generator::visit_variable_declaration(variable_declaration *node) {
    auto prefix = get_name(node);
    label(prefix, "variable declaration");
    for (auto sym : node->variables()) {
        auto sym_name = prefix + '_' + sym->get_name();
        link(prefix, sym_name);
        label(sym_name, sym->get_name());
    }
}

void dot_generator::visit_procedure_declaration(procedure_declaration *node) {
    auto prefix = get_name(node);
    auto name_field = prefix + "_name";
    label(name_field, node->symbol()->get_name());
    link(prefix, name_field);
    link(prefix, get_name(node->main_block()));
    visit_block(node->main_block());
}

void dot_generator::visit_unary_operation(unary_operation *node) {
    auto name = get_name(node);
    label(name, "unary");
    auto op_name = name + "_op";
    label(op_name, *node->op());
    link(name, op_name);
    link(name, get_name(node->expr()));
    visit(node->expr());
}

void dot_generator::visit_binary_operation(binary_operation *node) {
    auto name = get_name(node);
    label(name, "binary");
    auto op_name = name + "_op";
    label(op_name, *node->op());
    link(name, op_name);
    link(name, get_name(node->left()));
    link(name, get_name(node->right()));
    visit(node->left());
    visit(node->right());
}

void dot_generator::visit_variable_proxy(variable_proxy *node) {
    label(get_name(node), "variable " + node->target()->get_name());
}

void dot_generator::visit_literal(literal *node) {
    label(get_name(node), "literal " + std::to_string(node->value()));
}

void dot_generator::visit_block(block *node) {
    auto name = get_name(node);
    label(name, "block");
    if (node->const_declaration()) {
        link(name, get_name(node->const_declaration()));
        visit_constant_declaration(node->const_declaration());
    }
    if (node->var_declaration()) {
        link(name, get_name(node->var_declaration()));
        visit_variable_declaration(node->var_declaration());
    }
    for (auto method : node->sub_procedures()) {
        auto method_name = get_name(method);
        link(name, method_name);
        label(method_name, "procedure " + method->symbol()->get_name());
        visit_procedure_declaration(method);
    }
    link(name, get_name(node->body()));
    visit(node->body());
}

void dot_generator::visit_assign_statement(assign_statement *node) {
    auto name = get_name(node);
    label(name, "assign");
    link(name, get_name(node->target()));
    link(name, get_name(node->expr()));
    visit_variable_proxy(node->target());
    visit(node->expr());
}

void dot_generator::visit_call_statement(call_statement *node) {
    auto name = get_name(node);
    label(name, "call");
    link(name, node->callee());
}

void dot_generator::visit_if_statement(if_statement *node) {
    auto name = get_name(node);
    label(name, "if");
    auto cond = get_name(node->condition());
    auto then = get_name(node->then_statement());
    auto otherwise = get_name(node->else_statement());
    link(name, cond);
    link(name, then);
    if (node->has_else_statement())
        link(name, otherwise);
    visit(node->condition());
    visit(node->then_statement());
    if (node->has_else_statement())
        visit(node->else_statement());
}

void dot_generator::visit_read_statement(read_statement *node) {
    auto name = get_name(node);
    label(name, "read");
    for (auto sym : node->targets())
        link(name, name + sym->target()->get_name());
}

void dot_generator::visit_return_statement(return_statement *node) {
    auto name = get_name(node);
    label(name, "return");
}

void dot_generator::visit_statement_list(statement_list *node) {
    auto name = get_name(node);
    label(name, "statements");
    for (auto stmt : node->statements()) {
        link(name, get_name(stmt));
        visit(stmt);
    }
}

void dot_generator::visit_while_statement(while_statement *node) {
    auto name = get_name(node);
    label(name, "while");
    link(name, get_name(node->cond()));
    link(name, get_name(node->body()));
    visit(node->cond());
    visit(node->body());
}

void dot_generator::visit_write_statement(write_statement *node) {
    auto name = get_name(node);
    label(name, "write");
    for (auto expr : node->expressions()) {
        link(name, get_name(expr));
        visit(expr);
    }
}

void dot_generator::generate(block *root) {
    source_ << "digraph G {\n";
    visit_block(root);
    source_ << "}\n";
}

void dot_generator::save_to_file(const char *filename) {
    std::ofstream ofs(filename);
    ofs << source_.str();
    ofs.close();
}

}