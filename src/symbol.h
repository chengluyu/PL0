#ifndef PL_ZERO_SYMBOL_H
#define PL_ZERO_SYMBOL_H

#include <string>

namespace pl0 {

class symbol {
    std::string name_;
public:
    symbol(std::string name) : name_(name) {}

    const std::string &get_name() const {
        return name_;
    }

    virtual bool is_variable() const = 0;

    virtual bool is_constant() const = 0;

    virtual bool is_procedure() const = 0;
};


class variable : public symbol {
    int level_;
    int index_;
public:
    variable(std::string &&name, int level, int index)
        : symbol(name), level_(level), index_(index) {}

    int get_level() const {
        return level_;
    }

    int get_index() const {
        return index_;
    }

    virtual bool is_variable() const final {
        return true;
    }

    virtual bool is_constant() const final {
        return false;
    }

    virtual bool is_procedure() const final {
        return false;
    }
};


class constant : public symbol {
    int value_;
public:
    constant(std::string &&name, int value)
        : symbol(name), value_(value) {}

    int get_value() const {
        return value_;
    }

    virtual bool is_variable() const final {
        return false;
    }

    virtual bool is_constant() const final {
        return true;
    }

    virtual bool is_procedure() const final {
        return false;
    }
};


class procedure : public symbol {
    int level_;
    int entry_address_;
public:
    procedure(std::string &&name, int level, int entry_address)
        : symbol(name), level_(level), entry_address_(entry_address) {}

    int get_level() const {
        return level_;
    }

    void set_entry_address(int ea) {
        entry_address_ = ea;
    }

    int get_entry_address() const {
        return entry_address_;
    }

    virtual bool is_variable() const final {
        return false;
    }

    virtual bool is_constant() const final {
        return false;
    }

    virtual bool is_procedure() const final {
        return true;
    }
};

}

#endif
