#ifndef PL_ZERO_LEXER_H
#define PL_ZERO_LEXER_H

#include <istream>

#include "token.h"

namespace pl0 {

class location {
    int line_;
    int column_;
public:
    location(int line, int column) : line_(line), column_(column) { }

    std::string to_string() const {
        return std::to_string(line_) + ':' + std::to_string(column_);
    }
};

class lexer {
    std::istream &input_stream_;
    std::string literal_buffer_;
    token peek_;
    int line_;
    int column_;

    inline int get();
    inline token select(char cond, token conseq, token altern);
public:
    lexer(std::istream &input_stream)
        : input_stream_(input_stream), line_(1), column_(1) { advance(); }

    void advance();

    inline token peek() {
        return peek_;
    }

    inline bool peek(token tk) {
        return peek_ == tk;
    }

    token next() {
        token save = peek_;
        advance();
        return save;
    }

    bool match(token tk) {
        if (peek_ == tk) {
            next();
            return true;
        }
        return false;
    }
    
    inline std::string get_literal() {
        return literal_buffer_;
    }

    inline location current_location() {
        return location { line_, column_ };
    }
};

}

#endif
