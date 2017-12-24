#ifndef PL_ZERO_LEXER_H
#define PL_ZERO_LEXER_H

#include <istream>

#include "token.h"
#include "../util.h"

namespace pl0 {

class lexer {
    std::istream &input_stream_;
    std::string literal_buffer_;
    token peek_;
    location loc_;

    inline int get();
    inline token select(char cond, token conseq, token altern);
public:
    explicit lexer(std::istream &input_stream)
        : input_stream_(input_stream), peek_(token::UNUSED) { advance(); }

    void advance();

    inline token peek() {
        return peek_;
    }

    inline bool peek(token tk) {
        return peek_ == tk;
    }

    inline token next() {
        token save = peek_;
        advance();
        return save;
    }

    inline bool match(token tk) {
        if (peek_ == tk) {
            next();
            return true;
        }
        return false;
    }
    
    inline std::string get_literal() {
        return literal_buffer_;
    }

    inline location loc() {
        return loc_;
    }
};

}

#endif
