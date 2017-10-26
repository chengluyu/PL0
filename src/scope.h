#ifndef PL_ZERO_SCOPE_H
#define PL_ZERO_SCOPE_H

#include <string>
#include <unordered_map>

#include "symbol.h"
#include "util.h"

namespace pl0 {

class scope {
    std::unordered_map<std::string, symbol*> members_;
    scope *enclosing_scope_;
    int level_, variable_count_;
public:
    scope(scope *enclosing_scope)
        : enclosing_scope_(enclosing_scope)
        , level_(enclosing_scope ? enclosing_scope->level_ + 1 : 0)
        , variable_count_(0) {}

    ~scope() {
        for (auto &pair : members_) {
            delete pair.second;
        }
    }

    void define(symbol *sym) {
        auto result = members_.emplace(sym->get_name(), sym);
        if (!result.second) {
            throw general_error("duplicated symbol \"", sym->get_name(), '"');
        }
        if (sym->is_variable()) {
            variable_count_++;
        }
    }

    symbol *resolve(const std::string &name) {
        auto iter = members_.find(name);
        if (iter == members_.end()) {
            return enclosing_scope_ ? enclosing_scope_->resolve(name) : nullptr;
        }
        return iter->second;
    }

    inline scope *get_enclosing_scope() {
        return enclosing_scope_;
    }

    inline int get_level() const {
        return level_;
    }

    inline int get_variable_count() const {
        return variable_count_;
    }
};

}

#endif
