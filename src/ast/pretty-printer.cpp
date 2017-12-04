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
    begin_block();
    end_line();
    visit_block(node->main_block());
    end_block();
}

// statement visitor methods

void ast_printer::visit_assign_statement(assign_statement *node) {
    // assign statement {
    out_ << "assign statement";
    begin_block();
    end_line();
    out_ << "target = ";
    visit_variable_proxy(node->target());
    // expression =
    end_line();
    visit(node->expr());
    end_block();
}

void ast_printer::visit_if_statement(if_statement *node) {
    out_ << "while";
    begin_block();
    end_line();
    out_ << "condition = ";
    visit(node->condition());
    end_line();
    out_ << "consequence = ";
    visit(node->then_statement());
    if (node->has_else_statement()) {
        end_line();
        out_ << "alternation = ";
        visit(node->else_statement());
    }
    end_block();
}

void ast_printer::visit_while_statement(while_statement *node) {
    out_ << "while";
    begin_block();
    end_line();
    out_ << "condition = ";
    visit(node->cond());
    end_line();
    out_ << "body = ";
    visit(node->body());
    end_block();
}

void ast_printer::visit_call_statement(call_statement *node) {
    out_ << "invoke " << node->callee();
}

void ast_printer::visit_block(block *node) {
    out_ << "block";
    begin_block();
    end_line();
    if (node->var_declaration()) {
        out_ << "variables = ";
        visit_variable_declaration(node->var_declaration());
        end_line();
    }
    if (node->const_declaration()) {
        out_ << "constants = ";
        visit_constant_declaration(node->const_declaration());
        end_line();
    }
    if (!node->sub_procedures().empty()) {
        out_ << "procedures:";
        begin_block();
        int index = 0;
        for (auto method : node->sub_procedures()) {
            end_line();
            out_ << '[' << index++ << "] = ";
            visit_procedure_declaration(method);
        }
        end_block();
        end_line();
    }
    out_ << "body = ";
    visit(node->body());
    end_block();
}

void ast_printer::visit_statement_list(statement_list *node) {
    out_ << "statement list";
    increase_indent();
    int index = 0;
    for (auto statement : node->statements()) {
        end_line();
        out_ << '[' << index++ << "] = ";
        visit(statement);
    }
    end_block();
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
    begin_block();
    end_line();
    out_ << "operator = " << *node->op();
    end_line();
    out_ << "expression = ";
    visit(node->expr());
    end_block();
}

void ast_printer::visit_binary_operation(binary_operation *node) {
    // binary operation {
    out_ << "binary operation";
    begin_block();
    end_line();
    // operator =
    out_ << "operator = '" << *node->op() << '\'';
    end_line();
    out_ << "left = ";
    visit(node->left());
    end_line();
    out_ << "right = ";
    visit(node->right());
    end_block();
}

void ast_printer::visit_variable_proxy(variable_proxy *node) {
    symbol *sym = node->target();
    if (sym->is_constant()) {
        out_ << "constant ";
    } else if (sym->is_variable()) {
        out_ << "variable ";
    } else if (sym->is_procedure()) {
        out_ << "procedure ";
    }
    out_ << node->target()->get_name();
}

void ast_printer::visit_literal(literal *node) {
    out_ << "literal " << node->value();
}

}
