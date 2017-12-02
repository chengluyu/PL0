#include "pretty-printer.h"

namespace pl0::ast {

// declaration visitor methods

void ast_printer::visit_constant_declaration(constant_declaration *node) {
    out_ << "constant declaration [ ";
    for (auto sym : node->constants())
        out_ << sym->get_name() << ' ';
    out_.put(']');
}

void ast_printer::visit_variable_declaration(variable_declaration *node) {
    out_ << "variable declaration [ ";
    for (auto sym : node->variables())
        out_ << sym->get_name() << ' ';
    out_.put(']');
}

void ast_printer::visit_procedure_declaration(procedure_declaration *node) {
    out_ << "procedure declaration";
    increase_indent_and_newline();
    visit_block(node->main_block());
    decrease_indent_and_newline();
}

// statement visitor methods

void ast_printer::visit_assign_statement(assign_statement *node) {
    // assign statement {
    out_ << "assign statement";
    increase_indent_and_newline();
    out_ << "target = ";
    visit_variable_proxy(node->target());
    // expression =
    newline();
    visit(node->expr());
    decrease_indent_and_newline();
}

void ast_printer::visit_if_statement(if_statement *node) {
    out_ << "while";
    increase_indent_and_newline();
    out_ << "condition = ";
    visit(node->condition());
    newline();
    out_ << "consequence = ";
    visit(node->then_statement());
    if (node->has_else_statement()) {
        newline();
        out_ << "alternation = ";
        visit(node->else_statement());
    }
    decrease_indent_and_newline();
}

void ast_printer::visit_while_statement(while_statement *node) {
    out_ << "while";
    increase_indent_and_newline();
    out_ << "condition = ";
    visit(node->cond());
    newline();
    out_ << "body = ";
    visit(node->body());
    decrease_indent_and_newline();
}

void ast_printer::visit_call_statement(call_statement *node) {
    out_ << "invoke " << node->callee();
}

void ast_printer::visit_block(block *node) {
    out_ << "block";
    increase_indent_and_newline();
    if (node->var_declaration()) {
        out_ << "variables = ";
        visit_variable_declaration(node->var_declaration());
        newline();
    }
    if (node->const_declaration()) {
        out_ << "constants = ";
        visit_constant_declaration(node->const_declaration());
        newline();
    }
    auto methods = node->sub_procedures();
    if (!methods.empty()) {
        out_ << "procedures";
        increase_indent_and_newline();
        for (size_t i = 0; i < methods.size(); i++) {
            visit_procedure_declaration(methods[i]);
            if (i + 1 < methods.size()) newline();
        }
        decrease_indent_and_newline();
    }
    newline();
    out_ << "body = ";
    visit(node->body());
    decrease_indent_and_newline();
}

void ast_printer::visit_statement_list(statement_list *node) {
    out_ << "statement list";
    increase_indent();
    for (auto statement : node->statements()) {
        newline();
        visit(statement);
    }
    decrease_indent_and_newline();
}

void ast_printer::visit_read_statement(read_statement *node) {
    out_ << "read statement";
}

void ast_printer::visit_write_statement(write_statement *node) {
    out_ << "write statement";
}

void ast_printer::visit_return_statement(return_statement *node) {
    out_ << "return statement";
}

// expression visitor methods

void ast_printer::visit_unary_operation(unary_operation *node) {
    out_ << "unary operation";
    increase_indent_and_newline();
    out_ << "operator = " << *node->op();
    newline();
    out_ << "expression = ";
    visit(node->expr());
    decrease_indent_and_newline();
}

void ast_printer::visit_binary_operation(binary_operation *node) {
    // binary operation {
    out_ << "binary operation";
    increase_indent_and_newline();
    // operator =
    out_ << "operator = '" << *node->op() << '\'';
    newline();
    out_ << "left = ";
    visit(node->left());
    newline();
    out_ << "right = ";
    visit(node->right());
    decrease_indent_and_newline();
}

void ast_printer::visit_variable_proxy(variable_proxy *node) {
    out_ << "variable " << node->target()->get_name();
}

void ast_printer::visit_literal(literal *node) {
    out_ << "literal " << node->value();
}

}
