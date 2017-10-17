#ifndef PL_ZERO_SYMBOL_H
#define PL_ZERO_SYMBOL_H

#include <string>

namespace pl0 {

class symbol {
    std::string name_;
public:
    symbol(const std::string &name) : name_(name) {}

    const std::string &get_name() const {
        return name_;
    }

    virtual bool is_variable() const {
        return false;
    }

    virtual bool is_constant() const {
        return false;
    }

    virtual bool is_procedure() const {
        return false;
    }
};


class variable : public symbol {
    int level_;
    int index_;
public:
    variable(const std::string &name, int level, int index)
        : symbol(name), level_(level), index_(index) {}

    int get_level() const {
        return level_;
    }

    int get_index() const {
        return index_;
    }

    virtual bool is_variable() const {
        return true;
    }

};


class constant : public symbol {
    int value_;
public:
    constant(const std::string &name, int value)
        : symbol(name), value_(value) {}

    int get_value() const {
        return value_;
    }

    virtual bool is_constant() const {
        return true;
    }
};


class procedure : public symbol {
    int level_;
    int entry_address_;
public:
    procedure(const std::string &name, int level)
        : symbol(name), level_(level) {}

    int get_level() const {
        return level_;
    }

    void set_entry_address(int ea) {
        entry_address_ = ea;
    }

    int get_entry_address() const {
        return entry_address_;
    }

    virtual bool is_procedure() const {
        return true;
    }
};

}

#endif
