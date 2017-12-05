//
// Created by chengluyu on 2017/12/4.
//

#ifndef PL0_DOT_GENERATOR_H
#define PL0_DOT_GENERATOR_H


#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>

#include "ast.h"

namespace pl0::ast {

class dot_generator : public ast_visitor<dot_generator> {
    DEFINE_AST_VISITOR_SUBCLASS_MEMBERS()

    std::ostringstream source_;

    std::string get_name(ast_node *node) const {
        return 't' + std::to_string(reinterpret_cast<uint64_t>(node));
    }

    void link(const std::string &from, const std::string &to) {
        source_ << from << " -> " << to << ';' << '\n';
    }

    void label(const std::string &name, const std::string &label) {
        source_ << name << " [label=\"" << label << "\"];\n";
    }

    DECLARE_VISIT_METHODS
public:
    dot_generator() = default;

    void generate(block *root);

    void save_to_file(const char *filename);
};

}

#endif //PL0_DOT_GENERATOR_H
