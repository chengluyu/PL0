#ifndef PL0_PRETTY_PRINTER_H
#define PL0_PRETTY_PRINTER_H

#include <ostream>

#include "ast.h"

namespace pl0::ast {

class ast_printer : public ast_visitor<ast_printer> {
    DEFINE_AST_VISITOR_SUBCLASS_MEMBERS()

    std::ostream &out_;
    const int indent_size_;
    int indent_level_;

    void increase_indent() {
        indent_level_ += indent_size_;
    }

    void decrease_indent() {
        indent_level_ -= indent_size_;
    }

    void write_indent() {
        for (int i = 0; i < indent_level_; i++)
            out_.put(' ');
    }

    void newline() {
        out_.put('\n');
        write_indent();
    }

    void increase_indent_and_newline() {
        indent_level_ += indent_size_;
        newline();
    }

    void decrease_indent_and_newline() {
        indent_level_ -= indent_size_;
    }

public:
    explicit ast_printer(std::ostream &out, int indent_size = 2)
            : out_(out), indent_size_(indent_size), indent_level_(0) { }

    DECLARE_VISIT_METHODS
};

}

#endif //PL0_PRETTY_PRINTER_H
