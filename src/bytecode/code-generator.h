#ifndef PL0_CODE_GENERATOR_H
#define PL0_CODE_GENERATOR_H

#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/ast.h"
#include "assembler.h"
#include "../util.h"

namespace pl0::code {

class generator : public ast::ast_visitor<generator> {
    std::unordered_map<procedure *, int> entry_points_;
    std::unordered_map<procedure *, std::vector<backpatcher>> patch_list_;
    assembler assembler_;
    scope *top_scope_;

    DEFINE_AST_VISITOR_SUBCLASS_MEMBERS()
    DECLARE_VISIT_METHODS

    void visit_rvalue(ast::variable_proxy *node);
    void visit_lvalue(ast::variable_proxy *node);
public:
    generator() : top_scope_(nullptr) { }

    void generate(ast::block *program);

    const bytecode &code() { return assembler_.get_bytecode(); }
};

}

#endif //PL0_CODE_GENERATOR_H
