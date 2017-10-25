#ifndef PL_ZERO_LEXER_H
#define PL_ZERO_LEXER_H

#include <istream>

#include "token.h"
#include "util.h"

namespace pl0 {

class lexer {
    std::istream &input_stream_;
    std::string literal_buffer_;
    token peek_;
public:
    lexer(std::istream &input_stream) : input_stream_(input_stream) {
        advance();
    }

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
};

}

#endif
